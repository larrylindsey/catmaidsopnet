#include "SliceGuarantor.h"


SliceGuarantor::SliceGuarantor()
{
}

void
SliceGuarantor::onInputSet(const pipeline::InputSet<Slices>& signal)
{
    
}

void
SliceGuarantor::guaranteeSlices()
{
}

void 
SliceGuarantor::updateOutputs()
{
    guaranteeSlices();
}
