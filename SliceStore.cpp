#include "SliceStore.h"

/*********************/
/* Block Slice Store */
/*********************/


BlockSliceStoreNode::BlockSliceStoreNode()
{
	registerInput(_store, "store");
	registerInputs(_blocks, "block");
	registerInput(_slicesIn, "slice in");
	registerInput(_whole, "whole");
	registerOutput(_slicesOut, "slices out");
}

void
BlockSliceStoreNode::storeSlices()
{
	std::vector<boost::shared_ptr<Block> > blocks = inputBlocksToVector();
	
	_store->setWhole(*_whole);
	
	for(unsigned int i = 0; i < _slicesIn->size(); ++i)
	{
		_store->storeSlice((*_slicesIn)[i], blocks);
	}
}

void
BlockSliceStoreNode::removeSlices()
{
	for(unsigned int i = 0; i < _slicesIn->size(); ++i)
	{
		_store->removeSlice((*_slicesIn)[i]);
	}
}

void
BlockSliceStoreNode::removeSlicesFromBlocks()
{
	std::vector<boost::shared_ptr<Block> > blocks = inputBlocksToVector();
	
	for(unsigned int i = 0; i < _slicesIn->size(); ++i)
	{
		_store->removeSliceFromBlocks((*_slicesIn)[i], blocks);
	}
}

std::vector< boost::shared_ptr< Block > >
BlockSliceStoreNode::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	return _store->getAssociatedBlocks(slice);
}

void
BlockSliceStoreNode::updateOutputs()
{
	boost::shared_ptr<Slices> slices = _store->retrieveSlices(inputBlocksToVector());
	*_slicesOut = *slices;
}

std::vector<boost::shared_ptr<Block> >
BlockSliceStoreNode::inputBlocksToVector()
{
	std::vector<boost::shared_ptr<Block> > blockVector;

	foreach (boost::shared_ptr<Block> block, _blocks)
	{
		blockVector.push_back(block);
	}
	
	return blockVector;
}



/******************/
/* Id Slice Store */
/******************/
IdSliceStoreNode::IdSliceStoreNode()
{
	registerInput(_store, "store");
	registerInputs(_sliceId, "slice id");
	registerOutput(_slicesOut, "slice out");
}

void
IdSliceStoreNode::removeSlices()
{
	foreach (pipeline::Input<unsigned int> id, _sliceId)
	{
		boost::shared_ptr<Slice> slice = _store->retrieveSlice(*id);
		_store->removeSlice(slice);
	}
	_sliceId.clear();
}

std::vector< boost::shared_ptr< Block > >
IdSliceStoreNode::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	return _store->getAssociatedBlocks(slice);
}

void
IdSliceStoreNode::updateOutputs()
{
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	foreach (pipeline::Input<unsigned int> id, _sliceId)
	{
		boost::shared_ptr<Slice> slice = _store->retrieveSlice(*id);
		slices->add(slice);
	}
	
	*_slicesOut = *slices;
}
