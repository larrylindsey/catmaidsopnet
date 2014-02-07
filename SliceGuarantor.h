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

class SliceGuarantor : public pipeline::SimpleProcessNode<>
{
	typedef boost::unordered_map<ConnectedComponent, boost::shared_ptr<Slice> >  ComponentSliceMap;
public:

    SliceGuarantor();

private:
	
	void updateOutputs();
	
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
	pipeline::Input<Box<> > _box;
	pipeline::Input<bool> _forceExplanation;
	pipeline::Input<unsigned int> _maximumArea;
	pipeline::Input<BlockManager> _blockManager;
	
	pipeline::Output<Blocks> _neighborBlocks;
	pipeline::Output<SliceStoreResult> _count;
};

#endif //SLICE_GUARANTOR_H__
