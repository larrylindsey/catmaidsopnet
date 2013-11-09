#ifndef SLICE_STORE_H__
#define SLICE_STORE_H__

class SliceStore
{
public:
    SliceStore();

    /**
     * Store a slice with a single block reference
     * @param slice - the slice to store.
     * @param block - the block containing the slice (in whole).
     */
    void storeSlice(Slice& slice, Block& block);

    /**
     * Store a slice with a multi-block reference
     * @param slice - the slice to store.
     * @param blocks - the blocks containing the slice.
     */
    void storeSlice(Slice& slice, std::vector<Block>& blocks);

    /**
     * Retrieve the slice with the given slice id.
     * @param sliceId - the id for the slice in question
     */
    boost::shared_ptr<Slice> retrieveSlice(int sliceId);

    /**
     * Retrieve all slices that are at least partially contained in the given block.
     * @param block - the Block for which to retrieve all slices.
     */
    std::vector<Slice> retrieveSlices(Block& block);


}


#endif //SLICE_STORE_H__
