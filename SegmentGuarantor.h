#ifndef SEGMENT_GUARANTOR_H__
#define SEGMENT_GUARANTOR_H__

#include <pipeline/all.h>
#include <catmaidsopnet/SliceGuarantor.h>
#include <catmaidsopnet/persistence/SegmentStore.h>
#include <sopnet/block/BlockManager.h>
#include <catmaidsopnet/persistence/SliceReader.h>

class SegmentGuarantor : public pipeline::SimpleProcessNode<>
{
public:
	SegmentGuarantor();
	
	void updateOutputs();
	
	void guaranteeSegments(const boost::shared_ptr<Blocks>& guaranteeBlocks,
						   const boost::shared_ptr<Blocks>& sliceBlocks);
	
private:
	boost::shared_ptr<Slices> collectSlicesByZ(const boost::shared_ptr<Slices>& slices,
											   unsigned int z) const;
	boost::shared_ptr<LinearConstraints> collectConstraints(
		const boost::shared_ptr<Slices>& slices,
		const boost::shared_ptr<LinearConstraints>& constraints) const;
	bool isAssociated(const LinearConstraint& constraint,
					  const boost::shared_ptr<Slices>& slices) const;
	boost::shared_ptr<Slices> collectNecessarySlices(
		const boost::shared_ptr<SliceReader>& sliceReader,
		const boost::shared_ptr<Blocks>& sliceBlocks);
	
	pipeline::Input<SegmentStore> _segmentStore;
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<Box<> > _box;
	pipeline::Input<BlockManager> _blockManager;
	
	pipeline::Output<SegmentStoreResult> _result;
	
};

#endif //SEGMENT_GUARANTOR_H__