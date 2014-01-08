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
	_objectiveGenerator(boost::make_shared<ObjectiveGenerator>())
{
	registerInput(_priorCostFunctionParameters, "prior cost parameters");
	registerInput(_box, "box", pipeline::Optional);
	registerInput(_blocks, "blocks", pipeline::Optional);
	registerInput(_blockManager, "block manager", pipeline::Optional);
	
	registerInput(_segmentStore, "segment store");
	registerInput(_sliceStore, "slice store");
	registerInput(_rawImageFactory, "raw image factory");
	registerInput(_membraneFactory, "membrane factory");
	
	registerOutput(_neurons, "neurons");
	
	
	_blocks.registerBackwardCallback(&BlockSolver::onBlocksSet, this);
	_box.registerBackwardCallback(&BlockSolver::onBoxSet, this);
	_blockManager.registerBackwardCallback(&BlockSolver::onBlockManagerSet, this);
}

void
BlockSolver::updateOutputs()
{
	boost::shared_ptr<LinearSolverParameters> binarySolverParameters = 
		boost::make_shared<LinearSolverParameters>(Binary);
	pipeline::Value<SegmentTrees> neurons;
	
	_segmentReader->setInput("store", _segmentStore);
	_sliceReader->setInput("store", _sliceStore);
	_rawImageStackReader->setInput("factory", _rawImageFactory);
	_membraneStackReader->setInput("factory", _membraneFactory);
	
	_constraintExtractor->setInput("slices", _sliceReader->getOutput("slices"));
	_constraintExtractor->setInput("segments", _segmentReader->getOutput("segments"));
	
	_problemAssembler->setInput("segments", _segmentReader->getOutput("segments"));
	_problemAssembler->setInput("linear constraints",
								_constraintExtractor->getOutput("linear constraints"));
	
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

	_neuronExtractor->setInput("solution", _reconstructor->getOutput());
	
	neurons = _neuronExtractor->getOutput();
	*_neurons = *neurons;
}


void
BlockSolver::onBlockManagerSet(pipeline::InputSetBase& )
{
	_segmentReader->setInput("block manager", _blockManager);
	_sliceReader->setInput("block manager", _blockManager);
}

void
BlockSolver::onBlocksSet(pipeline::InputSetBase& )
{
	_segmentReader->setInput("blocks", _blocks);
	_sliceReader->setInput("blocks", _blocks);
	_rawImageStackReader->setInput("box", _blocks);
	_membraneStackReader->setInput("box", _blocks);
}

void
BlockSolver::onBoxSet(pipeline::InputSetBase& )
{
	_segmentReader->setInput("box", _blocks);
	_sliceReader->setInput("box", _blocks);
	_rawImageStackReader->setInput("box", _blocks);
	_membraneStackReader->setInput("box", _blocks);
}
