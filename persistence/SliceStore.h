#ifndef SLICE_STORE_H__
#define SLICE_STORE_H__

#include <boost/shared_ptr.hpp>

#include <sopnet/slices/Slice.h>
#include <pipeline/all.h>

#include <sopnet/slices/Slices.h>
#include <sopnet/block/Block.h>
#include <sopnet/block/Blocks.h>
#include <sopnet/inference/LinearConstraints.h>

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
 * Abstract Data class that handles the practicalities of storing and retrieving Slices from a
 * store.
 */
class SliceStore : public pipeline::Data
{
public:
    /**
     * Associates a slice with a block
     * @param slice - the slice to store.
     * @param block - the block containing the slice.
     */
    virtual void associate(const boost::shared_ptr<Slice>& slice,
						   const boost::shared_ptr<Block>& block) = 0;

    /**
     * Retrieve all slices that are at least partially contained in the given block.
     * @param block - the Block for which to retrieve all slices.
     */
    virtual boost::shared_ptr<Slices> retrieveSlices(const boost::shared_ptr<Block>& block) = 0;

	/**
	 * Disassociate the given slice from the given block
	 * @param slice - the slice to disassociate
	 * @param block - the block to disassociate
	 */
	virtual void disassociate(const boost::shared_ptr<Slice>& slice,
							  const boost::shared_ptr<Block>& block) = 0;

	/**
	 * Remove a Slice entirely from the store.
	 * @param slice - the slice to remove.
	 */
	virtual void removeSlice(const boost::shared_ptr<Slice>& slice) = 0;

	/**
	 * Retrieve all Blocks associated with the given slice.
	 * @param slice - the slice for which Blocks are to be retrieved.
	 */
	virtual boost::shared_ptr<Blocks> getAssociatedBlocks(
		const boost::shared_ptr<Slice>& slice) = 0;
	
	/**
	 * Store a parent-child relationship between two slices.
	 */
	virtual void setParent(const boost::shared_ptr<Slice>& childSlice,
						   const boost::shared_ptr<Slice>& parentSlice) = 0;

	/**
	 * Retrieve the children of the given Slice.
	 */
	virtual boost::shared_ptr<Slices> getChildren(const boost::shared_ptr<Slice>& parentSlice) = 0;
	
	virtual boost::shared_ptr<Slice> getParent(const boost::shared_ptr<Slice>& childSlice) = 0;
};

#endif //SLICE_STORE_H__