#ifndef SLICE_GUARANTOR_H__
#define SLICE_GUARANTOR_H__

#include <set>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/SliceStore.h>
#include <sopnet/sopnet/block/Block.h>
#include <pipeline/all.h>
#include <imageprocessing/io/ImageBlockStackReader.h>
#include <imageprocessing/io/ImageBlockFactory.h>
#include <imageprocessing/MserParameters.h>
#include "SliceGuarantorParameters.h"

class SliceGuarantor : public pipeline::ProcessNode
{
public:

    SliceGuarantor();

    void guaranteeSlices();


private:
	
	void onBlockSet(const pipeline::Modified&);
	
	void onSliceStoreSet(const pipeline::Modified&);
	
	void onImageBlockFactorySet(const pipeline::Modified&);
	
	/**
	 * Helper function that checks whether a Slice can be considered whole or
	 * not, setting its wholeness flag apropriately. Also checks whether the
	 * Block that contains its potential neighbors 
	 */
	void checkWhole(const boost::shared_ptr<Slice>& slice, std::set<boost::shared_ptr<Block> >& blocksToSubmit) const;
	
	boost::shared_ptr<ImageBlockStackReader> _stackReader;
	boost::shared_ptr<BlockSliceStoreNode> _blockSliceStore;
	
	pipeline::Input<MserParameters> _mserParameters;
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _blockFactory;
	pipeline::Input<Block> _block;
	pipeline::Input<bool> _forceExplanation;
	pipeline::Input<SliceGuarantorParameters> _parameters;
    
	std::vector<boost::shared_ptr<ProcessNode> > _sliceExtractors;
};

#endif //SLICE_GUARANTOR_H__
