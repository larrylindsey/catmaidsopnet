#include "SliceGuarantor.h"

#include <imageprocessing/ImageExtractor.h>
#include <sopnet/sopnet/slices/SliceExtractor.h>
#include <sopnet/sopnet/slices/Slice.h>
#include <util/rect.hpp>
#include <util/Logger.h>

/*
 * TODO
 * Things left to code:
 * - check if a Slice is whole
 * - rectify Slice coordinates to Block coordinates
 * - merge Slices
 * - SliceGuarantor input class
 *   * is this a user-generated guarantee or a propogated one?
 *   * for propogated, which slices need to be whole?
 * 
 * Question: do I need to overlap Slice extraction in order to perform the merge correctly?
 */

logger::LogChannel sliceguarantorlog("sliceguarantorlog", "[SliceGuarantor] ");

SliceGuarantor::SliceGuarantor()
{
	LOG_DEBUG(sliceguarantorlog) << "Registering inputs on this" << std::endl;
	
	_stackReader = boost::make_shared<ImageBlockStackReader>();
	_blockSliceStore = boost::make_shared<BlockSliceStoreNode>();
	
	registerInput(_block, "block");
	registerInput(_sliceStore, "store");
	registerInput(_blockFactory, "block factory");
	registerInput(_forceExplanation, "force explanation");
	registerInput(_parameters, "parameters");
	
	_block.registerBackwardCallback(&SliceGuarantor::onBlockSet, this);
	_sliceStore.registerBackwardCallback(&SliceGuarantor::onSliceStoreSet, this);
	_blockFactory.registerBackwardCallback(&SliceGuarantor::onImageBlockFactorySet, this);
}

void
SliceGuarantor::onBlockSet(const pipeline::Modified&)
{
	LOG_DEBUG(sliceguarantorlog) << "Setting block inputs on inner ProcessNodes" << std::endl;	
	_stackReader->setInput("block", _block);
	_blockSliceStore->addInput("block", _block);
}

void
SliceGuarantor::onImageBlockFactorySet(const pipeline::Modified&)
{
	LOG_DEBUG(sliceguarantorlog) << "Registering imageblockfactory input on the stack reader" << std::endl;
	_stackReader->setInput("factory", _blockFactory);
}

void
SliceGuarantor::onSliceStoreSet(const pipeline::Modified&)
{
	LOG_DEBUG(sliceguarantorlog) << "Registering SliceStore input on BlockSliceStoreNode" << std::endl;	
	_blockSliceStore->setInput("store", _sliceStore);
}

void
SliceGuarantor::guaranteeSlices()
{
	if (!_block->setSlicesFlag(true))
	{
		LOG_DEBUG(sliceguarantorlog) << "The given block has not yet had slices extracted" << std::endl;
		LOG_DEBUG(sliceguarantorlog) << "Init'ing local variables" << std::endl;
		int zMin = _block->location()->z;
		boost::shared_ptr<SlicesCollector> slicesCollector = boost::make_shared<SlicesCollector>();
		boost::shared_ptr<ImageExtractor> sliceImageExtractor = boost::make_shared<ImageExtractor>();
		boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
		std::set<boost::shared_ptr<Block> > submitBlocks;
		
		LOG_DEBUG(sliceguarantorlog) << "Set ImageExtractor Stack Input" << std::endl;
		
		sliceImageExtractor->setInput("stack", _stackReader->getOutput());

		LOG_DEBUG(sliceguarantorlog) << "Set SlicesCollector Block Input" << std::endl;
		
		slicesCollector->setInput("block", _block);

		LOG_DEBUG(sliceguarantorlog) << "Setting up a SliceCollector" << std::endl;
		// Collect all Slice's
		for (unsigned int i = 0; i < _block->size()->z; ++i)
		{
			unsigned int z = i + zMin;
			boost::shared_ptr<ProcessNode> sliceExtractor =
				boost::make_shared<SliceExtractor<unsigned char> >(z);

			LOG_DEBUG(sliceguarantorlog) << "Setting up a SliceExtractor for i " << i
				<< ", z " << z << std::endl;

			LOG_DEBUG(sliceguarantorlog) << "Setting membrane input" << std::endl;
			sliceExtractor->setInput("membrane", sliceImageExtractor->getOutput(i));
			
			LOG_DEBUG(sliceguarantorlog) << "Setting explanation input" << std::endl;
			sliceExtractor->setInput("force explanation", _forceExplanation);

			LOG_DEBUG(sliceguarantorlog) << "Pushing output as input to SlicesCollector" << std::endl;
			slicesCollector->addInput("slices", sliceExtractor->getOutput("slices"));
		}
		
		LOG_DEBUG(sliceguarantorlog) << "Checking slice wholeness" << std::endl;
		//Separate Whole slices from fractured slices
		foreach(boost::shared_ptr<Slice> slice, *(slicesCollector->getSlices()))
		{
			checkWhole(slice, submitBlocks);
			slices->add(slice);
		}
		
		//_blockSliceStore->setInput("whole", boost::make_shared<pipeline::Wrap<bool> >(true));
		LOG_DEBUG(sliceguarantorlog) << "Setting slice input on SliceStore" << std::endl;
		
		_blockSliceStore->setInput("slice in", slices);
		
		LOG_DEBUG(sliceguarantorlog) << "Storing slices" << std::endl;
		_blockSliceStore->storeSlices();
	}
}

void
SliceGuarantor::checkWhole(const boost::shared_ptr<Slice>& slice,
						   std::set<boost::shared_ptr<Block> >& blocksToSubmit) const
{
	//TODO: figure out the best way to maintain a map from a slice to its bordering Blocks.
	// Check whether the slice's bounding box touches the Block boundary
	util::rect<int> sliceBound = slice->getComponent()->getBoundingBox();
	boost::shared_ptr<point3<int> > blockLocation = _block->location();
	boost::shared_ptr<point3<int> > blockSize = _block->size();
	int borderX = 0, borderY = 0;
	
	if (sliceBound.minX <= blockLocation->x)
	{
		borderX = -1;
	}
	else if (sliceBound.maxX >= blockLocation->x + blockSize->x)
	{
		borderX = 1;
	}
	
	if (sliceBound.minY <= blockLocation->y)
	{
		borderY = -1;
	}
	else if (sliceBound.maxY >= blockLocation->y + blockSize->y)
	{
		borderY = 1;
	}
	
	if(borderX || borderY)
	{
		slice->setWhole(false);
		if (_parameters->guaranteeAllSlices)
		{
			if (borderX)
			{
				blocksToSubmit.insert(
					_block->getManager()->blockAtOffset(*_block, util::ptrTo(borderX, 0, 0)));
			}
			
			if (borderY)
			{
				blocksToSubmit.insert(
					_block->getManager()->blockAtOffset(*_block, util::ptrTo(0, borderY, 0)));
			}
		}
		else
		{
			// Check this slice against incomplete slices in neighboring blocks.
		}
	}
	else
	{
		slice->setWhole(true);
	}
}


SliceGuarantor::SlicesCollector::SlicesCollector()
{
	registerInput(_block, "block");
	registerInputs(_multiSlices, "slices");
}

boost::shared_ptr<Slices>
SliceGuarantor::SlicesCollector::getSlices()
{
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	foreach (boost::shared_ptr<Slices> zSlices, _multiSlices)
	{
		slices->addAll(*zSlices);
	}
	return slices;
}


