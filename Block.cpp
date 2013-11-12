#include "Block.h"

Block::Block(unsigned int id, int min_x, int min_y, int min_z,
            int width, int height, int depth) : _id(id),
            _min_x(min_x), _min_y(min_y), _min_z(min_z),
            _width(width), _height(height), _depth(depth)
{
    
}

int
Block::xMin()
{
    return _min_x;
}

int
Block::yMin()
{
    return _min_y;
}

int
Block::zMin()
{
    return _min_z;
}

int
Block::width()
{
    return _width;
}

int
Block::height()
{
    return _height;
}

int
Block::depth()
{
    return _depth;
}

unsigned int
Block::getId()
{
    return _id;
}

int
*Block::size()
{
    int *sz = new int[3];
    sz[0] = _width;
    sz[1] = _height;
    sz[2] = _depth;
    return sz;
}

bool
Block::contains(int x, int y, int z)
{
  return x >= _min_x && (x - _min_x) < _width &&
         y >= _min_y && (y - _min_y) < _height &&
         z >= _min_z && (z - _min_z) < _depth;
}
