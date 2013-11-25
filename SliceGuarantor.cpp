#include "SliceGuarantor.h"

#include <imageprocessing/ImageExtractor.h>
#include <sopnet/sopnet/slices/SliceExtractor.h>
#include <sopnet/sopnet/slices/Slice.h>
#include <util/rect.hpp>

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

SliceGuarantor::SliceGuarantor() :
	_stackReader(boost::make_shared<ImageBlockStackReader>()),
	_blockSliceStore(boost::make_shared<BlockSliceStoreNode>())
{
	registerInput(_block, "block");
	registerInput(_sliceStore, "store");
	registerInput(_blockFactory, "block factory");
	registerInput(_forceExplanation, "force explanation");
	registerInput(_parameters, "parameters");
	
	_stackReader->setInput("factory", _blockFactory);
	_stackReader->setInput("block", _block);
	
	_blockSliceStore->addInput("block", _block);
	_blockSliceStore->setInput("store", _sliceStore);
}

void
SliceGuarantor::guaranteeSlices()
{
	if (!_block->setSlicesFlag(true))
	{
		int zMin = _block->location()->z;
		boost::shared_ptr<SlicesCollector> slicesCollector = boost::make_shared<SlicesCollector>();
		boost::shared_ptr<ProcessNode> sliceImageExtractor = boost::make_shared<ImageExtractor>();
		boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
		std::set<boost::shared_ptr<Block> > submitBlocks;
		
		sliceImageExtractor->setInput(_stackReader->getOutput());

		slicesCollector->setInput("block", _block);
		
		// Collect all Slice's
		for (unsigned int i = 0; i < _block->size()->z; ++i)
		{
			unsigned int z = i + zMin;
			boost::shared_ptr<ProcessNode> sliceExtractor =
				boost::make_shared<SliceExtractor<unsigned char> >(z);
			
			sliceExtractor->setInput("membrane", sliceImageExtractor->getInput(i));
			sliceExtractor->setInput("force explanation", _forceExplanation);
			
			slicesCollector->addInput("slices", sliceExtractor->getOutput("slices"));
		}
		
		//Separate Whole slices from fractured slices
		foreach(boost::shared_ptr<Slice> slice, *(slicesCollector->getSlices()))
		{
			checkWhole(slice, submitBlocks);
			slices->add(slice);
		}
		
		_blockSliceStore->setInput("whole", boost::make_shared<pipeline::Wrap<bool> >(true));
		_blockSliceStore->setInput("slice in", slices);
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
