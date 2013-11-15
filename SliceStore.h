#ifndef SLICE_STORE_H__
#define SLICE_STORE_H__

#include <boost/shared_ptr.hpp>
#include <vector>
#include <sopnet/slices/Slice.h>
#include <pipeline/all.h>

#include <catmaidsopnet/Block.h>

/**
 * Abstract Data class that handles the practicalities of storing and retrieving Slices from a store.
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
	
	virtual void removeSlice(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block) = 0;
	
	virtual std::vector<boost::shared_ptr<Block>> getAssociatedBlocks(const boost::shared_ptr<Slice>& slice) = 0;


};


/**
 * ProcessNode class to handle the batch storage, retrieval, or removal of Slices with respect to
 * a set of Blocks. 
 * 
 * Inputs:
 *   blocks - the blocks used for addressing the slices in the store
 *   slices in - slices to be stored in the SliceStore
 *   store - the SliceStore
 * 
 * Outputs:
 *   slices out - slices retrieved from the SliceStore
 * 
 * Store
 *   To write Slices to the store, addInputs as usual. When storeSlices() is called,
 *   this BatchSliceStore will write all Slices to the store for the given Blocks. The 
 *   inputs will then all be cleared, so that this BatchSliceStore may be re-used
 * 
 * Remove
 *   The remove operation is similar to the store operation. Instead of storing a Slice
 *   as associated with a given Block, this operation removes that association.
 * 
 * Retrieve
 *  Slices are retrieved automatically through the pipeline when the slice outputs are
 *  dereferenced. This is handled through a call to updateOutputs.
 */
class BlockSliceStoreNode : public pipeline::SimpleProcessNode<>
{
public:
	BatchSliceStore();
	
	/**
	 * Put Slice Inputs into the SliceStore, associating them with the Block Inputs,
	 * then clear Slice Inputs.
	 */
	void storeSlices();
	
	/**
	 * Remove associations between Slice Inputs and Block Inputs in the SliceStore,
	 * then clear Slice Inputs.
	 */
	void removeSlicesFromBlocks();
	
	/**
	 * Remove Slice Inputs from the SliceStore entirely
	 */
	void removeSlices();
	
	/**
	 * Get a vector containing all Blocks associated with the given Slice.
	 */
	std::vector<boost::shared_ptr<Block> > getAssociatedBlocks(const shared_ptr<Slice>& slice);
	
private:
	void updateOutputs();
	
	pipeline::Input<SliceStore> _store;
	pipeline::Inputs<Block> _blocks;
	pipeline::Inputs<Slice> _slicesIn;
	
	std::vector<pipeline::Output<Slice> > _slicesOut;
	
};

/**
 * ProcessNode class to handle the the batch storage, retrieval, or removal of Slices with 
 * respect to a set of Slice ids.
 * 
 */
class IdSliceStoreNode : public pipeline::SimpleProcessNode<>
{
public:
	SingletonSliceStore();
	
	/**
	 * Put the Input Slice in the SliceStore as associated with the Block Inputs.
	 */
	void storeSlices();
	
	/**
	 * Remove the Slices from the SliceStore entirely.
	 */
	void removeSlices();
	
	/**
	 * Get a vector containing all Blocks associated with the given Slice.
	 */
	std::vector<boost::shared_ptr<Block> > getAssociatedBlocks(const shared_ptr<Slice>& slice);
	
private:
	void updateOutputs();
	
	pipeline::Input<SliceStore> _store;
	
	pipeline::Inputs<unsigned int> _sliceId;
	
	std::vector<pipeline::Output<Slice>> _sliceOut;

};

#endif //SLICE_STORE_H__
