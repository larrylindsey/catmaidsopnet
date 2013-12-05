#ifndef SLICE_GUARANTOR_H__
#define SLICE_GUARANTOR_H__

#include <set>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/persistence/SliceStore.h>
#include <sopnet/sopnet/block/Block.h>
#include <sopnet/sopnet/block/Blocks.h>
#include <pipeline/all.h>
#include <imageprocessing/io/ImageBlockStackReader.h>
#include <imageprocessing/io/ImageBlockFactory.h>
#include <imageprocessing/MserParameters.h>
#include "SliceGuarantorParameters.h"

class SliceGuarantor : public pipeline::SimpleProcessNode<>
{
public:

    SliceGuarantor();

private:
	
	void updateOutputs();
	
	/**
	 * Helper function that checks whether a Slice can be considered whole or
	 * not, setting its wholeness flag apropriately. Also checks whether the
	 * Block that contains its potential neighbors 
	 */
	void checkWhole(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Blocks>& blocksToSubmit) const;
	
	pipeline::Input<MserParameters> _mserParameters;
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _blockFactory;
	pipeline::Input<Block> _block;
	pipeline::Input<bool> _forceExplanation;
	pipeline::Input<SliceGuarantorParameters> _parameters;
	
	pipeline::Output<Blocks> _neighborBlocks;
	pipeline::Output<SliceStoreResult> _count;
};

#endif //SLICE_GUARANTOR_H__
