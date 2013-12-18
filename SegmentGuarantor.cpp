#include "SegmentGuarantor.h"
#include <util/Logger.h>
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

boost::shared_ptr<Slices>
SegmentGuarantor::collectNecessarySlices(const boost::shared_ptr<SliceReader>& sliceReader,
										 const boost::shared_ptr< Blocks >& sliceBlocks)
{
	pipeline::Value<Slices> slices;
	boost::shared_ptr<Blocks> extraBlocks = boost::make_shared<Blocks>(sliceBlocks);
	boost::shared_ptr<Slices> ptrSlices; // For semi-explicit casting.
	
	// Read minimal slices for guaranteed box.
	sliceReader->setInput("blocks", sliceBlocks);
	sliceReader->setInput("store", _sliceStore);
	slices = sliceReader->getOutput("slices");
	
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		if (!extraBlocks->contains(slice->getComponent()->getBoundingBox()))
		{
			boost::shared_ptr<Box<> > sliceBox =
				boost::make_shared<Box<> >(slice->getComponent()->getBoundingBox(),
						sliceBlocks->location().z, sliceBlocks->size().z);
			extraBlocks->addAll(_blockManager->blocksInBox(sliceBox));
		}
	}
	
	sliceReader->setInput("blocks", extraBlocks);
	slices = sliceReader->getOutput("slices");
	
	ptrSlices = slices;
	
	return ptrSlices;
}


void SegmentGuarantor::guaranteeSegments(
	const boost::shared_ptr<Blocks>& guaranteeBlocks,
	const boost::shared_ptr<Blocks>& sliceBlocks)
{
	boost::shared_ptr<SliceReader> sliceReader = boost::make_shared<SliceReader>();
	boost::shared_ptr<LinearConstraints> emptyConstraints =
		boost::make_shared<LinearConstraints>();
	boost::shared_ptr<Slices> slices;
	//boost::shared_ptr<LinearConstraints> segmentConstraints;
	boost::shared_ptr<Segments> segments = boost::make_shared<Segments>();
	boost::shared_ptr<SegmentWriter> segmentWriter = boost::make_shared<SegmentWriter>();
	boost::shared_ptr<Blocks> extraBlocks;
	pipeline::Value<SegmentStoreResult> result;
	
	unsigned int zBegin = guaranteeBlocks->location().z;
	unsigned int zEnd = zBegin + guaranteeBlocks->size().z;
	
	slices = collectNecessarySlices(sliceReader, sliceBlocks);
	
	LOG_DEBUG(segmentguarantorlog) << "Read " << slices->size() << " slices." << std::endl;
	
	for (unsigned int z = zBegin; z < zEnd; ++z)
	{
		pipeline::Value<LinearConstraints> extractedConstraints;
		pipeline::Value<Segments> extractedSegments;
		
		boost::shared_ptr<SegmentExtractor> extractor = boost::make_shared<SegmentExtractor>();
		boost::shared_ptr<Slices> prevSlices = collectSlicesByZ(slices, z);
		boost::shared_ptr<Slices> nextSlices = collectSlicesByZ(slices, z + 1);
		
		extractor->setInput("previous slices", prevSlices);
		extractor->setInput("next slices", nextSlices);
		//extractor->setInput("previous linear constraints", emptyConstraints);
		//extractor->setInput("next linear constraints", emptyConstraints);
		
		extractedSegments = extractor->getOutput("segments");
		
		LOG_DEBUG(segmentguarantorlog) << "Extractor getOutput has returned" << std::endl;
		LOG_DEBUG(segmentguarantorlog) << "Got " << extractedSegments->size() << " segments" << std::endl;
		
		segments->addAll(extractedSegments);
		//segmentConstraints->addAll(*extractedConstraints);
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
		
		// We need the slices across the +z boundary, in order to ensure that we'll extract all
		// of the required segments.
		sliceBlocks->expand(util::ptrTo(0, 0, 1));

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
		if (slice->getSection() == z)
		{
			zSlices->add(slice);
			zSlices->setConflicts(slice->getId(), slices->getConflicts(slice->getId()));
		}
	}
	
	LOG_DEBUG(segmentguarantorlog) << "Collected " << zSlices->size() << " slices for z=" << z << std::endl;
	
	return zSlices;
}

