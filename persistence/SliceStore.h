#ifndef SLICE_STORE_H__
#define SLICE_STORE_H__

#include <boost/shared_ptr.hpp>

#include <sopnet/sopnet/slices/Slice.h>
#include <pipeline/all.h>

#include <sopnet/sopnet/slices/Slices.h>
#include <sopnet/sopnet/block/Block.h>

typedef std::vector<boost::shared_ptr<Block> > Blocks;

class SliceStoreResult : public pipeline::Data
{
public:
	SliceStoreResult() : count(0) {}
	
	SliceStoreResult(int c) : count(c)
	{
	}
	
	int count;
	
};


/**
 * Abstract Data class that handles the practicalities of storing and retrieving Slices from a store.
 */
class SliceStore : public pipeline::Data
{
public:
    /**
     * Associates a slice with a block
     * @param slice - the slice to store.
     * @param block - the block containing the slice (in whole).
     */
    virtual void associate(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block) = 0;

    /**
     * Retrieve all slices that are at least partially contained in the given block.
     * @param block - the Block for which to retrieve all slices.
     */
    virtual boost::shared_ptr<Slices> retrieveSlices(const boost::shared_ptr<Block>& block) = 0;

	virtual void disassociate(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block) = 0;

	virtual void removeSlice(const boost::shared_ptr<Slice>& slice) = 0;

	virtual boost::shared_ptr<Blocks> getAssociatedBlocks(const boost::shared_ptr<Slice>& slice) = 0;

};

#endif //SLICE_STORE_H__