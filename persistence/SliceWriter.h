#ifndef SLICE_WRITER_H__
#define SLICE_WRITER_H__

#include <pipeline/all.h>
#include <sopnet/block/Blocks.h>
#include <sopnet/slices/Slices.h>
#include <sopnet/slices/ConflictSets.h>
#include <catmaidsopnet/persistence/SliceStore.h>

class SliceWriter : public pipeline::SimpleProcessNode<>
{
public:
	SliceWriter();
	
	void writeSlices();
	
private:
	
	void updateOutputs(){}
	
	pipeline::Value<Slices> collectSlicesByBlocks(const boost::shared_ptr<Block> block);
	boost::shared_ptr<ConflictSets> collectConflictBySlices(const pipeline::Value<Slices> slices);

	pipeline::Input<Blocks> _blocks;
	pipeline::Input<Slices> _slices;
	pipeline::Input<SliceStore> _store;
	pipeline::Input<ConflictSets> _conflictSets;
};


#endif //SLICE_WRITER_H__