#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <stdint.h>
#include <string>

struct Entry
{
    uint64_t key;
    std::string val;
    Entry(uint64_t k, std::string v): key(k), val(v) {}
};

struct Header
{
    uint64_t timeStamp;
    uint64_t size;
    uint64_t minKey, maxKey;
    Header(): timeStamp(0), size(0), minKey(0), maxKey(0){}
};

struct Index
{
    uint64_t key;
    uint32_t offset;
    Index(uint64_t k = 0, uint32_t o = 0): key(k), offset(o) {}
};


struct Node{
    Node *right, *down;   //向右向下足矣
    uint64_t key;
    std::string val;
    Node(Node *right, Node *down, uint64_t key, std::string val): right(right), down(down), key(key), val(val){}
    Node(): right(nullptr), down(nullptr) {}
};


#endif // DATASTRUCTS_H
