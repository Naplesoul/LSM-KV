#include "BloomFilter.h"
#include "MurmurHash3.h"
#include <string.h>

void BloomFilter::add(const uint64_t &key)
{
    uint32_t hashVal[4] = {0};
    MurmurHash3_x64_128(&key, sizeof(key), 1, hashVal);
    (*bitSet)[hashVal[0] % FILTER_SIZE] = 1;
    (*bitSet)[hashVal[1] % FILTER_SIZE] = 1;
    (*bitSet)[hashVal[2] % FILTER_SIZE] = 1;
    (*bitSet)[hashVal[3] % FILTER_SIZE] = 1;
}

bool BloomFilter::contains(const uint64_t &key)
{
    uint32_t hashVal[4] = {0};
    MurmurHash3_x64_128(&key, sizeof(key), 1, hashVal);
    return ((*bitSet)[hashVal[0] % FILTER_SIZE] == 1
            && (*bitSet)[hashVal[1] % FILTER_SIZE] == 1
            && (*bitSet)[hashVal[2] % FILTER_SIZE] == 1
            && (*bitSet)[hashVal[3] % FILTER_SIZE] == 1);
}

void BloomFilter::save2Buffer(char *buf)
{
    strncpy(buf, (bitSet->to_string()).c_str(), FILTER_SIZE/8);
}

