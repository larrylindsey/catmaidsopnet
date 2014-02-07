#ifndef SOPNET_CATMAIDSOPNET_PERSISTENCE_LOCAL_STACK_STORE_H__
#define SOPNET_CATMAIDSOPNET_PERSISTENCE_LOCAL_STACK_STORE_H__

#include <string>
#include "StackStore.h"

class LocalStackStore : public StackStore {

public:

	LocalStackStore(std::string directory);

	pipeline::Value<ImageStack> getImageStack(const Box<>& box);

private:

	std::string _directory;
};

#endif // SOPNET_CATMAIDSOPNET_PERSISTENCE_LOCAL_STACK_STORE_H__

