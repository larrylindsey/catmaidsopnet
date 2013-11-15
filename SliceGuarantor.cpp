#include "SliceGuarantor.h"

#include <sopnet/slices/SliceExtractor.h>
#include <sopnet/slices/Slice.h>

SliceGuarantor::SliceGuarantor() : _stackReader(boost::make_shared<ImageBlockStackReader>())
{
	registerInput(_block, "block");
	registerInput(_sliceStore, "store");
	registerInput(_blockFactory, "block factory");
	registerInput(_forceExplanation, "force explanation");
	
	_stackReader->setInput("factory", _blockFactory);
	_stackReader->setInput("block", _block);
}

bool
SliceGuarantor::guaranteeSlices()
{
	int zMin = _block->zMin();
	std::vector<boost::shared_ptr<ProcessNode> > sliceExtractors;
	boost::shared_ptr<ProcessNode> sliceImageExtractor = boost::make_shared<ImageExtractor>();
	sliceImageExtractor->setInput(_stackReader->getOutput().getAssignedOutput());
	
	
	for (unsigned int i = 0; i < _block->depth(); ++i)
	{
		unsigned int z = i + zMin;
		boost::shared_ptr<ProcessNode> sliceExtractor =
			 boost::make_shared<SliceExtractor<unsigned char> >(z);
		sliceExtractor->setInput("membrane", sliceImageExtractor->getInput(i));
		sliceExtractor->setInput("force explanation", _forceExplanation);
		sliceExtractors.push_back(sliceExtractor);
	}
	
	return false;
}

