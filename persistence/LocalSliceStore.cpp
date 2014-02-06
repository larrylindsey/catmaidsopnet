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
	LOG_ALL(localslicestorelog) << "Retrieved " << slices->size() << " slices for block " <<
		*block << std::endl;
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
	LOG_ALL(localslicestorelog) << "Block " << *block << " associated with " << slices->size() <<
		" slices" << std::endl;
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
	LOG_ALL(localslicestorelog) << "Slice " << slice->getId() << " associated with " << blocks->size() <<
		" blocks" << std::endl;
}

void
LocalSliceStore::associate(const boost::shared_ptr< Slice >& sliceIn,
							const boost::shared_ptr< Block >& block)
{
	LOG_ALL(localslicestorelog) << "Got a slice with " <<
		sliceIn->getComponent()->getSize() << " pixels." << std::endl;

	boost::shared_ptr<Slice> slice = equivalentSlice(sliceIn);
	
	if (slice != sliceIn)
	{
		LOG_ALL(localslicestorelog) <<
			" Rejected slice argument, using the one I found in my box instead" << std::endl;
	}
	
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

void LocalSliceStore::dumpStore()
{
	LOG_DEBUG(localslicestorelog) << _sliceMasterList.size() << " Slices stored" << std::endl;
	LOG_DEBUG(localslicestorelog) << _blockSliceMap->size() << " Blocks mapped" << std::endl;
	LOG_DEBUG(localslicestorelog) << _sliceBlockMap->size() << " Slices mapped" << std::endl;
	
	BlockSliceMap::iterator bsm_it;
	SliceBlockMap::iterator sbm_it;
	
	for (bsm_it = _blockSliceMap->begin(); bsm_it != _blockSliceMap->end(); ++bsm_it)
	{
		LOG_DEBUG(localslicestorelog) << "Block " << bsm_it->first << " associated to " <<
			bsm_it->second->size() << " slices" << std::endl;
	}
	
	for (sbm_it = _sliceBlockMap->begin(); sbm_it != _sliceBlockMap->end(); ++sbm_it)
	{
		LOG_DEBUG(localslicestorelog) << "Slice " << sbm_it->first.getId() << " associated to " <<
			sbm_it->second->length() << " blocks" << std::endl;
	}
}




boost::shared_ptr<Slice>
LocalSliceStore::equivalentSlice(const boost::shared_ptr<Slice>& slice)
{
	if (_sliceMasterList.count(*slice))
	{
		//LOG_ALL(localslicestorelog) << "Slice already exists in master list" << std::endl;
		unsigned int id = _sliceMasterList.find(*slice)->getId();
		boost::shared_ptr<Slice> eqSlice = (*_idSliceMap)[id];
		return eqSlice;
	}
	else
	{
		//LOG_ALL(localslicestorelog) << "Slice does not yet exist, returning as new" << std::endl;
		return slice;
	}
}

