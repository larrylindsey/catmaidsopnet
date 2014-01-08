#ifndef BLOCK_SOLVER_H__
#define BLOCK_SOLVER_H__

#include <boost/shared_ptr.hpp>
#include <pipeline/all.h>
#include <imageprocessing/io/ImageBlockFactory.h>
#include <imageprocessing/io/ImageBlockStackReader.h>
#include <sopnet/inference/PriorCostFunctionParameters.h>
#include <sopnet/inference/ProblemAssembler.h>
#include <sopnet/inference/LinearSolver.h>
#include <sopnet/inference/Reconstructor.h>
#include <sopnet/inference/PriorCostFunction.h>
#include <sopnet/inference/ProblemAssembler.h>
#include <sopnet/inference/RandomForestCostFunction.h>
#include <sopnet/inference/ObjectiveGenerator.h>
#include <sopnet/inference/io/RandomForestHdf5Reader.h>
#include <sopnet/neurons/NeuronExtractor.h>
#include <sopnet/block/Blocks.h>
#include <sopnet/block/Box.h>
#include <sopnet/block/BlockManager.h>
#include <sopnet/features/SegmentFeaturesExtractor.h>
#include <catmaidsopnet/persistence/SegmentStore.h>
#include <catmaidsopnet/persistence/SliceStore.h>
#include <catmaidsopnet/ConsistencyConstraintExtractor.h>
#include <catmaidsopnet/persistence/SegmentReader.h>
#include <catmaidsopnet/persistence/SliceReader.h>

class BlockSolver : public pipeline::ProcessNode
{
public:
	BlockSolver();
	
private:
	void onBoxSet(pipeline::InputSetBase&);
	void onBlocksSet(pipeline::InputSetBase&);
	void onBlockManagerSet(pipeline::InputSetBase&);
	
	pipeline::Input<PriorCostFunctionParameters> _priorCostFunctionParameters;
	pipeline::Input<Blocks> _blocks;
	pipeline::Input<Box<> > _box;
	pipeline::Input<BlockManager> _blockManager;
	pipeline::Input<SegmentStore> _segmentStore;
	pipeline::Input<SliceStore> _sliceStore;
	pipeline::Input<ImageBlockFactory> _rawImageFactory;
	pipeline::Input<ImageBlockFactory> _membraneFactory;
	
	boost::shared_ptr<ProblemAssembler> _problemAssembler;
	boost::shared_ptr<ConsistencyConstraintExtractor> _constraintExtractor;
	boost::shared_ptr<Reconstructor> _reconstructor;
	boost::shared_ptr<LinearSolver> _linearSolver;
	boost::shared_ptr<NeuronExtractor> _neuronExtractor;
	boost::shared_ptr<SegmentReader> _segmentReader;
	boost::shared_ptr<SliceReader> _sliceReader;
	boost::shared_ptr<PriorCostFunction> _priorCostFunction;
	boost::shared_ptr<RandomForestHdf5Reader> _randomForestHDF5Reader;
	boost::shared_ptr<ImageBlockStackReader> _rawImageStackReader;
	boost::shared_ptr<ImageBlockStackReader> _membraneStackReader;
	boost::shared_ptr<RandomForestCostFunction> _randomForestCostFunction;
	boost::shared_ptr<SegmentFeaturesExtractor> _segmentFeaturesExtractor;
	boost::shared_ptr<ObjectiveGenerator> _objectiveGenerator;
};

#endif //BLOCK_SOLVER_H__