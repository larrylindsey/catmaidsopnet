#ifndef SLICE_WRITER_H__
#define SLICE_WRITER_H__

#include <pipeline/all.h>
#include <sopnet/block/Block.h>
#include <sopnet/slices/Slices.h>
#include <imageprocessing/ComponentTrees.h>
#include <catmaidsopnet/persistence/SliceStore.h>
#include <boost/unordered_map.hpp>

class SliceWriter : public pipeline::SimpleProcessNode<>
{
	typedef boost::unordered_map<ConnectedComponent, boost::shared_ptr<Slice> >  ComponentSliceMap;
public:
	SliceWriter();
	
private:
	void updateOutputs();
	
	void assignParents(ComponentSliceMap& componentSliceMap,
					   const boost::shared_ptr<ComponentTree::Node>& node);
	
	pipeline::Input<Block> _block;
	pipeline::Input<Slices> _slices;
	pipeline::Input<SliceStore> _store;
	pipeline::Input<ComponentTrees> _trees;
	pipeline::Output<SliceStoreResult> _count;
};


#endif //SLICE_WRITER_H__