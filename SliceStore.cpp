#include "SliceStore.h"
#include <util/Logger.h>
#include <pipeline/Value.h>
logger::LogChannel slicestorelog("slicestorelog", "[SliceStore] ");


/*********************/
/* Block Slice Store */
/*********************/


BlockSliceStoreNode::BlockSliceStoreNode()
{
	registerInput(_store, "store");
	registerInput(_blocks, "block");
	registerInput(_slicesIn, "slice in");
	registerOutput(_slicesOut, "slices out");
}

void
BlockSliceStoreNode::storeSlices()
{
	LOG_DEBUG(slicestorelog) << "Storing " << _slicesIn->size() << " slices" << std::endl;
	boost::shared_ptr<Blocks> blocks = inputBlocksToVector();
	
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
	boost::shared_ptr<Blocks> blocks = inputBlocksToVector();
	
	for(unsigned int i = 0; i < _slicesIn->size(); ++i)
	{
		_store->removeSliceFromBlocks((*_slicesIn)[i], blocks);
	}
}

boost::shared_ptr<Blocks>
BlockSliceStoreNode::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	return _store->getAssociatedBlocks(slice);
}

void
BlockSliceStoreNode::updateOutputs()
{
	LOG_DEBUG(slicestorelog) << "Retrieving slices" << std::endl;
	boost::shared_ptr<Slices> slices = _store->retrieveSlices(inputBlocksToVector());
	*_slicesOut = *slices;
}

boost::shared_ptr<Blocks>
BlockSliceStoreNode::inputBlocksToVector()
{
	//updateInputs();
	boost::shared_ptr<Blocks> blockVector = boost::make_shared<Blocks>();
	
	LOG_DEBUG(slicestorelog) << "I have " << _blocks->size() << " blocks" << std::endl;
	
	foreach (pipeline::Value<Block> block, *_blocks)
	{
		blockVector->push_back(block);
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

boost::shared_ptr<Blocks>
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
