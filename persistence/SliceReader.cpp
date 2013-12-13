#include "SliceReader.h"
#include <sopnet/block/Block.h>
#include <boost/unordered_set.hpp>

#include <util/Logger.h>
logger::LogChannel slicereaderlog("slicereaderlog", "[SliceReader] ");

SliceReader::SliceReader()
{
	registerInput(_box, "box");
	registerInput(_store, "store");
	registerInput(_blockManager, "block manager");
	registerOutput(_slices, "slices");
	registerOutput(_constraints, "linear constraints");
}

void SliceReader::updateOutputs()
{
	boost::unordered_set<Slice> sliceSet;
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	
	boost::shared_ptr<Blocks> blocks = _blockManager->blocksInBox(_box);
	
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		boost::shared_ptr<Slices> blockSlices = _store->retrieveSlices(block);
		foreach (boost::shared_ptr<Slice> slice, *blockSlices)
		{
			if (!sliceSet.count(*slice))
			{
				slices->add(slice);
				sliceSet.insert(*slice);
			}
		}
		slices->addConflictsFromSlices(*blockSlices);
	}
	
	*_slices = *slices;
	*_constraints = *(_store->retrieveConstraints(slices));
}
