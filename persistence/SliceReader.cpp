#include "SliceReader.h"
#include <sopnet/block/Block.h>


#include <util/Logger.h>
logger::LogChannel slicereaderlog("slicereaderlog", "[SliceReader] ");

SliceReader::SliceReader()
{
	registerInput(_box, "box", pipeline::Optional);
	registerInput(_blocks, "blocks", pipeline::Optional);
	registerInput(_store, "store");
	registerInput(_blockManager, "block manager");
	registerOutput(_slices, "slices");
	registerOutput(_trees, "component trees");
	
	_box.registerBackwardCallback(&SliceReader::onBoxSet, this);
	_blocks.registerBackwardCallback(&SliceReader::onBlocksSet, this);
}

void SliceReader::onBoxSet(const pipeline::InputSetBase& )
{
	_sourceIsBox = true;
}

void SliceReader::onBlocksSet(const pipeline::InputSetBase& )
{
	_sourceIsBox = false;
	setInput("block manager", _blocks->getManager());
}



void SliceReader::updateOutputs()
{
	boost::unordered_set<Slice> sliceSet;
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	boost::shared_ptr<Blocks> blocks;
	boost::shared_ptr<ComponentTrees> trees = boost::make_shared<ComponentTrees>();

	if (!_blocks && !_box)
	{
		LOG_ERROR(slicereaderlog) << "Need either box or blocks, neither was set" << std::endl;
		*_slices = *slices;
		*_trees = *trees;
		return;
	}
	else if (_sourceIsBox)
	{
		LOG_DEBUG(slicereaderlog) << "Blocks derived from box input" << std::endl;
		blocks = _blockManager->blocksInBox(_box);
	}
	else
	{
		LOG_DEBUG(slicereaderlog) << "Using blocks input directly" << std::endl;
		blocks = _blocks;
	}
	
	foreach (boost::shared_ptr<Block> block, *blocks)
	{
		boost::shared_ptr<Slices> blockSlices = _store->retrieveSlices(block);
		foreach (boost::shared_ptr<Slice> slice, *blockSlices)
		{
			if (!sliceSet.count(*slice))
			{
				slices->add(slice);
				sliceSet.insert(*slice);
			}
		}
		
	}
	
	LOG_DEBUG(slicereaderlog) << "Inserting slices into trees" << std::endl;
	insertSlicesIntoTrees(slices, trees, sliceSet);
	LOG_DEBUG(slicereaderlog) << "Done inserting slices into trees" << std::endl;
	
	*_slices = *slices;
	*_trees = *trees;
}

void
SliceReader::insertSlicesIntoTrees(const boost::shared_ptr<Slices>& slices,
								 const boost::shared_ptr<ComponentTrees>& trees,
								 const boost::unordered_set<Slice>& sliceSet)
{
	std::map<unsigned int, boost::shared_ptr<Slices> > rootSlices;
	std::map<unsigned int, boost::shared_ptr<Slices> >::iterator it;
	
	LOG_DEBUG(slicereaderlog) << "Finding root slices" << std::endl;
	
	foreach (boost::shared_ptr<Slice> slice, *slices)
	{
		boost::shared_ptr<Slice> parentSlice = _store->getParent(slice);
		if (!parentSlice || sliceSet.count(*parentSlice))
		{
			if (!rootSlices.count(slice->getSection()))
			{
				rootSlices[slice->getSection()] = boost::make_shared<Slices>();
			}
			rootSlices[slice->getSection()]->add(slice);
		}
	}
	
	LOG_DEBUG(slicereaderlog) << "Done assigning root slices, re-creating trees" << std::endl;
	
	for (it = rootSlices.begin(); it != rootSlices.end(); ++it)
	{
		
		boost::shared_ptr<ComponentTree::Node> fakeNode =
			boost::make_shared<ComponentTree::Node>(boost::make_shared<ConnectedComponent>());
		foreach (boost::shared_ptr<Slice> slice, *(it->second))
		{
			std::deque<unsigned int> idq;
			boost::shared_ptr<ComponentTree::Node> rootNode = 
				boost::make_shared<ComponentTree::Node>(slice->getComponent());
			
			addNode(rootNode, slice, sliceSet, slices, idq);
			rootNode->setParent(fakeNode);
		}	
		
		(*trees)[it->first]->setRoot(fakeNode);
	}
	
	LOG_DEBUG(slicereaderlog) << "Done assigning root slices, re-creating trees" << std::endl;
}

void
SliceReader::addNode(const boost::shared_ptr<ComponentTree::Node>& node,
					 const boost::shared_ptr<Slice>& slice,
					 const boost::unordered_set<Slice>& sliceSet,
					 const boost::shared_ptr<Slices>& outputSlices,
					 std::deque<unsigned int>& slice_ids)
{
	bool addConflict = true;
	slice_ids.push_back(slice->getId());

	foreach (boost::shared_ptr<Slice> childSlice, *(_store->getChildren(slice)))
	{
		if (sliceSet.count(*childSlice))
		{
			boost::shared_ptr<ComponentTree::Node> childNode = 
				boost::make_shared<ComponentTree::Node>(childSlice->getComponent());

			addConflict = false;
			childNode->setParent(node);
			addNode(childNode, childSlice, sliceSet, outputSlices, slice_ids);
		}
	}

	if (addConflict)
	{
		outputSlices->addConflicts(slice_ids);
	}

	slice_ids.pop_back();
}


