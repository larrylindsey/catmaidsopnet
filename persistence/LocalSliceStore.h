#ifndef LOCAL_SLICE_STORE_H__
#define LOCAL_SLICE_STORE_H__

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/persistence/SliceStore.h>


class LocalSliceStore : public SliceStore
{
	typedef boost::unordered_map<Slice, boost::shared_ptr<Blocks> > SliceBlockMap;
	typedef boost::unordered_map<Block, boost::shared_ptr<Slices> > BlockSliceMap;

public:
	LocalSliceStore();

    void associate(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block);

    boost::shared_ptr<Slices> retrieveSlices(const boost::shared_ptr<Block>& block);

	void disassociate(const boost::shared_ptr<Slice>& slice,
					  const boost::shared_ptr<Block>& block);

	void removeSlice(const boost::shared_ptr<Slice>& slice);

	boost::shared_ptr<Blocks> getAssociatedBlocks(const boost::shared_ptr<Slice>& slice);
	
    boost::shared_ptr<LinearConstraints> retrieveConstraints(
		const boost::shared_ptr<Slices>& slices);
	
	void storeConstraints(const boost::shared_ptr<LinearConstraints>& constraints);
	
	void storeConflicts(const boost::shared_ptr<Slices>& slices);


private:
	void mapSliceToBlock(const boost::shared_ptr<Slice>& slice,
						 const boost::shared_ptr<Block>& block);
	void mapBlockToSlice(const boost::shared_ptr<Block>& block,
						 const boost::shared_ptr<Slice>& slice);
	
	boost::shared_ptr<SliceBlockMap> _sliceBlockMap;
	boost::shared_ptr<BlockSliceMap> _blockSliceMap;
};

#endif //LOCAL_SLICE_STORE_H__