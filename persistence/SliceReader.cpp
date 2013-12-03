#include "SliceReader.h"

SliceReader::SliceReader()
{
	registerInput(_block, "block");
	registerInput(_store, "store");
	registerOutput(_slices, "slices");
}

void SliceReader::updateOutputs()
{
	*_slices = *(_store->retrieveSlices(_block));
}
