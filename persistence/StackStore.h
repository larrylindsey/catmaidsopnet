#ifndef SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__
#define SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__

#include <pipeline/Data.h>
#include <pipeline/Value.h>
#include <imageprocessing/ImageStack.h>
#include <sopnet/block/Box.h>

/**
 * Database abstraction for image stacks.
 */
class StackStore : public pipeline::Data {

public:

	virtual pipeline::Value<ImageStack> getImageStack(const Box<>& box) = 0;
};

#endif // SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__

