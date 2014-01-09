#include "BlockSolver.h"
#include <boost/make_shared.hpp>
#include <util/ProgramOptions.h>
#include <pipeline/Value.h>


util::ProgramOption optionRandomForestFileBlock(
		util::_module           = "blockSolver",
		util::_long_name        = "segmentRandomForest",
		util::_description_text = "Path to an HDF5 file containing the segment random forest.",
		util::_default_value    = "segment_rf.hdf");

BlockSolver::BlockSolver() :
	_problemAssembler(boost::make_shared<ProblemAssembler>()),
	_constraintExtractor(boost::make_shared<ConsistencyConstraintExtractor>()),
	_randomForestHDF5Reader(
		boost::make_shared<RandomForestHdf5Reader>(optionRandomForestFileBlock.as<std::string>())),
	_reconstructor(boost::make_shared<Reconstructor>()),
	_linearSolver(boost::make_shared<LinearSolver>()),
	_neuronExtractor(boost::make_shared<NeuronExtractor>()),
	_segmentReader(boost::make_shared<SegmentReader>()),
	_sliceReader(boost::make_shared<SliceReader>()),
	_priorCostFunction(boost::make_shared<PriorCostFunction>()),
	_rawImageStackReader(boost::make_shared<ImageBlockStackReader>()),
	_membraneStackReader(boost::make_shared<ImageBlockStackReader>()),
	_randomForestCostFunction(boost::make_shared<RandomForestCostFunction>()),
	_objectiveGenerator(boost::make_shared<ObjectiveGenerator>()),
	_segmentFeaturesExtractor(boost::make_shared<SegmentFeaturesExtractor>())
{
	registerInput(_priorCostFunctionParameters, "prior cost parameters");
	registerInput(_blocks, "blocks");
	
	registerInput(_segmentStore, "segment store");
	registerInput(_sliceStore, "slice store");
	registerInput(_rawImageFactory, "raw image factory");
	registerInput(_membraneFactory, "membrane factory");
	
	registerOutput(_neurons, "neurons");
}

void
BlockSolver::updateOutputs()
{
	boost::shared_ptr<LinearSolverParameters> binarySolverParameters = 
		boost::make_shared<LinearSolverParameters>(Binary);
	pipeline::Value<SegmentTrees> neurons;
	boost::shared_ptr<pipeline::Wrap<util::point3<unsigned int> > > offset;
		
	boost::shared_ptr<Blocks> boundingBlocks;
		
		
	_segmentReader->setInput("blocks", _blocks);
	_sliceReader->setInput("blocks", _blocks);
	
	_segmentReader->setInput("block manager", _blocks);
	_sliceReader->setInput("block manager", _blocks);
	
	_segmentReader->setInput("store", _segmentStore);
	_sliceReader->setInput("store", _sliceStore);
	_rawImageStackReader->setInput("factory", _rawImageFactory);
	_membraneStackReader->setInput("factory", _membraneFactory);
	
	_constraintExtractor->setInput("slices", _sliceReader->getOutput("slices"));
	_constraintExtractor->setInput("segments", _segmentReader->getOutput("segments"));
	
	_problemAssembler->addInput("segments", _segmentReader->getOutput("segments"));
	_problemAssembler->addInput("linear constraints",
								_constraintExtractor->getOutput("linear constraints"));
	
	// Use problem assembler output to compute the bounding box we need to contain all
	// of the slices that are present. This will usually be larger than the requested
	// Blocks.
	boundingBlocks = computeBound();
	offset = boost::make_shared<pipeline::Wrap<util::point3<unsigned int> > >(
		boundingBlocks->location());
	_rawImageStackReader->setInput("block", boundingBlocks);
	_membraneStackReader->setInput("block", boundingBlocks);
	
	_segmentFeaturesExtractor->setInput("crop offset", offset);
	_segmentFeaturesExtractor->setInput("segments", _problemAssembler->getOutput("segments"));
	_segmentFeaturesExtractor->setInput("raw sections", _rawImageStackReader->getOutput());
	
	_priorCostFunction->setInput("parameters", _priorCostFunctionParameters);
	
	_randomForestCostFunction->setInput("random forest",
										_randomForestHDF5Reader->getOutput("random forest"));
	_randomForestCostFunction->setInput("features",
										_segmentFeaturesExtractor->getOutput("all features"));
	
	
	_objectiveGenerator->setInput("segments", _problemAssembler->getOutput("segments"));
	_objectiveGenerator->setInput("segment cost function",
								  _randomForestCostFunction->getOutput("cost function"));
	_objectiveGenerator->addInput("additional cost functions",
								  _priorCostFunction->getOutput("cost function"));
	
	_linearSolver->setInput("objective", _objectiveGenerator->getOutput());
	_linearSolver->setInput("linear constraints", _problemAssembler->getOutput("linear constraints"));
	_linearSolver->setInput("parameters", binarySolverParameters);
	
	_reconstructor->setInput("segments", _problemAssembler->getOutput("segments"));
	_reconstructor->setInput("solution", _linearSolver->getOutput());

	_neuronExtractor->setInput("segments", _reconstructor->getOutput());
	
	std::cout << " GO! " << std::endl;
	
	neurons = _neuronExtractor->getOutput();
	*_neurons = *neurons;
}

boost::shared_ptr<Blocks>
BlockSolver::computeBound()
{
	pipeline::Value<Segments> segments = _problemAssembler->getOutput("segments");
	if (segments->size() > 0)
	{
			
		util::rect<int> bound =
			segments->getSegments()[0]->getSlices()[0]->getComponent()->getBoundingBox();
		boost::shared_ptr<Box<> > box;
		boost::shared_ptr<Blocks> boundBlocks;
			
		foreach (boost::shared_ptr<Segment> segment, segments->getSegments())
		{
			foreach (boost::shared_ptr<Slice> slice, segment->getSlices())
			{
				util::rect<int> componentBound = slice->getComponent()->getBoundingBox();
				bound.fit(componentBound);
			}
		}
		
		box = boost::make_shared<Box<> >(bound, _blocks->location().z, _blocks->size().z);
		
		boundBlocks = _blocks->getManager()->blocksInBox(box);
		return boundBlocks;
	}
	else
	{
		return _blocks;
	}
}


