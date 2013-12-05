#ifndef LOCAL_SLICE_STORE_H__
#define LOCAL_SLICE_STORE_H__

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <catmaidsopnet/persistence/SliceStore.h>

typedef boost::unordered_map<Slice, boost::shared_ptr<Blocks> > SliceBlockMap;
typedef boost::unordered_map<Block, boost::shared_ptr<Slices> > BlockSliceMap;

class LocalSliceStore : public SliceStore
{
public:
	LocalSliceStore();

    void associate(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block);

    boost::shared_ptr<Slices> retrieveSlices(const boost::shared_ptr<Block>& block);

	void disassociate(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block);

	void removeSlice(const boost::shared_ptr<Slice>& slice);

	boost::shared_ptr<Blocks> getAssociatedBlocks(const boost::shared_ptr<Slice>& slice);


private:
	void mapSliceToBlock(const boost::shared_ptr<Slice>& slice, const boost::shared_ptr<Block>& block);
	void mapBlockToSlice(const boost::shared_ptr<Block>& block, const boost::shared_ptr<Slice>& slice);
	
	boost::shared_ptr<SliceBlockMap> _sliceBlockMap;
	boost::shared_ptr<BlockSliceMap> _blockSliceMap;
};

#endif //LOCAL_SLICE_STORE_H__