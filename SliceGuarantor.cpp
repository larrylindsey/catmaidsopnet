#include "SliceGuarantor.h"

#include <imageprocessing/ImageExtractor.h>
#include <sopnet/sopnet/slices/SliceExtractor.h>
#include <sopnet/sopnet/slices/Slice.h>
#include <util/rect.hpp>
#include <util/Logger.h>
#include <pipeline/Value.h>

logger::LogChannel sliceguarantorlog("sliceguarantorlog", "[SliceGuarantor] ");

SliceGuarantor::SliceGuarantor()
{
	registerInput(_blocks, "blocks");
	registerInput(_sliceStore, "store");	
	registerInput(_blockFactory, "block factory");
	registerInput(_maximumArea, "maximum area");
	registerInput(_mserParameters, "mser parameters", pipeline::Optional);

	registerOutput(_count, "count");
}

void
SliceGuarantor::updateOutputs()
{
	// We're given a Box. Find all Blocks overlapping that Box.
	boost::shared_ptr<BlockManager> blockManager = _blocks->getManager();
	boost::shared_ptr<Blocks> guaranteeBlocks = _blocks;
	boost::shared_ptr<SliceStoreResult> count = boost::make_shared<SliceStoreResult>();
	bool slicesExtracted = true;

	// Expand in z. Segment guarantor needs +z boundary, if it exists.
	guaranteeBlocks->expand(util::ptrTo(0, 0, 1));
	
	LOG_DEBUG(sliceguarantorlog) << "Asked to guarantee " << guaranteeBlocks->length() << " blocks." << std::endl;
	LOG_DEBUG(sliceguarantorlog) << "Request size: " << _blocks->size() << std::endl;
	LOG_DEBUG(sliceguarantorlog) << "Guarantee size: " << guaranteeBlocks->size()<< std::endl;
	
	// Check to see whether each block in guaranteeBlocks has already had its slices extracted.
	// If this is the case, we have no work to do
	foreach (boost::shared_ptr<Block> block, *guaranteeBlocks)
	{
		slicesExtracted &= block->setSlicesFlag(true);
	}
	
	
	if (slicesExtracted)
	{
		LOG_DEBUG(sliceguarantorlog) << "All blocks have already been extracted" << std::endl;
		return;
	}
	
	LOG_DEBUG(sliceguarantorlog) << "The given blocks have not yet had slices extracted" <<
		std::endl;
	LOG_DEBUG(sliceguarantorlog) << "Init'ing local variables" << std::endl;
	
	bool guaranteed = false;
	int extractArea;
	
	// Extract blocks is a superset of guarantee blocks.
	boost::shared_ptr<Blocks> extractBlocks = boost::make_shared<Blocks>(guaranteeBlocks);

	// Slices and ComponentTrees extracted from the image underlying the requested area.
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	boost::shared_ptr<ComponentTrees> trees = boost::make_shared<ComponentTrees>();

	// Assume that some Slice's will overlap the guarantee box's boundary, so dilate.
	extractBlocks->dilateXY();
	extractArea = extractBlocks->size().x * extractBlocks->size().y;
	
	LOG_DEBUG(sliceguarantorlog) << "Extract area " << extractArea << ". Maximum area " <<
		*_maximumArea << std::endl;


	// Extract slices from the extract area.
	// The extract area is expanded until all Slice's extracted from the guarantee area are
	// whole.

	while (!guaranteed && extractBlocks->size().x * extractBlocks->size().y < *_maximumArea)
	{
		slices->clear();
		trees->clear();
		
		LOG_DEBUG(sliceguarantorlog) << "Extracting from box at " << extractBlocks->location()
			<< " with size " << extractBlocks->size() << std::endl;
		guaranteed = guaranteeSlices(extractBlocks, guaranteeBlocks, slices, trees);
		if (guaranteed)
		{
			LOG_DEBUG(sliceguarantorlog) << "success!" << std::endl;
		}
		else
		{
			LOG_DEBUG(sliceguarantorlog) <<
				"There were unwhole slices in the guarantee box. Trying again" << std::endl;
		}
	}

	// If the extract area is grel maximumArea, then we probably have some not-fully-extracted
	// Slice's, so tell the user about it.
	if (extractBlocks->size().x * extractBlocks->size().y >= *_maximumArea)
	{
		LOG_ERROR(sliceguarantorlog) << "Extraction size " << extractBlocks->size() <<
			" has a greater area than the maximum " << *_maximumArea <<
			". Some large Slices have not been fully extracted." << std::endl;
	}
	
	// Write the slices to the store.
	count = writeSlices(guaranteeBlocks, extractBlocks, slices, trees);
	
	LOG_DEBUG(sliceguarantorlog) << "Wrote " << count->count << " slices total" << std::endl;
	*_count = *count;
}

boost::shared_ptr<SliceStoreResult>
SliceGuarantor::writeSlices(const boost::shared_ptr<Blocks>& guaranteeBlocks,
							const boost::shared_ptr<Blocks>& extractBlocks,
							const boost::shared_ptr<Slices>& slices,
							const boost::shared_ptr<ComponentTrees>& trees)
{
	//TODO: Consider migrating this behavior to the SliceWriter.
	// * possible to reduce the number of db hits
	// * it seems philosophically "cleaner" to move the nitty-gritty of slice storage to the class
	//      that handles it.
	
	boost::shared_ptr<SliceStoreResult> count = boost::make_shared<SliceStoreResult>();
	
	
	// Slice and LinearConstraints persistence.
	boost::shared_ptr<SliceWriter> sliceWriter = boost::make_shared<SliceWriter>();
	
	// A collection of slices that overlap the guarantee blocks. Used in a later step.
	boost::shared_ptr<Slices> guaranteeBlockSlices = boost::make_shared<Slices>();
	// The whole "family" of the above slices, also used later.
	boost::shared_ptr<Slices> sliceFamily;
	
	
	/*
	 * write slices in two steps
	 * 1) Write the slices that overlap any of the gauranteeBlocks
	 * 2) Write the slices that are descendants of those in step 1, that
	 *    were not written during step 1.
	 */
	
	sliceWriter->setInput("store", _sliceStore);
	
	// Write the Slices in the guarantee blocks to the store.
	foreach (boost::shared_ptr<Block> block, *guaranteeBlocks)
	{
		writeSlicesHelper(block, slices, trees, sliceWriter, guaranteeBlockSlices, count);
	}
	
	// Collect the descendants of the slices we just wrote, and write them to the store,
	// regardless of whether they are in the guarantee blocks.
	sliceFamily = collectDescendants(guaranteeBlockSlices, trees);
	
	foreach (boost::shared_ptr<Block> block, *extractBlocks)
	{
		if (!guaranteeBlocks->contains(block))
		{
			boost::shared_ptr<Slices> emptySlices;
			writeSlicesHelper(block, sliceFamily, trees, sliceWriter, emptySlices, count);
		}
		
	}
	
	return count;
}


void SliceGuarantor::writeSlicesHelper(const boost::shared_ptr<Block>& block,
									   const boost::shared_ptr<Slices>& slices,
									   const boost::shared_ptr<ComponentTrees>& trees,
									   const boost::shared_ptr<SliceWriter>& sliceWriter,
									   const boost::shared_ptr<Slices>& writtenSlices,
									   const boost::shared_ptr<SliceStoreResult>& count)
{
	// Value used to force updateOutputs on the slice writer.
	pipeline::Value<SliceStoreResult> sliceWriteCount;
	LOG_ALL(sliceguarantorlog) << "Collecting slices assocated with block at location "
		<< block->location() << std::endl;
	boost::shared_ptr<Slices> blockSlices = boost::make_shared<Slices>();
	
	// Collect only the slices that overlap the guarantee box.
	
	foreach(boost::shared_ptr<Slice> slice, *slices)
	{
		if (block->overlaps(slice->getComponent()))
		{
			blockSlices->add(slice);
			if (writtenSlices)
			{
				writtenSlices->add(slice);
			}
		}
	}
	
	sliceWriter->setInput("slices", blockSlices);
	sliceWriter->setInput("component trees", trees);
	sliceWriter->setInput("block", block);

	// Force sliceWriter to run updateOutputs
	sliceWriteCount = sliceWriter->getOutput();
	count->count += sliceWriteCount->count;
	
	LOG_ALL(sliceguarantorlog) << "Wrote " << sliceWriteCount->count << " slices to Block" <<
		std::endl;
}

void
SliceGuarantor::guaranteeSlices(const boost::shared_ptr<Blocks> extractBlocks,
								const boost::shared_ptr<Blocks> guaranteeBlocks,
								const boost::shared_ptr<Slices> guaranteeSlices,
								const boost::shared_ptr<LinearConstraints> constraints)
{
	LOG_DEBUG(sliceguarantorlog) << "Setting up mini pipeline" << std::endl;

	int zMin = extractBlocks->location().z;
	
	boost::shared_ptr<ImageExtractor> sliceImageExtractor = boost::make_shared<ImageExtractor>();
	boost::shared_ptr<ImageBlockStackReader> stackReader = boost::make_shared<ImageBlockStackReader>();
	boost::shared_ptr<Blocks> nbdBlocks = boost::make_shared<Blocks>(extractBlocks);
	util::point<int> translate(extractBlocks->location().x, extractBlocks->location().y);
	
	constraints->clear();
	guaranteeSlices->clear();
	
	stackReader->setInput("block", extractBlocks);
	stackReader->setInput("factory", _blockFactory);

	sliceImageExtractor->setInput("stack", stackReader->getOutput());

	// Collect all Slice's
	for (unsigned int i = 0; i < extractBlocks->size().z; ++i)
	{
		unsigned int z = i + zMin;
		boost::shared_ptr<ProcessNode> sliceExtractor =
			boost::make_shared<SliceExtractor<unsigned char> >(z);
		boost::shared_ptr<Slices> guaranteedSlices;
		pipeline::Value<Slices> extractedSlices;
		pipeline::Value<ComponentTree> valueTree;
		pipeline::Value<LinearConstraints> sliceConflictSets;
		boost::shared_ptr<pipeline::Wrap<bool> > yep =
			boost::make_shared<pipeline::Wrap<bool> >(true);

		sliceExtractor->setInput("membrane", sliceImageExtractor->getOutput(i));
		sliceExtractor->setInput("force explanation", yep);

		if (_mserParameters)
		{
			sliceExtractor->setInput("mser parameters", _mserParameters);
		}
		
		extractedSlices = sliceExtractor->getOutput("slices");
		sliceConflictSets = sliceExtractor->getOutput("linear constraints");
		
		LOG_DEBUG(sliceguarantorlog) << "Extracted " << extractedSlices->size() << " slices" << std::endl;
		
		// Slices are extracted in [0 0 w h]. Translate them to Box coordinates.
		LOG_DEBUG(sliceguarantorlog) << "Translating slices to request coordinates" << std::endl;
		foreach(boost::shared_ptr<Slice> slice, *extractedSlices)
		{
			slice->translate(translate);

			if (guaranteeBlocks->overlaps(slice->getComponent()))
			{
				guaranteeSlices->add(slice);
			}
		}
		
	}
	
	LOG_DEBUG(sliceguarantorlog) << "Checking " << guaranteeSlices->size() << " slices for wholeness" <<
		std::endl;
	foreach(boost::shared_ptr<Slice> slice, *guaranteeSlices)
	{
		checkWhole(slice, extractBlocks, nbdBlocks);
	}
	
	util::point3<unsigned int> nbdSize = nbdBlocks->size();
	util::point3<unsigned int> extractSize = extractBlocks->size();
	
	if (nbdSize.x > extractSize.x || nbdSize.y > extractSize.y)
	{
		LOG_DEBUG(sliceguarantorlog) << "Need to expand to at least " << nbdSize <<
			" in order to extract whole slices" << std::endl;
		extractBlocks->addAll(nbdBlocks->getBlocks());
		return false;
	}
	else
	{
		LOG_DEBUG(sliceguarantorlog) << "We're done!" << std::endl;
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
	
	if (sliceBound.minX <= blockLocation.x)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(-1, 0, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}
	else if (sliceBound.maxX >= blockLocation.x + blockSize.x - 1)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(1, 0, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}
	
	if (sliceBound.minY <= blockLocation.y)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(0, -1, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}
	else if (sliceBound.maxY >= blockLocation.y + blockSize.y - 1)
	{
		Blocks expandBlocks = Blocks(*extractBlocks);
		expandBlocks.expand(util::ptrTo(0, 1, 0));
		nbdBlocks->addAll(expandBlocks.getBlocks());
	}

}


boost::shared_ptr<Slices>
SliceGuarantor::collectDescendants(const boost::shared_ptr<Slices>& slices,
								   const boost::shared_ptr<ComponentTrees>& trees)
{
	ComponentSliceMap componentSliceMap;
	ComponentTrees::iterator ctit;
	
	boost::shared_ptr<Slices> descendants = boost::make_shared<Slices>();

	descendants->addAll(*slices);
	
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		componentSliceMap[*(slice->getComponent())] = slice;
	}
	
	for (ctit = trees->begin(); ctit != trees->end(); ++ctit)
	{
		boost::shared_ptr<ComponentTree::Node> rootNode = ctit->second->getRoot();
		
		foreach (boost::shared_ptr<ComponentTree::Node> node, rootNode->getChildren())
		{
			getChildren(componentSliceMap, node, descendants);
		}
	}
	
	return descendants;
}

void
SliceGuarantor::getChildren(SliceGuarantor::ComponentSliceMap& componentSliceMap,
							const boost::shared_ptr<ComponentTree::Node>& node,
							const boost::shared_ptr<Slices>& descendants)
{
	boost::shared_ptr<Slice> parentSlice = componentSliceMap[*(node->getComponent())];
	
	if (parentSlice)
	{
		foreach (boost::shared_ptr<ComponentTree::Node> childNode, node->getChildren())
		{
			boost::shared_ptr<Slice> childSlice = componentSliceMap[*(childNode->getComponent())];
			if (childSlice)
			{
				descendants->add(childSlice);
				getChildren(componentSliceMap, childNode, descendants);
			}
		}
	}
}


