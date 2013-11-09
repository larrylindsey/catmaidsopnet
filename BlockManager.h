#ifndef BLOCK_MANAGER_H__
#define BLOCK_MANAGER_H__

#include "Block.h"

class BlockManager
{
public:
    /* I don't like having this many parameters. Maybe a size struct is in order?
    Like:
    struct geom_size {
        int width;
        int height;
        int depth;
    };
    */

    // width - x dimension, height - y dimension, depth - z dimension
    BlockManager(int stackWidth, int stackHeight, int stackDepth,
                 int blockWidth, int blockHeight, int blockDepth);

    Block blockAtLocation(int x, int y, int z);
    
    Block blockAtOffset(Block block, int xOffset, int yOffset, int zOffset);
    
    
private:
    int _stackWidth, _stackHeight, _stackDepth;
    int _blockWidth, _blockHeight, _blockDepth;
};

#endif //BLOCK_MANAGER_H__

