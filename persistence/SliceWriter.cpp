#include "SliceWriter.h"

#include <boost/shared_ptr.hpp>
#include <sopnet/slices/Slice.h>
#include <util/foreach.h>

SliceWriter::SliceWriter()
{
	registerInput(_blocks, "blocks");
	registerInput(_slices, "slices");
	registerInput(_store, "store");
	registerInput(_conflictSets, "conflict sets");
}

void
SliceWriter::writeSlices()
{
	updateInputs();
	
	foreach (boost::shared_ptr<Block> block, *_blocks)
	{
		pipeline::Value<Slices> blockSlices = collectSlicesByBlocks(block);
		boost::shared_ptr<ConflictSets> slicesConflictSets = collectConflictBySlices(blockSlices);

		_store->associate(blockSlices, pipeline::Value<Block>(*block));
		_store->storeConflict(slicesConflictSets);
	}
}

boost::shared_ptr<ConflictSets>
SliceWriter::collectConflictBySlices(const pipeline::Value<Slices> slices)
{
	//Write me!
}

pipeline::Value<Slices>
SliceWriter::collectSlicesByBlocks(const boost::shared_ptr<Block> block)
{
	//Write me!
}
