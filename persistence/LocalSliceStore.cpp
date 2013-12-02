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


std::vector<boost::shared_ptr<Block> >
LocalSliceStore::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	if (_sliceBlockMap->count(*slice))
	{
		return (*_sliceBlockMap)[*slice];
	}
	else
	{
		std::vector<boost::shared_ptr<Block> > empty;
		return empty;
	}
}

void
LocalSliceStore::removeSlice(const boost::shared_ptr< Slice >& slice)
{
	std::vector<boost::shared_ptr<Block> > blocks = getAssociatedBlocks(slice);
	_idSliceMap->erase(slice->getId());
	_sliceBlockMap->erase(*slice);
	foreach (boost::shared_ptr<Block> block, blocks)
	{
		(*_blockSliceMap)[*block]->remove(slice);
	}
}

void
LocalSliceStore::removeSliceFromBlocks(const boost::shared_ptr< Slice >& slice, std::vector< boost::shared_ptr< Block > > blocks)
{
	foreach (boost::shared_ptr<Block> block, blocks)
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
LocalSliceStore::retrieveSlices(std::vector< boost::shared_ptr< Block > > blocks)
{
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	std::set<boost::shared_ptr<Slice> > sliceSet;
	
	// This will be sloooow.
	foreach (boost::shared_ptr<Block> block, blocks)
	{
		boost::shared_ptr<Slices> slices = (*_blockSliceMap)[*block];
		foreach (boost::shared_ptr<Slice> slice, *slices)
		{
			sliceSet.insert(slice);
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
	
	slices->add(slice);
}

void
LocalSliceStore::storeSlice(const boost::shared_ptr< Slice >& slice, std::vector< boost::shared_ptr< Block > > blocks)
{
	foreach(boost::shared_ptr<Block> block, blocks)
	{
		storeSlice(slice, block);
	}
}


void
LocalSliceStore::removeBlockFromVector(const boost::shared_ptr< Block >& block, std::vector< boost::shared_ptr<Block> >& vector)
{
	vector.erase(std::remove(vector.begin(), vector.end(), block), vector.end());
}