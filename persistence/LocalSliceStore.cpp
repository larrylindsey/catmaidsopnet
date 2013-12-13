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
	_conflictMap = boost::make_shared<ConflictMap>();
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
		LOG_DEBUG(localslicestorelog) << "Adding conflict info for " << slices->size() <<
			" slices" << std::endl;
		foreach (boost::shared_ptr<Slice> slice, *slices)
		{
			addConflict(slice, slices);
		}
	}

	return slices;
}

void
LocalSliceStore::addConflict(const boost::shared_ptr< Slice >& slice,
							 const boost::shared_ptr< Slices >& slices)
{
	if (_conflictMap->count(*slice))
	{
		boost::shared_ptr<std::vector<unsigned int> > conflict = 
			boost::make_shared<std::vector<unsigned int> >();
			
		boost::shared_ptr<Slices> conflictSlices = (*_conflictMap)[*slice];
		foreach (boost::shared_ptr<Slice> conflictSlice, *conflictSlices)
		{
			conflict->push_back(conflictSlice->getId());
		}
		slices->setConflicts(slice->getId(), *conflict);
	}
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


boost::shared_ptr<LinearConstraints>
LocalSliceStore::retrieveConstraints(const boost::shared_ptr<Slices>& slices)
{
	boost::shared_ptr<LinearConstraints> constraints = boost::make_shared<LinearConstraints>();
	
	LOG_DEBUG(localslicestorelog) << "Retrieving constraints for " << slices->size() <<
		" slices" << std::endl;
	
	foreach (boost::shared_ptr<SliceStoreLinearConstraint> constraint, _constraints)
	{
		if (constraint->associated(slices))
		{
			constraints->add(*constraint->getConstraint(slices));
		}
	}
	
	return constraints;
}


void
LocalSliceStore::storeConstraints(const boost::shared_ptr<LinearConstraints>& constraints)
{
	foreach (LinearConstraint& constraint, *constraints)
	{
		boost::shared_ptr<SliceStoreLinearConstraint> storeConstraint =
			boost::make_shared<SliceStoreLinearConstraint>(constraint, _idSliceMap);
		_constraints.push_back(storeConstraint);
	}
}

void
LocalSliceStore::storeConflicts(const boost::shared_ptr<Slices>& slices)
{
	
	// Conflict in Slices is stored as a set of id's. 
	//
	// We will have multiple Slice objects with the same data and different id's, since
	// we're extracting across Block borders. Here, we store conflict information in a 
	// way that is tied to the Slice == operator and hashValue, which are id-agnostic.
	
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		// If slice is already in the slice master list...
		if (_sliceMasterList.count(*slice))
		{
			boost::shared_ptr<Slices> conflictSlices;
			
			if (_conflictMap->count(*slice))
			{
				// If the conflict map already has the slice, retrieve the associated Slices.
				conflictSlices = (*_conflictMap)[*slice];
			}
			else
			{
				// Otherwise, create the Slices and push it into the conflict map.
				conflictSlices = boost::make_shared<Slices>();
				(*_conflictMap)[*slice] = conflictSlices;
			}

			// Grab the id. If the id corresponds to a Slice in the map, add it to conflicts.
			foreach (unsigned int id, slices->getConflicts(slice->getId()))
			{
				if (_idSliceMap->count(id))
				{
					conflictSlices->add((*_idSliceMap)[id]);
				}
			}
		}
	}
}

LocalSliceStore::SliceStoreLinearConstraint::SliceStoreLinearConstraint(
	const LinearConstraint& constraint,
	const boost::shared_ptr<IdSliceMap>& idSliceMap) : _relation(constraint.getRelation()),
		_value(constraint.getValue())
{
	std::map<unsigned int, double>::const_iterator iter;
	for (iter = constraint.getCoefficients().begin();
		 iter != constraint.getCoefficients().end(); ++iter)
	{
		if (idSliceMap->count(iter->first))
		{
			boost::shared_ptr<Slice> slice = (*idSliceMap)[iter->first];
			_coefs[*slice] = iter->second;
		}
	}
}

bool
LocalSliceStore::SliceStoreLinearConstraint::associated(const boost::shared_ptr< Slices >& slices)
{
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		if (_coefs.count(*slice))
		{
			return true;
		}
	}
	return false;
}

boost::shared_ptr<LinearConstraint>
LocalSliceStore::SliceStoreLinearConstraint::getConstraint(
	const boost::shared_ptr< Slices >& slices)
{
	boost::shared_ptr<LinearConstraint> constraint = boost::make_shared<LinearConstraint>();
	constraint->setRelation(_relation);
	constraint->setValue(_value);
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		if (_coefs.count(*slice))
		{
			constraint->setCoefficient(slice->getId(), _coefs[*slice]);
		}
	}
	return constraint;
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

