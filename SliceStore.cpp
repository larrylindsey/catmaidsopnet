#include "SliceStore.h"

/*********************/
/* Block Slice Store */
/*********************/


BlockSliceStoreNode::BlockSliceStoreNode()
{
	registerInput(_store, "store");
	registerInputs(_blocks, "block");
	registerInputs(_slicesIn, "slice in");
	registerOutput(_slicesOut, "slices out");
}

void
BlockSliceStoreNode::storeSlices()
{
	std::vector<boost::shared_ptr<Block> > blocks = inputBlocksToVector();
	
	
	foreach (pipeline::Input<Slice> slice, _slicesIn)
	{
		boost::shared_ptr<Slice> slicePtr = boost::shared_ptr<Slice>(&(*slice));
		_store->storeSlice(slicePtr, blocks);
	}
	
	_slicesIn.clear();
}

void
BlockSliceStoreNode::removeSlices()
{
	foreach (pipeline::Input<Slice> slice, _slicesIn)
	{
	boost::shared_ptr<Slice> slicePtr = boost::shared_ptr<Slice>(&(*slice));
		_store->removeSlice(slicePtr);
	}
	
	_slicesIn.clear();
}

void
BlockSliceStoreNode::removeSlicesFromBlocks()
{
	std::vector<boost::shared_ptr<Block> > blocks = inputBlocksToVector();
	
	foreach (pipeline::Input<Slice> slice, _slicesIn)
	{
		boost::shared_ptr<Slice> slicePtr = boost::shared_ptr<Slice>(&(*slice)); 
		_store->removeSliceFromBlocks(slicePtr, blocks);
	}
	
	_slicesIn.clear();
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

	foreach (pipeline::Input<Block> block, _blocks)
	{
		boost::shared_ptr<Block> blockPtr = boost::shared_ptr<Block>(&(*block));
		blockVector.push_back(blockPtr);
	}
	
	return blockVector;
}



/*******************/
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
