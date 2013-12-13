#ifndef LOCAL_SLICE_STORE_H__
#define LOCAL_SLICE_STORE_H__

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <inference/Relation.h>

#include <catmaidsopnet/persistence/SliceStore.h>

/**
 * A SliceStore implemented locally in RAM for testing purposes.
 */

class LocalSliceStore : public SliceStore
{
	typedef boost::unordered_map<Slice, boost::shared_ptr<Blocks> > SliceBlockMap;
	typedef boost::unordered_map<Block, boost::shared_ptr<Slices> > BlockSliceMap;
	typedef boost::unordered_map<Slice, boost::shared_ptr<Slices> > ConflictMap;
	typedef boost::unordered_map<unsigned int, boost::shared_ptr<Slice> > IdSliceMap;
	

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
	class SliceStoreLinearConstraint
	{
	public:
		SliceStoreLinearConstraint(const LinearConstraint& constraint,
				const boost::shared_ptr<IdSliceMap>& idSliceMap);
		
		bool associated(const boost::shared_ptr<Slices>& slices);
		
		boost::shared_ptr<LinearConstraint> getConstraint(const boost::shared_ptr<Slices>& slices);
	private:
		boost::unordered_map<Slice, double> _coefs;
		double _value;
		Relation _relation;
	};
	
	void mapSliceToBlock(const boost::shared_ptr<Slice>& slice,
						 const boost::shared_ptr<Block>& block);
	void mapBlockToSlice(const boost::shared_ptr<Block>& block,
						 const boost::shared_ptr<Slice>& slice);
	void addConflict(const boost::shared_ptr<Slice>& slice,
					 const boost::shared_ptr<Slices>& slices);
	boost::shared_ptr<Slice> equivalentSlice(const boost::shared_ptr<Slice>& slice);
	
	boost::shared_ptr<SliceBlockMap> _sliceBlockMap;
	boost::shared_ptr<BlockSliceMap> _blockSliceMap;
	boost::shared_ptr<IdSliceMap> _idSliceMap;
	boost::shared_ptr<ConflictMap> _conflictMap;
	boost::unordered_set<Slice> _sliceMasterList;
	std::vector<boost::shared_ptr<SliceStoreLinearConstraint> > _constraints;
	
};

#endif //LOCAL_SLICE_STORE_H__