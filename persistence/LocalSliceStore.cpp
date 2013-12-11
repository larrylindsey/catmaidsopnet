#include "LocalSliceStore.h"
#include <boost/make_shared.hpp>
#include <set>

#include <util/Logger.h>
logger::LogChannel localslicestorelog("localslicestorelog", "[LocalSliceStore] ");

LocalSliceStore::LocalSliceStore()
{	
	_sliceBlockMap = boost::make_shared<SliceBlockMap>();
	_blockSliceMap = boost::make_shared<BlockSliceMap>();
}


boost::shared_ptr<Blocks>
LocalSliceStore::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	if (_sliceBlockMap->count(*slice))
	{
		return (*_sliceBlockMap)[*slice];
	}
	else
	{
		boost::shared_ptr<Blocks> empty = boost::make_shared<Blocks>();
		return empty;
	}
}

void
LocalSliceStore::removeSlice(const boost::shared_ptr< Slice >& slice)
{
	boost::shared_ptr<Blocks> blocks = getAssociatedBlocks(slice);
	
	_sliceBlockMap->erase(*slice);
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		(*_blockSliceMap)[*block]->remove(slice);
	}
}

void
LocalSliceStore::disassociate(const boost::shared_ptr< Slice >& slice, const boost::shared_ptr<Block>& block)
{
	if (_sliceBlockMap->count(*slice))
	{
		(*_sliceBlockMap)[*slice]->remove(block);
	}
	if (_blockSliceMap->count(*block))
	{
		(*_blockSliceMap)[*block]->remove(slice);
	}
}

boost::shared_ptr<Slices>
LocalSliceStore::retrieveSlices(const boost::shared_ptr<Block>& block)
{
	boost::shared_ptr<Slices> slices;
	
	if (_blockSliceMap->count(*block))
	{
		slices = (*_blockSliceMap)[*block];
	}
	else
	{
		slices = boost::make_shared<Slices>();
	}

	return slices;
}


void
LocalSliceStore::mapBlockToSlice(const boost::shared_ptr< Block >& block, const boost::shared_ptr< Slice >& slice)
{
	// Place entry in block slice map
	boost::shared_ptr<Slices> slices;
	
	if (_blockSliceMap->count(*block))
	{
		slices = (*_blockSliceMap)[*block];
	}
	else
	{
		slices = boost::make_shared<Slices>();
		(*_blockSliceMap)[*block] = slices;
	}
	
	foreach (boost::shared_ptr<Slice> cSlice, *slices)
	{
		if (*cSlice == *slice)
		{
			LOG_DEBUG(localslicestorelog) << "BlockSliceMap already links Block " <<
				block->getId() << " to slice " << slice->getId() << std::endl;
			return;
		}
	}
	
	slices->add(slice);
}

void
LocalSliceStore::mapSliceToBlock(const boost::shared_ptr< Slice >& slice, const boost::shared_ptr< Block >& block)
{
	// Place entry in slice block map
	boost::shared_ptr<Blocks> blocks;
	
	if (_sliceBlockMap->count(*slice))
	{
		blocks = (*_sliceBlockMap)[*slice];
	}
	else
	{
		blocks = boost::make_shared<Blocks>();
		(*_sliceBlockMap)[*slice] = blocks;
	}
	
	foreach (boost::shared_ptr<Block> cBlock, *blocks)
	{
		if (*block == *cBlock)
		{
			LOG_DEBUG(localslicestorelog) << "SliceBlockMap already links slice " <<
				slice->getId() << " to block " << block->getId() << std::endl;
			return;
		}
		
	}
	
	blocks->add(block);
}




void
LocalSliceStore::associate(const boost::shared_ptr< Slice >& slice,
							const boost::shared_ptr< Block >& block)
{
	LOG_ALL(localslicestorelog) << "Got a slice with " <<
		slice->getComponent()->getSize() << " pixels." << std::endl;
	
	mapBlockToSlice(block, slice);
	mapSliceToBlock(slice, block);

}


boost::shared_ptr<LinearConstraints>
LocalSliceStore::retrieveConstraints(const boost::shared_ptr<Slices>& slices)
{

	return boost::shared_ptr<LinearConstraints>();
}


void
LocalSliceStore::storeConstraints(const boost::shared_ptr<LinearConstraints>& constraints)
{

}

void
LocalSliceStore::storeConflicts(const boost::shared_ptr<Slices>& slices)
{
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		if (_sliceBlockMap->count(*slice))
		{
			boost::shared_ptr<Blocks> blocks = (*_sliceBlockMap)[*slice];
			foreach (boost::shared_ptr<Block> block, *blocks)
			{
				retrieveSlices(block)->setConflicts(slice->getId(),
													 slices->getConflicts(slice->getId()));
			}
		}
	}
}
