#include <imageprocessing/io/ImageStackDirectoryReader.h>
#include "LocalStackStore.h"

LocalStackStore::LocalStackStore(std::string directory) :
	_directory(directory) {}

pipeline::Value<ImageStack>
LocalStackStore::getImageStack(const Box<>& box) {

	pipeline::Process<ImageStackDirectoryReader> reader(_directory);

	// TODO:
	// crop to box

	return reader->getOutput();
}
