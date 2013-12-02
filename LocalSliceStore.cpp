#include "LocalSliceStore.h"
#include <boost/make_shared.hpp>
#include <set>

#include <util/Logger.h>
logger::LogChannel localslicestorelog("localslicestorelog", "[LocalSliceStore] ");

LocalSliceStore::LocalSliceStore()
{	
	_sliceBlockMap = boost::make_shared<SliceBlockMap>();
	_idSliceMap = boost::make_shared<IdSliceMap>();
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
	_idSliceMap->erase(slice->getId());
	_sliceBlockMap->erase(*slice);
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		(*_blockSliceMap)[*block]->remove(slice);
	}
}

void
LocalSliceStore::removeSliceFromBlocks(const boost::shared_ptr< Slice >& slice, const boost::shared_ptr<Blocks>& blocks)
{
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		removeBlockFromVector(block, (*_sliceBlockMap)[*slice]);
		//(*_sliceBlockMap)[*slice].erase(block);
		(*_blockSliceMap)[*block]->remove(slice);
	}
}

boost::shared_ptr<Slice>
LocalSliceStore::retrieveSlice(unsigned int sliceId)
{
	return (*_idSliceMap)[sliceId];
}

boost::shared_ptr<Slices>
LocalSliceStore::retrieveSlices(const boost::shared_ptr<Blocks>& blocks)
{
	LOG_DEBUG(localslicestorelog) << "Retrieving slices for " << blocks->size() << " blocks." << std::endl;
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	std::set<boost::shared_ptr<Slice> > sliceSet;
	
	// This will be sloooow.
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		if(block)
		{
			LOG_DEBUG(localslicestorelog) << "Non-null block" << std::endl;
		}
		else
		{
			LOG_DEBUG(localslicestorelog) << "NULL block!" << std::endl;
		}

		if (_blockSliceMap->count(*block))
		{
			boost::shared_ptr<Slices> slices = (*_blockSliceMap)[*block];
			LOG_DEBUG(localslicestorelog) << "For block with id " << block->getId() << " retrieved "
				<< slices->size() << " slices." << std::endl;
			foreach (boost::shared_ptr<Slice> slice, *slices)
			{
				sliceSet.insert(slice);
			}
		}
		else
		{
			LOG_DEBUG(localslicestorelog) << "No slices associated with " << block->getId() << std::endl;
		}
	}
	
	foreach (boost::shared_ptr<Slice> slice, sliceSet)
	{
		slices->add(slice);
	}
	
	return slices;
}

void
LocalSliceStore::storeSlice(const boost::shared_ptr< Slice >& slice,
							const boost::shared_ptr< Block >& block)
{	
	boost::shared_ptr<Slices> slices;
	LOG_DEBUG(localslicestorelog) << "Storing slice with id " << slice->getId() << std::endl;
	
	(*_idSliceMap)[slice->getId()] = slice;
	
	if (_blockSliceMap->count(*block))
	{
		LOG_DEBUG(localslicestorelog) << "BlockSliceMap already contains block " <<
			block->getId() << std::endl;
		slices = (*_blockSliceMap)[*block];
	}
	else
	{
		LOG_DEBUG(localslicestorelog) << "BlockSliceMap does not contain block " <<
			block->getId() << ". Adding it" << std::endl;
		slices = boost::make_shared<Slices>();
		(*_blockSliceMap)[*block] = slices;
	}
	
	foreach (boost::shared_ptr<Slice> cSlice, *slices)
	{
		if (cSlice->getId() == slice->getId())
		{
			LOG_DEBUG(localslicestorelog) << "BlockSliceMap already links Block " <<
				block->getId() << " to slice " << slice->getId() << std::endl;
			return;
		}
	}
	
	if (_sliceBlockMap->count(*slice))
	{
		(*_sliceBlockMap)[*slice]->push_back(block);
	}
	else
	{
		boost::shared_ptr<Blocks> blocks = boost::make_shared<Blocks>();
		blocks->push_back(block);
		(*_sliceBlockMap)[*slice] = blocks;
	}
	
	slices->add(slice);
}

void
LocalSliceStore::storeSlice(const boost::shared_ptr< Slice >& slice, const boost::shared_ptr<Blocks>& blocks)
{
	LOG_DEBUG(localslicestorelog) << "Associating slice with id " << slice->getId() << " with " << blocks->size() << " blocks." << std::endl;
	foreach(boost::shared_ptr<Block> block, *blocks)
	{
		storeSlice(slice, block);
	}
}


void
LocalSliceStore::removeBlockFromVector(const boost::shared_ptr< Block >& block, const boost::shared_ptr<Blocks>& vector)
{
	vector->erase(std::remove(vector->begin(), vector->end(), block), vector->end());
}