#include "SegmentWriter.h"

SegmentWriter::SegmentWriter()
{
	registerInput(_segments, "segments");
	registerInput(_box, "box");
	registerInput(_store, "store");
	registerInput(_blockManager, "block manager");
	
	registerOutput(_result, "count");
}


void SegmentWriter::updateOutputs()
{
	boost::shared_ptr<Blocks> blocks = _blockManager->blocksInBox(_box);
	boost::shared_ptr<SegmentStoreResult> result = boost::make_shared<SegmentStoreResult>();
	
	foreach(boost::shared_ptr<Block> block, *blocks)
	{
		foreach (boost::shared_ptr<EndSegment> segment, _segments->getEnds())
		{
			if (associated(segment, block))
			{
				_store->associate(segment, block);
				result->count += 1;
			}
		}
		
		foreach (boost::shared_ptr<ContinuationSegment> segment, _segments->getContinuations())
		{
			if (associated(segment, block))
			{
				_store->associate(segment, block);
				result->count += 1;
			}
		}
		
		foreach (boost::shared_ptr<BranchSegment> segment, _segments->getBranches())
		{
			if (associated(segment, block))
			{
				_store->associate(segment, block);
				result->count += 1;
			}
		}
	}
	
	*_result = *result;
}

bool
SegmentWriter::associated(const boost::shared_ptr<Segment>& segment,
						  const boost::shared_ptr<Block>& block)
{
	foreach (boost::shared_ptr<Slice> slice, segment->getSlices())
	{
		if (block->overlaps(slice->getComponent()))
		{
			return true;
		}
	}
	
	return false;
}
