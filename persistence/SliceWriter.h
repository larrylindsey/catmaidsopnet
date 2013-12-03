#ifndef SLICE_WRITER_H__
#define SLICE_WRITER_H__

#include <pipeline/all.h>
#include <sopnet/block/Block.h>
#include <sopnet/slices/Slices.h>
#include <catmaidsopnet/persistence/SliceStore.h>

class SliceWriter : public pipeline::SimpleProcessNode<>
{
public:
	SliceWriter();
	
private:
	void updateOutputs();
	
	pipeline::Input<Block> _block;
	pipeline::Input<Slices> _slices;
	pipeline::Input<SliceStore> _store;
	pipeline::Output<SliceStoreResult> _count;
};


#endif //SLICE_WRITER_H__