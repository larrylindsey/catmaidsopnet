#include "SliceReader.h"

SliceReader::SliceReader()
{
	registerInput(_block, "block");
	registerInput(_store, "store");
	registerOutput(_slices, "slices");
	registerOutput(_constraints, "linear constraints");
}

void SliceReader::updateOutputs()
{
	*_slices = *(_store->retrieveSlices(_block));
	*_constraints = *(_store->retrieveConstraints(_slices));
}
