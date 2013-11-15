#ifndef SLICE_STORE_H__
#define SLICE_STORE_H__

#include <boost/shared_ptr.hpp>
#include <vector>
#include <sopnet/slices/Slice.h>
#include <pipeline/Data.h>

#include <catmaidsopnet/Block.h>

/**
 * Abstract class to handle storing and retrieving Slices. 
 */
class SliceStore : public pipeline::Data
{
public:
    SliceStore();

    /**
     * Store a slice with a single block reference
     * @param slice - the slice to store.
     * @param block - the block containing the slice (in whole).
     */
    virtual void storeSlice(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block) = 0;

    /**
     * Store a slice with a multi-block reference
     * @param slice - the slice to store.
     * @param blocks - the blocks containing the slice.
     */
    virtual void storeSlice(const boost::shared_ptr<Slice>& slice, std::vector<boost::shared_ptr<Block> > block) = 0;

    /**
     * Retrieve the slice with the given slice id.
     * @param sliceId - the id for the slice in question
     */
    virtual boost::shared_ptr<Slice> retrieveSlice(int sliceId) = 0;

    /**
     * Retrieve all slices that are at least partially contained in the given block.
     * @param block - the Block for which to retrieve all slices.
     */
    virtual std::vector<boost::shared_ptr<Slice>> retrieveSlices(const boost::shared_ptr<Block>& block) = 0;


};

#endif //SLICE_STORE_H__
