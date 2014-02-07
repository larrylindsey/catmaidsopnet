#ifndef COMPONENT_TREE_EXTRACTOR_H__
#define COMPONENT_TREE_EXTRACTOR_H__

#include <deque>
#include <imageprocessing/ComponentTrees.h>
#include <boost/unordered_set.hpp>
#include <sopnet/block/Block.h>
#include <sopnet/block/Blocks.h>
#include <sopnet/slices/Slices.h>
#include <sopnet/segments/Segments.h>
#include <pipeline/all.h>
#include <catmaidsopnet/persistence/SliceStore.h>


class ComponentTreeExtractor : public pipeline::SimpleProcessNode<>
{
public:
	ComponentTreeExtractor();

private:
	void updateOutputs();
	
	void insertSlicesIntoTrees(
								 const boost::shared_ptr<ComponentTrees>& trees,
								 const boost::shared_ptr<LinearConstraints>& constraints,
								 const boost::unordered_set<Slice>& sliceSet);
	
	void addNode(const boost::shared_ptr<ComponentTree::Node>& node,
				 const boost::shared_ptr<Slice>& slice,
				 const boost::unordered_set<Slice>& sliceSet,
				 const boost::shared_ptr<Slices>& outputSlices,
				 const boost::shared_ptr<LinearConstraints>& constraints,
				 std::deque<unsigned int>& slice_ids);
	
	boost::shared_ptr<LinearConstraints> assembleSegmentConstraints(
				const boost::shared_ptr<LinearConstraints>& sliceConstraints);
	
	pipeline::Input<Blocks> _blocks;
	pipeline::Input<SliceStore> _store;
	pipeline::Input<Slices> _slices;
	pipeline::Input<Segments> _segments;
	pipeline::Input<bool> _forceExplanation;
	
	pipeline::Output<LinearConstraints> _constraints;
	pipeline::Output<ComponentTrees> _componentTrees;
	pipeline::Output<Slices> _slicesOut;
	
};

#endif //COMPONENT_TREE_EXTRACTOR_H__