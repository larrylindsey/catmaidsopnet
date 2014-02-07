#include "SliceReader.h"
#include <sopnet/block/Block.h>


#include <util/Logger.h>
logger::LogChannel slicereaderlog("slicereaderlog", "[SliceReader] ");

SliceReader::SliceReader()
{
	registerInput(_box, "box", pipeline::Optional);
	registerInput(_blocks, "blocks", pipeline::Optional);
	registerInput(_store, "store");
	registerInput(_blockManager, "block manager");
	registerOutput(_slices, "slices");
	
	_box.registerBackwardCallback(&SliceReader::onBoxSet, this);
	_blocks.registerBackwardCallback(&SliceReader::onBlocksSet, this);
}

void SliceReader::onBoxSet(const pipeline::InputSetBase& )
{
	_sourceIsBox = true;
}

void SliceReader::onBlocksSet(const pipeline::InputSetBase& )
{
	_sourceIsBox = false;
	setInput("block manager", _blocks->getManager());
}



void SliceReader::updateOutputs()
{
	boost::unordered_set<Slice> sliceSet;
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	boost::shared_ptr<Slices> parentSlices;
	boost::shared_ptr<Blocks> blocks;
	bool fullHouse = false;

	if (!_blocks && !_box)
	{
		LOG_ERROR(slicereaderlog) << "Need either box or blocks, neither was set" << std::endl;
		*_slices = *slices;
		return;
	}
	else if (_sourceIsBox)
	{
		LOG_DEBUG(slicereaderlog) << "Blocks derived from box input" << std::endl;
		blocks = _blockManager->blocksInBox(_box);
	}
	else
	{
		LOG_DEBUG(slicereaderlog) << "Using blocks input directly" << std::endl;
		blocks = _blocks;
	}
	
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
	}
	
	// In addition to the Slices contained in this block, fetch any Slice that is a descendant of
	// a Slice in this block.
	parentSlices = slices;
	
	while (!fullHouse)
	{
		boost::shared_ptr<Slices> childSlices = boost::make_shared<Slices>();
		fullHouse = !fetchChildren(parentSlices, childSlices);
		slices->addAll(childSlices);
		parentSlices = childSlices;
	}

	*_slices = *slices;
}

bool
SliceReader::fetchChildren(const boost::shared_ptr<Slices>& slicesIn,
						   const boost::shared_ptr<Slices>& slicesOut)
{

}


