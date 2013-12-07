#include "SliceGuarantor.h"

#include <imageprocessing/ImageExtractor.h>
#include <sopnet/sopnet/slices/SliceExtractor.h>
#include <sopnet/sopnet/slices/Slice.h>
#include <catmaidsopnet/persistence/SliceWriter.h>
#include <util/rect.hpp>
#include <util/Logger.h>
#include <pipeline/Value.h>

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
	registerInput(_block, "block");
	registerInput(_sliceStore, "store");
	registerInput(_blockFactory, "block factory");
	registerInput(_forceExplanation, "force explanation");
	registerInput(_parameters, "parameters");
	registerInput(_maximumArea, "maximum area");
	//registerInput(_mserParameters, "mser parameters");
	registerOutput(_neighborBlocks, "neighbor blocks");
	registerOutput(_count, "count");
}

void
SliceGuarantor::updateOutputs()
{
	if (!_block->setSlicesFlag(true))
	{
		LOG_DEBUG(sliceguarantorlog) << "The given block has not yet had slices extracted" << std::endl;
		LOG_DEBUG(sliceguarantorlog) << "Init'ing local variables" << std::endl;
		
		bool guaranteed = false;
		
		int wholeCount = 0, extractArea;
		
		boost::shared_ptr<Block> inputBlock = _block;
		
		boost::shared_ptr<Blocks> extractBlocks = boost::make_shared<Blocks>(inputBlock);
		
		pipeline::Value<SliceStoreResult> sliceWriteCount;
		
		boost::shared_ptr<SliceWriter> sliceWriter = boost::make_shared<SliceWriter>();
		boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
		
		
		extractBlocks->dilateXY();
		extractArea = extractBlocks->size().x * extractBlocks->size().y;
		
		LOG_DEBUG(sliceguarantorlog) << "Extract area " << extractArea << ". Maximum area " << *_maximumArea << std::endl;
		
		while (!guaranteed && extractBlocks->size().x * extractBlocks->size().y < *_maximumArea)
		{
			slices->clear();
			LOG_DEBUG(sliceguarantorlog) << "Extracting from box at " << extractBlocks->location() << " with size " << extractBlocks->size() << std::endl;
			guaranteed = guaranteeSlices(extractBlocks, slices);
			if (guaranteed)
			{
				LOG_DEBUG(sliceguarantorlog) << "success!" << std::endl;
			}
			else
			{
				LOG_DEBUG(sliceguarantorlog) << "There were unwhole slices in the guarantee block. Trying again" << std::endl;
			}
		}
		
		
		//_blockSliceStore->setInput("whole", boost::make_shared<pipeline::Wrap<bool> >(true));
		LOG_DEBUG(sliceguarantorlog) << "Setting slice input on SliceStore" << std::endl;
		
		sliceWriter->setInput("slices", slices);
		sliceWriter->setInput("block", _block);
		sliceWriter->setInput("store", _sliceStore);
		
		LOG_DEBUG(sliceguarantorlog) << "Storing " << slices->size() << " slices, " << wholeCount
			<< " of which are whole." << std::endl;
		
		LOG_DEBUG(sliceguarantorlog) << "Storing neighbor blocks" << std::endl;
		// Force sliceWriter to run updateOutputs
		sliceWriteCount = sliceWriter->getOutput();
		*_count = *sliceWriteCount;
		
		// Update neighborBlocks output.
		//*_neighborBlocks = *neighborBlocks;
		
	}
}

bool
SliceGuarantor::guaranteeSlices(const boost::shared_ptr<Blocks>& extractBlocks,
								const boost::shared_ptr<Slices>& slices)
{
	LOG_DEBUG(sliceguarantorlog) << "Setting up mini pipeline" << std::endl;

	int zMin = extractBlocks->location().z;
	bool okSlices = true;
	
	boost::shared_ptr<ImageExtractor> sliceImageExtractor = boost::make_shared<ImageExtractor>();
	boost::shared_ptr<ImageBlockStackReader> stackReader = boost::make_shared<ImageBlockStackReader>();
	boost::shared_ptr<Blocks> nbdBlocks = boost::make_shared<Blocks>(extractBlocks);
	
	stackReader->setInput("block", extractBlocks);
	stackReader->setInput("factory", _blockFactory);

	sliceImageExtractor->setInput("stack", stackReader->getOutput());

	// Collect all Slice's
	for (unsigned int i = 0; i < extractBlocks->size().z; ++i)
	{
		unsigned int z = i + zMin;
		boost::shared_ptr<ProcessNode> sliceExtractor =
			boost::make_shared<SliceExtractor<unsigned char> >(i);
		boost::shared_ptr<Slices> guaranteedSlices;
		pipeline::Value<Slices> valueSlices;
		
		
		LOG_DEBUG(sliceguarantorlog) << "Setting up a SliceExtractor for i " << i
			<< ", z " << z << std::endl;

		LOG_DEBUG(sliceguarantorlog) << "Setting membrane input" << std::endl;
		sliceExtractor->setInput("membrane", sliceImageExtractor->getOutput(i));
		
		LOG_DEBUG(sliceguarantorlog) << "Setting explanation input" << std::endl;
		sliceExtractor->setInput("force explanation", _forceExplanation);

		if (_mserParameters)
		{
			LOG_DEBUG(sliceguarantorlog) << "Setting MSER Parameters" << std::endl;
			sliceExtractor->setInput("mser parameters", _mserParameters);
		}
		
		LOG_DEBUG(sliceguarantorlog) << "Collecting output" << std::endl;
		valueSlices = sliceExtractor->getOutput("slices");
		
		foreach (boost::shared_ptr<Slice> slice, *valueSlices)
		{
			if (_block->overlaps(slice->getComponent()))
			{
				slices->add(slice);
			}
		}
		LOG_DEBUG(sliceguarantorlog) << "Pushing conflicts into output" << std::endl;
		slices->addConflictsFromSlices(*valueSlices);
	}
	
	// Slices are extracted in [0 0 w h]. Translate them to Box coordinates.
	foreach(boost::shared_ptr<Slice> slice, *slices)
	{
		util::point<int> translate(extractBlocks->location().x, extractBlocks->location().y);
		slice->translate(translate);
	}
	
	
	foreach(boost::shared_ptr<Slice> slice, *slices)
	{
		
		checkWhole(slice, extractBlocks, nbdBlocks);
	}
	
	util::point3<unsigned int> nbdSize = nbdBlocks->size();
	util::point3<unsigned int> extractSize = extractBlocks->size();
	
	if (nbdSize.x > extractSize.x && nbdSize.y > extractSize.y)
	{
		extractBlocks->addAll(nbdBlocks->getBlocks());
		return false;
	}
	else
	{
		return true;
	}
}


void
SliceGuarantor::checkWhole(const boost::shared_ptr<Slice>& slice,
						   const boost::shared_ptr<Blocks>& extractBlocks,
						   const boost::shared_ptr<Blocks>& nbdBlocks) const
{
	//TODO: figure out the best way to maintain a map from a slice to its bordering Blocks.
	// Check whether the slice's bounding box touches the Block boundary
	util::rect<int> sliceBound = slice->getComponent()->getBoundingBox();
	point3<unsigned int> blockLocation = extractBlocks->location();
	point3<unsigned int> blockSize = extractBlocks->size();
	int borderX = 0, borderY = 0;
	
	if (sliceBound.minX <= blockLocation.x)
	{
		borderX = -1;
	}
	else if (sliceBound.maxX >= blockLocation.x + blockSize.x)
	{
		borderX = 1;
	}
	
	if (sliceBound.minY <= blockLocation.y)
	{
		borderY = -1;
	}
	else if (sliceBound.maxY >= blockLocation.y + blockSize.y)
	{
		borderY = 1;
	}

	if (borderX)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(borderX, 0, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}

	if (borderY)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(0, borderY, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}
}
