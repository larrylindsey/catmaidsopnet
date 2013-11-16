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

}

void
BlockSliceStoreNode::removeSlices()
{

}

void
BlockSliceStoreNode::removeSlicesFromBlocks()
{

}

std::vector< boost::shared_ptr< Block > >
BlockSliceStoreNode::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	return _store->getAssociatedBlocks(slice);
}

void BlockSliceStoreNode::updateOutputs()
{

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
IdSliceStoreNode::storeSlices()
{

}

void
IdSliceStoreNode::removeSlices()
{

}

std::vector< boost::shared_ptr< Block > >
IdSliceStoreNode::getAssociatedBlocks(const boost::shared_ptr< Slice >& slice)
{
	return _store->getAssociatedBlocks(slice);
}

void
IdSliceStoreNode::updateOutputs()
{

}
