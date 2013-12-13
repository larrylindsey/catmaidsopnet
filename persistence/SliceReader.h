#ifndef SLICE_READER_H__
#define SLICE_READER_H__

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
	
	pipeline::Input<Box<> > _box;
	pipeline::Input<BlockManager> _blockManager;
	pipeline::Input<SliceStore> _store;
	pipeline::Output<Slices> _slices;
	pipeline::Output<LinearConstraints> _constraints;
};

#endif //SLICE_READER_H__