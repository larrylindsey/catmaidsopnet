#include "SliceWriter.h"

#include <boost/shared_ptr.hpp>
#include <sopnet/slices/Slice.h>

SliceWriter::SliceWriter()
{
	registerInput(_block, "block");
	registerInput(_slices, "slices");
	registerInput(_store, "store");
	registerInput(_trees, "component trees");
	registerOutput(_count, "count");
}

void
SliceWriter::updateOutputs()
{
	//TODO verify slice inputs against component trees
	// IE, each slice should have an entry in a tree.
	int count = 0;
	ComponentSliceMap componentSliceMap;
	ComponentTrees::iterator ctit;
	std::vector<boost::shared_ptr<ConnectedComponent> > components;
	
	
	foreach (boost::shared_ptr<Slice> slice, *_slices)
	{
		_store->associate(slice, _block);
		componentSliceMap[*(slice->getComponent())] = slice;
		++count;
	}

	for (ctit = _trees->begin(); ctit != _trees->end(); ++ctit)
	{
		boost::shared_ptr<ComponentTree::Node> rootNode = ctit->second->getRoot();
		
		foreach (boost::shared_ptr<ComponentTree::Node> node, rootNode->getChildren())
		{
			assignParents(componentSliceMap, node);
		}
	}
	
	*_count = SliceStoreResult(count);
}

void
SliceWriter::assignParents(ComponentSliceMap& componentSliceMap,
						   const boost::shared_ptr<ComponentTree::Node>& node)
{
	boost::shared_ptr<Slice> parentSlice = componentSliceMap[*(node->getComponent())];
	
	if (parentSlice)
	{
		foreach (boost::shared_ptr<ComponentTree::Node> childNode, node->getChildren())
		{
			boost::shared_ptr<Slice> childSlice = componentSliceMap[*(childNode->getComponent())];
			_store->setParent(childSlice, parentSlice);
			assignParents(componentSliceMap, childNode);
		}
	}
}
