#include "LocalSliceStore.h"
#include <boost/make_shared.hpp>
#include <set>
#include <utility>

#include <util/Logger.h>
logger::LogChannel localslicestorelog("localslicestorelog", "[LocalSliceStore] ");

LocalSliceStore::LocalSliceStore()
{
	_sliceBlockMap = boost::make_shared<SliceBlockMap>();
	_blockSliceMap = boost::make_shared<BlockSliceMap>();
	_idSliceMap = boost::make_shared<IdSliceMap>();
	_parentChildrenMap = boost::make_shared<SliceSlicesMap>();
	_childParentMap = boost::make_shared<SliceSliceMap>();
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
	
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		(*_blockSliceMap)[*block]->remove(slice);
	}
	
	_sliceBlockMap->erase(*slice);
	_sliceMasterList.erase(*slice);
}

void
LocalSliceStore::disassociate(const boost::shared_ptr< Slice >& slice, const boost::shared_ptr<Block>& block)
{
	if (_sliceBlockMap->count(*slice))
	{
		(*_sliceBlockMap)[*slice]->remove(block);
		
		if ((*_sliceBlockMap)[*slice]->length() == 0)
		{
			_sliceBlockMap->erase(*slice);
		}
	}
	
	if (_blockSliceMap->count(*block))
	{
		(*_blockSliceMap)[*block]->remove(slice);
		
		if ((*_blockSliceMap)[*block]->size() == 0)
		{
			_blockSliceMap->erase(*block);
		}
	}
}

boost::shared_ptr<Slices>
LocalSliceStore::retrieveSlices(const boost::shared_ptr<Block>& block)
{
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();;
	
	LOG_DEBUG(localslicestorelog) << "Retrieving slices for block at " << block->location() << std::endl;
	
	if (_blockSliceMap->count(*block))
	{
		LOG_DEBUG(localslicestorelog) << "Found block in block slice map" << std::endl;
		boost::shared_ptr<Slices> blockSlices = (*_blockSliceMap)[*block];
		slices->addAll(*blockSlices);
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
LocalSliceStore::associate(const boost::shared_ptr< Slice >& sliceIn,
							const boost::shared_ptr< Block >& block)
{
	LOG_ALL(localslicestorelog) << "Got a slice with " <<
		sliceIn->getComponent()->getSize() << " pixels." << std::endl;

	boost::shared_ptr<Slice> slice = equivalentSlice(sliceIn);
	
	mapBlockToSlice(block, slice);
	mapSliceToBlock(slice, block);
	
	if (_sliceMasterList.count(*slice))
	{
		unsigned int existingId = _sliceMasterList.find(*slice)->getId();
		(*_idSliceMap)[slice->getId()] = (*_idSliceMap)[existingId];
	}
	else
	{
		(*_idSliceMap)[slice->getId()] = slice;
		_sliceMasterList.insert(*slice);
	}
}

boost::shared_ptr<Slice>
LocalSliceStore::equivalentSlice(const boost::shared_ptr<Slice>& slice)
{
	if (_sliceMasterList.count(*slice))
	{
		unsigned int id = _sliceMasterList.find(*slice)->getId();
		boost::shared_ptr<Slice> eqSlice = (*_idSliceMap)[id];
		return eqSlice;
	}
	else
	{
		return slice;
	}
}

void
LocalSliceStore::setParent(const boost::shared_ptr<Slice>& childSlice,
						   const boost::shared_ptr<Slice>& parentSlice)
{
	if (! _parentChildrenMap->count(*parentSlice))
	{
		(*_parentChildrenMap)[*parentSlice] = boost::make_shared<Slices>();
	}
	
	(*_parentChildrenMap)[*parentSlice]->add(childSlice);
	(*_childParentMap)[*childSlice] = parentSlice;
}

boost::shared_ptr<Slices>
LocalSliceStore::getChildren(const boost::shared_ptr<Slice>& parentSlice)
{
	if (_parentChildrenMap->count(*parentSlice))
	{
		return (*_parentChildrenMap)[*parentSlice];
	}
	else
	{
		boost::shared_ptr<Slices> emptySlices = boost::make_shared<Slices>();
		return emptySlices;
	}
}

boost::shared_ptr<Slice>
LocalSliceStore::getParent(const boost::shared_ptr< Slice >& childSlice)
{
	if (_childParentMap->count(*childSlice))
	{
		return (*_childParentMap)[*childSlice];
	}
	else
	{
		return boost::shared_ptr<Slice>();
	}
}
