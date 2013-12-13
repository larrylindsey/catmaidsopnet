#include "SegmentGuarantor.h"
#include <util/Logger.h>
#include <catmaidsopnet/persistence/SliceReader.h>
#include <catmaidsopnet/persistence/SegmentWriter.h>
#include <sopnet/segments/SegmentExtractor.h>
#include <pipeline/Value.h>

logger::LogChannel segmentguarantorlog("segmentguarantorlog", "[SegmentGuarantor] ");

SegmentGuarantor::SegmentGuarantor()
{
	registerInput(_box, "box");
	registerInput(_blockManager, "block manager");
	registerInput(_segmentStore, "segment store");
	registerInput(_sliceStore, "slice store");
	
	registerOutput(_result, "count");
}


void SegmentGuarantor::guaranteeSegments(
	const boost::shared_ptr<Blocks>& guaranteeBlocks,
	const boost::shared_ptr<Blocks>& sliceBlocks)
{
	boost::shared_ptr<SliceReader> sliceReader = boost::make_shared<SliceReader>();
	pipeline::Value<Slices> slices;
	pipeline::Value<LinearConstraints> sliceConstraints;
	boost::shared_ptr<LinearConstraints> segmentConstraints;
	boost::shared_ptr<Segments> segments;
	boost::shared_ptr<SegmentWriter> segmentWriter;
	pipeline::Value<SegmentStoreResult> result;
	
	unsigned int zBegin = guaranteeBlocks->location().z;
	unsigned int zEnd = zBegin + guaranteeBlocks->size().z;
	
	sliceReader->setInput("box", sliceBlocks);
	sliceReader->setInput("store", _sliceStore);
	sliceReader->setInput("block manager", _blockManager);
	
	slices = sliceReader->getOutput("slices");
	sliceConstraints = sliceReader->getOutput("linear constraints");
	
	LOG_DEBUG(segmentguarantorlog) << "Read " << slices->size() << " slices and " <<
		sliceConstraints->size() << " constraints" << std::endl;
	
	for (unsigned int z = zBegin; z < zEnd; ++z)
	{
		pipeline::Value<LinearConstraints> extractedConstraints;
		pipeline::Value<Segments> extractedSegments;
		
		boost::shared_ptr<SegmentExtractor> extractor = boost::make_shared<SegmentExtractor>();
		boost::shared_ptr<Slices> prevSlices = collectSlicesByZ(slices, z);
		boost::shared_ptr<Slices> nextSlices = collectSlicesByZ(slices, z);
		boost::shared_ptr<LinearConstraints> prevConstraints = 
			collectConstraints(prevSlices, sliceConstraints);
		boost::shared_ptr<LinearConstraints> nextConstraints = 
			collectConstraints(nextSlices, sliceConstraints);
		
		extractor->setInput("previous slices", prevSlices);
		extractor->setInput("next slices", nextSlices);
		extractor->setInput("previous linear constraints", prevConstraints);
		extractor->setInput("next linear constraints", nextConstraints);
		
		extractedSegments = extractor->getOutput("segments");
		extractedConstraints = extractor->getOutput("linear constraints");
		
		segments->addAll(extractedSegments);
		segmentConstraints->addAll(*extractedConstraints);
	}
	
	segmentWriter->setInput("segments", segments);
	segmentWriter->setInput("box", guaranteeBlocks);
	segmentWriter->setInput("block manager", _blockManager);
	segmentWriter->setInput("store", _segmentStore);
	
	result = segmentWriter->getOutput("count");
	*_result = *result;
}


void SegmentGuarantor::updateOutputs()
{
	boost::shared_ptr<Blocks> guaranteeBlocks = _blockManager->blocksInBox(_box);
	boost::shared_ptr<Blocks> sliceBlocks = boost::make_shared<Blocks>(guaranteeBlocks);
	bool allExtracted = true;
	
	// Check whether this update needs to occur.
	foreach (boost::shared_ptr<Block> block, *guaranteeBlocks)
	{
		allExtracted = block->setSegmentsFlag(true) && allExtracted;
	}
	
	if (!allExtracted)
	{
		
		// We need the boundaries at +z, +/- x and +/- y to be complete.
		sliceBlocks->expand(util::ptrTo(0, 0, 1));
		sliceBlocks->dilateXY();
		
		// Check that we have slices in all of the necessary blocks.
		foreach (boost::shared_ptr<Block> block, *sliceBlocks)
		{
			if (!block->getSlicesFlag())
			{
				boost::shared_ptr<SegmentStoreResult> result = 
					boost::make_shared<SegmentStoreResult>();
				*_result = *result;
				LOG_DEBUG(segmentguarantorlog) << "Block at " << block->location() <<
					" has no slices. Cannot proceed" << std::endl;
				return;
			}
		}
		
		guaranteeSegments(guaranteeBlocks, sliceBlocks);
	}
	else
	{
		LOG_DEBUG(segmentguarantorlog) << "Segments already in cache for all requested Blocks" <<
			std::endl;
	}
	
}

bool
SegmentGuarantor::isAssociated(const LinearConstraint& constraint,
							   const boost::shared_ptr< Slices >& slices) const
{
	std::map<unsigned int, double>::const_iterator iter;
	for (iter = constraint.getCoefficients().begin();
			iter != constraint.getCoefficients().end(); ++iter)
	{
		foreach (boost::shared_ptr<Slice> slice, *slices)
		{
			if (iter->first == slice->getId())
			{
				return true;
			}
		}
	}
	
	return false;
}


boost::shared_ptr<LinearConstraints>
SegmentGuarantor::collectConstraints(const boost::shared_ptr<Slices>& slices,
								const boost::shared_ptr<LinearConstraints>& constraints) const
{
	boost::shared_ptr<LinearConstraints> restriction = boost::make_shared<LinearConstraints>();
	
	foreach (LinearConstraint constraint, *constraints)
	{
		if (isAssociated(constraint, slices))
		{
			restriction->add(constraint);
		}
	}
	
	return restriction;
	
}

boost::shared_ptr<Slices> SegmentGuarantor::collectSlicesByZ(
	const boost::shared_ptr< Slices >& slices, unsigned int z) const
{
	boost::shared_ptr<Slices> zSlices = boost::make_shared<Slices>();
	
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		if (slice->getId() == z)
		{
			zSlices->add(slice);
			zSlices->setConflicts(slice->getId(), slices->getConflicts(slice->getId()));
		}
	}
	
	return zSlices;
}

