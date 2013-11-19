#ifndef SLICE_GUARANTOR_H__
#define SLICE_GUARANTOR_H__

#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/SliceStore.h>
#include <sopnet/sopnet/block/Block.h>
#include <pipeline/all.h>
#include <imageprocessing/io/ImageBlockStackReader.h>
#include <imageprocessing/io/ImageBlockFactory.h>

class SliceGuarantor : public pipeline::ProcessNode
{
public:

    SliceGuarantor();

    void guaranteeSlices();


private:

	boost::shared_ptr<ImageBlockStackReader> _stackReader;
	
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _blockFactory;
	pipeline::Input<Block> _block;
	pipeline::Input<bool> _forceExplanation;
    
	std::vector<boost::shared_ptr<ProcessNode> > _sliceExtractors;
	
	// Helper functions
	
	void extractSlices();
	void dispatchNeighbors(std::vector<boost::shared_ptr<Slice> > slices);
};

#endif //SLICE_GUARANTOR_H__
