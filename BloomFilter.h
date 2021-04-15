#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <bitset>
#include "MurmurHash3.h"


class BloomFilter
{
private:
    std::bitset<81920> bitSet;
public:
    void insert(const uint64_t &key)
    {
        uint32_t hashVal[4] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hashVal);
        bitSet[hashVal[0] % 81920] = 1;
        bitSet[hashVal[1] % 81920] = 1;
        bitSet[hashVal[2] % 81920] = 1;
        bitSet[hashVal[3] % 81920] = 1;
    }

    bool contains(const uint64_t &key)
    {
        uint32_t hashVal[4] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hashVal);
        if (bitSet[hashVal[0] % 81920] == 1
                && bitSet[hashVal[1] % 81920] == 1
                && bitSet[hashVal[2] % 81920] == 1
                && bitSet[hashVal[3] % 81920] == 1)
            return true;
        else
            return false;
    }
};


#endif // BLOOMFILTER_H
