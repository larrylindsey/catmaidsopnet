#ifndef BLOCK_H__
#define BLOCK_H__

#include <vector>

class Block
{
public:
    Block(unsigned int id, int min_x, int min_y, int min_z,
            int width, int height, int depth);

    int xMin();
    int yMin();
    int zMin();
    int width();
    int height();
    int depth();
    unsigned int getId() const;
    int *size();
    bool contains(int x, int y, int z);
	bool contains(int z);

private:
    int _min_x, _min_y, _min_z;
    int _width, _height, _depth;
    unsigned int _id;
};

#endif //BLOCK_H__
