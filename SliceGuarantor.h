#ifndef SLICE_GUARANTOR_H__
#define SLICE_GUARANTOR_H__

#include <set>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/persistence/SliceWriter.h>
#include <catmaidsopnet/persistence/SliceStore.h>
#include <sopnet/sopnet/block/Box.h>
#include <sopnet/sopnet/block/Blocks.h>
#include <pipeline/all.h>
#include <imageprocessing/ComponentTrees.h>
#include <imageprocessing/io/ImageBlockStackReader.h>
#include <imageprocessing/io/ImageBlockFactory.h>
#include <imageprocessing/MserParameters.h>
#include "SliceGuarantorParameters.h"

/**
 * SliceGuarantor is a class that, given a sub-stack (measured in Blocks), guarantees that all
 * Slices in that substack will exist in the given SliceStore after guaranteeSlices is called.
 * If this guarantee cannot be made, say because the membrane images are not yet available,
 * guaranteeSlices will report those Blocks and exit (in this case, no guarantee about the slices
 * has been made).
 * 
 * This extends to any conflict sets over the slices. In the MSER sense, two Slices conflict if
 * one is the ancestor of the other. A SliceGuarantor will guarantee that all Slices that belong to
 * the same conflict set as a Slice in the guaranteed substack are also populated in the Slice Store,
 * even if they don't overlap with the guaranteed substack.
 * 
 * 
 */

class SliceGuarantor : public pipeline::SimpleProcessNode<>
{
	typedef boost::unordered_map<ConnectedComponent, boost::shared_ptr<Slice> >  ComponentSliceMap;
public:

    SliceGuarantor();

	/**
	 * Makes sure the inputs are up-to-date and extracts the slices for the 
	 * requested blocks.
	 */
	void guaranteeSlices();

private:
	
	void updateOutputs();

	SliceStoreResult extractSlices();
	
	bool guaranteeSlices(const boost::shared_ptr<Blocks>& extractBlocks,
						 const boost::shared_ptr<Blocks>& guaranteeBlocks,
						 const boost::shared_ptr<Slices>& slices,
						 const boost::shared_ptr<ComponentTrees>& trees);
	
	boost::shared_ptr<SliceStoreResult> writeSlices(
		const boost::shared_ptr<Blocks>& guaranteeBlocks,
		const boost::shared_ptr<Blocks>& extractBlocks,
		const boost::shared_ptr<Slices>& slices,
		const boost::shared_ptr<ComponentTrees>& trees);
	
	void writeSlicesHelper(const boost::shared_ptr<Block>& block,
									   const boost::shared_ptr<Slices>& slices,
									   const boost::shared_ptr<ComponentTrees>& trees,
									   const boost::shared_ptr<SliceWriter>& sliceWriter,
									   const boost::shared_ptr<Slices>& writtenSlices,
									   const boost::shared_ptr<SliceStoreResult>& count);
	
	boost::shared_ptr<Slices> collectDescendants(const boost::shared_ptr<Slices>& slices,
												 const boost::shared_ptr<ComponentTrees>& trees);
	
	void getChildren(ComponentSliceMap& componentSliceMap,
					 const boost::shared_ptr<ComponentTree::Node>& node,
					 const boost::shared_ptr<Slices>& descendants);
	
	/**
	 * Helper function that checks whether a Slice can be considered whole or
	 * not, setting its wholeness flag apropriately. Also checks whether the
	 * Block that contains its potential neighbors 
	 */
	void checkWhole(const boost::shared_ptr<Slice>& slice,
					const boost::shared_ptr<Blocks>& extractBlocks,
					const boost::shared_ptr<Blocks>& nbdBlocks) const;
	
	pipeline::Input<MserParameters> _mserParameters;
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _blockFactory;
	pipeline::Input<unsigned int> _maximumArea;
	pipeline::Input<Blocks> _blocks;
	
	pipeline::Output<Blocks> _neighborBlocks;
	pipeline::Output<SliceStoreResult> _count;
};

#endif //SLICE_GUARANTOR_H__
