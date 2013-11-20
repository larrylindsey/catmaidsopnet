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
	
	class SlicesCollector : public pipeline::ProcessNode
	{
	public:
		SlicesCollector();
		
		boost::shared_ptr<Slices> getSlices();
		
	private:
		//void updateOutputs();
		
		pipeline::Input<Block> _block;
		pipeline::Inputs<Slices> _multiSlices;
	};
	
	bool isWhole(const boost::shared_ptr<Slice>& slice);

	boost::shared_ptr<ImageBlockStackReader> _stackReader;
	boost::shared_ptr<BlockSliceStoreNode> _blockSliceStore;
	
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _blockFactory;
	pipeline::Input<Block> _block;
	pipeline::Input<bool> _forceExplanation;
    
	std::vector<boost::shared_ptr<ProcessNode> > _sliceExtractors;
};

#endif //SLICE_GUARANTOR_H__
