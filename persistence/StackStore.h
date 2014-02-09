#ifndef SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__
#define SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__

#include <pipeline/Data.h>
#include <pipeline/Value.h>
#include <imageprocessing/ImageStack.h>
#include <sopnet/block/Box.h>

/**
 * Database abstraction for image stacks.
 */
class StackStore : public pipeline::Data
{
public:
	virtual pipeline::Value<ImageStack> getImageStack(const Box<>& box);
	
protected:
	
	virtual boost::shared_ptr<Image> getImage(const util::rect<unsigned int> bound,
											  const unsigned int section) = 0;
	
	
};

#endif // SOPNET_CATMAIDSOPNET_PERSISTENCE_STACK_STORE_H__

