#include "SliceWriter.h"

#include <boost/shared_ptr.hpp>
#include <sopnet/slices/Slice.h>

SliceWriter::SliceWriter()
{
	registerInput(_block, "block");
	registerInput(_slices, "slices");
	registerInput(_store, "store");
	registerOutput(_count, "count");
}

void
SliceWriter::updateOutputs()
{
	int count = 0;
	foreach (boost::shared_ptr<Slice> slice, *_slices)
	{
		_store->associate(slice, _block);
		++count;
	}
	
	*_count = SliceStoreResult(count);
}
