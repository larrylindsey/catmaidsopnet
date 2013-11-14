#ifdef JUNK
#ifndef SLICE_GUARANTOR_H__
#define SLICE_GUARANTOR_H__

#include <boost/shared_ptr.hpp>

#include <sopnet/slices/SliceExtractor.h>

class SliceGuarantor
{
public:

    SliceGuarantor(SliceStore *sliceStore, ImageBlockReader *blockReader);

    bool guaranteeSlices(Block *block);


private:

    SliceStore *_sliceStore;
    ImageBlockReader *_blockReader;
    std::vector<boost::shared_ptr<ProcessNode> > _sliceExtractors;


};

#endif //SLICE_GUARANTOR_H__
#endif