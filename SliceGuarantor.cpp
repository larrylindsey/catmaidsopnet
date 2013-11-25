#include "SliceGuarantor.h"

#include <imageprocessing/ImageExtractor.h>
#include <sopnet/sopnet/slices/SliceExtractor.h>
#include <sopnet/sopnet/slices/Slice.h>

/*
 * TODO
 * Things left to code:
 * - check if a Slice is whole
 * - rectify Slice coordinates to Block coordinates
 * - merge Slices
 * - SliceGuarantor input class
 *   * is this a user-generated guarantee or a propogated one?
 *   * for propogated, which slices need to be whole?
 * 
 * Question: do I need to overlap Slice extraction in order to perform the merge correctly?
 */

SliceGuarantor::SliceGuarantor() :
	_stackReader(boost::make_shared<ImageBlockStackReader>()),
	_blockSliceStore(boost::make_shared<BlockSliceStoreNode>())
{
	registerInput(_block, "block");
	registerInput(_sliceStore, "store");
	registerInput(_blockFactory, "block factory");
	registerInput(_forceExplanation, "force explanation");
	
	_stackReader->setInput("factory", _blockFactory);
	_stackReader->setInput("block", _block);
	
	_blockSliceStore->addInput("block", _block);
	_blockSliceStore->setInput("store", _sliceStore);
	
	
}

void
SliceGuarantor::guaranteeSlices()
{
	if (!_block->setSlicesFlag(true))
	{
		int zMin = _block->location()->z;
		boost::shared_ptr<SlicesCollector> slicesCollector = boost::make_shared<SlicesCollector>();
		boost::shared_ptr<ProcessNode> sliceImageExtractor = boost::make_shared<ImageExtractor>();
		boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
		boost::shared_ptr<Slices> fragmentSlices = boost::make_shared<Slices>();
		
		sliceImageExtractor->setInput(_stackReader->getOutput());

		slicesCollector->setInput("block", _block);
		
		// Collect all Slice's
		for (unsigned int i = 0; i < _block->size()->z; ++i)
		{
			unsigned int z = i + zMin;
			boost::shared_ptr<ProcessNode> sliceExtractor =
				boost::make_shared<SliceExtractor<unsigned char> >(z);
			
			sliceExtractor->setInput("membrane", sliceImageExtractor->getInput(i));
			sliceExtractor->setInput("force explanation", _forceExplanation);
			
			slicesCollector->addInput("slices", sliceExtractor->getOutput("slices"));
		}
		
		//Separate Whole slices from fractured slices
		foreach(boost::shared_ptr<Slice> slice, *(slicesCollector->getSlices()))
		{
			checkWhole(slice);
			slices->add(slice);
		}
		
		_blockSliceStore->setInput("whole", boost::make_shared<pipeline::Wrap<bool> >(true));
		_blockSliceStore->setInput("slice in", slices);
		_blockSliceStore->storeSlices();
	}
}

void
SliceGuarantor::checkWhole(const boost::shared_ptr<Slice>& slice) const
{

}


SliceGuarantor::SlicesCollector::SlicesCollector()
{
	registerInput(_block, "block");
	registerInputs(_multiSlices, "slices");
}

boost::shared_ptr<Slices>
SliceGuarantor::SlicesCollector::getSlices()
{
	boost::shared_ptr<Slices> slices = boost::make_shared<Slices>();
	foreach (boost::shared_ptr<Slices> zSlices, _multiSlices)
	{
		slices->addAll(*zSlices);
	}
	return slices;
}
