#ifndef LOCAL_SLICE_STORE_H__
#define LOCAL_SLICE_STORE_H__

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/SliceStore.h>

typedef boost::unordered_map<Slice, boost::shared_ptr<Blocks> > SliceBlockMap;
typedef boost::unordered_map<Block, boost::shared_ptr<Slices> > BlockSliceMap;
typedef boost::unordered_map<unsigned int, boost::shared_ptr<Slice> > IdSliceMap;

class LocalSliceStore : public SliceStore
{
public:
	LocalSliceStore();
	
	void storeSlice(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block);

	void storeSlice(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Blocks>& blocks);

	boost::shared_ptr<Slice> retrieveSlice(unsigned int sliceId);

	boost::shared_ptr<Slices> retrieveSlices(const boost::shared_ptr<Blocks>& blocks);
	
	void removeSliceFromBlocks(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Blocks>& blocks);
	
	void removeSlice(const boost::shared_ptr<Slice>& slice);
	
	boost::shared_ptr<Blocks> getAssociatedBlocks(const boost::shared_ptr<Slice>& slice);

private:
	void removeBlockFromVector(const boost::shared_ptr<Block>& block, const boost::shared_ptr<Blocks>& vector);

	boost::shared_ptr<SliceBlockMap> _sliceBlockMap;
	boost::shared_ptr<IdSliceMap> _idSliceMap;
	boost::shared_ptr<BlockSliceMap> _blockSliceMap;
};

#endif //LOCAL_SLICE_STORE_H__