#ifndef SLICE_READER_H__
#define SLICE_READER_H__

#include <deque>
#include <boost/unordered_set.hpp>
#include <imageprocessing/ComponentTrees.h>
#include <catmaidsopnet/persistence/SliceStore.h>
#include <sopnet/block/BlockManager.h>
#include <sopnet/block/Box.h>
#include <sopnet/slices/Slices.h>
#include <pipeline/all.h>

class SliceReader : public pipeline::SimpleProcessNode<>
{
public:
	SliceReader();

private:
	void updateOutputs();
	
	void onBoxSet(const pipeline::InputSetBase&);
	
	void onBlocksSet(const pipeline::InputSetBase&);
	
	
	void fetchChildren(const boost::shared_ptr<Slices>& slicesIn,
					   const boost::shared_ptr<Slices>& slicesOut);
	
	pipeline::Input<Blocks> _blocks;
	pipeline::Input<Box<> > _box;
	pipeline::Input<BlockManager> _blockManager;
	pipeline::Input<SliceStore> _store;
	pipeline::Output<Slices> _slices;
	pipeline::Output<ComponentTrees> _trees;
	
	bool _sourceIsBox;
};

#endif //SLICE_READER_H__