#include "ComponentTreeExtractor.h"

#include <util/Logger.h>
#include <boost/unordered_map.hpp>

logger::LogChannel componenttreeextractorlog("componenttreeextractorlog", "[ComponentTreeExtractor] ");

ComponentTreeExtractor::ComponentTreeExtractor()
{
	registerInput(_blocks, "blocks");
	registerInput(_store, "store");
	registerInput(_slices, "slices");
	registerInput(_segments, "segments", pipeline::Optional);
	registerInput(_forceExplanation, "force explanation");
	
	registerOutput(_componentTrees, "trees");
	registerOutput(_slicesOut, "slices");
	registerOutput(_constraints, "linear constraints");
}


void
ComponentTreeExtractor::updateOutputs()
{
	boost::shared_ptr<ComponentTrees> trees = boost::make_shared<ComponentTrees>();
	boost::unordered_set<Slice> sliceSet;
	boost::shared_ptr<LinearConstraints> constraints = boost::make_shared<LinearConstraints>();
	
	foreach (boost::shared_ptr<Slice> slice, *_slices)
	{
		sliceSet.insert(*slice);
	}
	
	insertSlicesIntoTrees(trees, constraints, sliceSet);
	
	if (_segments)
	{
		*_constraints = *assembleSegmentConstraints(constraints);
	}
	else
	{
		*_constraints = *constraints;
	}
	
	*_componentTrees = *trees;
	*_slicesOut = *_slices;
	
}


void
ComponentTreeExtractor::insertSlicesIntoTrees(
										const boost::shared_ptr<ComponentTrees>& trees,
										const boost::shared_ptr<LinearConstraints>& constraints,
										const boost::unordered_set<Slice>& sliceSet)
{
	std::map<unsigned int, boost::shared_ptr<Slices> > rootSlices;
	std::map<unsigned int, boost::shared_ptr<Slices> >::iterator it;
	
	LOG_DEBUG(componenttreeextractorlog) << "Finding root slices" << std::endl;
	
	// Find the root slices in each section. There will be many, and we will make them
	// children of a fake node.
	foreach (boost::shared_ptr<Slice> slice, *_slices)
	{
		boost::shared_ptr<Slice> parentSlice = _store->getParent(slice);
		if (!parentSlice)
		{
			if (!rootSlices.count(slice->getSection()))
			{
				rootSlices[slice->getSection()] = boost::make_shared<Slices>();
			}
			rootSlices[slice->getSection()]->add(slice);
		}
	}
	
	LOG_DEBUG(componenttreeextractorlog) << "Done assigning root slices, re-creating trees" << std::endl;
	
	for (it = rootSlices.begin(); it != rootSlices.end(); ++it)
	{
		
		boost::shared_ptr<ComponentTree::Node> fakeNode =
			boost::make_shared<ComponentTree::Node>(boost::make_shared<ConnectedComponent>());
		boost::shared_ptr<Slices> slices = it->second;
		foreach (boost::shared_ptr<Slice> slice, *slices)
		{
			std::deque<unsigned int> idq;
			boost::shared_ptr<ComponentTree::Node> rootNode = 
				boost::make_shared<ComponentTree::Node>(slice->getComponent());
			
			addNode(rootNode, slice, sliceSet, slices, constraints, idq);
			rootNode->setParent(fakeNode);
		}
		
		(*trees)[it->first]->setRoot(fakeNode);
	}
	
	LOG_DEBUG(componenttreeextractorlog) << "Done." << std::endl;
}


void
ComponentTreeExtractor::addNode(const boost::shared_ptr<ComponentTree::Node>& node,
								const boost::shared_ptr<Slice>& slice,
								const boost::unordered_set<Slice>& sliceSet,
								const boost::shared_ptr<Slices>& outputSlices,
								const boost::shared_ptr<LinearConstraints>& constraints,
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

			// if this node has a child, then we are not ready to add the conflict info.
			addConflict = false;
			childNode->setParent(node);
			addNode(childNode, childSlice, sliceSet, outputSlices, constraints, slice_ids);
		}
	}

	if (addConflict)
	{
		LinearConstraint constraint;
		
		foreach (unsigned int id, slice_ids)
		{
			constraint.setCoefficient(id, 1);
		}
		
		if (_forceExplanation && ! *_forceExplanation)
		{
			constraint.setRelation(LessEqual);
		}
		else
		{
			constraint.setRelation(Equal);
		}

		
		constraint.setValue(1);

		outputSlices->addConflicts(slice_ids);
	}

	slice_ids.pop_back();
}


boost::shared_ptr<LinearConstraints>
ComponentTreeExtractor::assembleSegmentConstraints(
	const boost::shared_ptr<LinearConstraints>& sliceConstraints)
{
	LOG_DEBUG(componenttreeextractorlog) << "Assembling segment constraints" << std::endl;
	
	boost::unordered_map<unsigned int, std::vector<unsigned int> > sliceSegments;
	boost::shared_ptr<LinearConstraints> segmentConstraints = 
		boost::make_shared<LinearConstraints>();
	//First, map segment ids to slice ids;
		
	foreach (boost::shared_ptr<Segment> segment, _segments->getSegments())
	{
		foreach(boost::shared_ptr<Slice> slice, segment->getSlices())
		{
			sliceSegments[slice->getId()].push_back(segment->getId());
		}
	}
		
	//Code here lifted from SegmentExtractor::assembleLinearConstraints.
		
		
	foreach (const LinearConstraint& sliceConstraint, *sliceConstraints)
	{
		LinearConstraint constraint;
		
		// for each slice in the constraint
		typedef std::map<unsigned int, double>::value_type pair_t;
		foreach (const pair_t& pair, sliceConstraint.getCoefficients()) {

			unsigned int sliceId = pair.first;

			// for all the segments that involve this slice
			const std::vector<unsigned int> segmentIds = sliceSegments[sliceId];

			foreach (unsigned int segmentId, segmentIds)
			{
				constraint.setCoefficient(segmentId, 1.0);
			}
		}

		if (_forceExplanation && ! *_forceExplanation)
		{
			constraint.setRelation(LessEqual);
		}
		else
		{
			constraint.setRelation(Equal);
		}

		constraint.setValue(1);
		
		segmentConstraints->add(constraint);
	}
	LOG_DEBUG(componenttreeextractorlog) << "Done." << std::endl;
	return segmentConstraints;
}
