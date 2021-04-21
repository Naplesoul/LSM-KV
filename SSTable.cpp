#include "SSTable.h"
#include <fstream>
#include <iostream>
#include <io.h>

SSTable::SSTable(Node* head)
{
    uint32_t offset = 0;
    Node *cur = head;
    bloomFilter = new BloomFilter();
    while(cur) {
        uint64_t key = cur->key;
        if(key > header.maxKey)
           header.maxKey = key;
        if(key < header.minKey)
           header.minKey = key;
        bloomFilter->add(key);
        indexes.push_back(Index(key, offset));
        values.push_back(cur->val);
        offset += (cur->val).size() + 1;
        cur = cur->right;
    }
}

SSTableCache::SSTableCache(const std::string &dir)
{
    path = dir;
    std::ifstream file(dir, std::ios::binary);
    if(!file) {
        printf("Fail to open file %s", dir.c_str());
        exit(-1);
    }
    // load header
    file.read((char*)&header.timeStamp, 8);
    file.read((char*)&header.size, 8);
    uint64_t length = header.size;
    file.read((char*)&header.minKey, 8);
    file.read((char*)&header.maxKey, 8);

    // load bloom filter
    char *filterBuf = new char[FILTER_SIZE/8];
    file.read(filterBuf, FILTER_SIZE/8);
    bloomFilter = new BloomFilter(filterBuf);


    char *indexBuf = new char[length * 12];
    file.read(indexBuf, length * 12);
    for(uint32_t i = 0; i < length; ++i) {
        indexes.push_back(Index(*(uint64_t*)(indexBuf + 12*i), *(uint32_t*)(indexBuf + 12*i + 8)));
    }

    delete[] filterBuf;
    delete[] indexBuf;
    file.close();

}

int SSTableCache::get(const uint64_t &key)
{
    if(!bloomFilter->contains(key))
        return -1;
    return find(key, 0, indexes.size() - 1);
}

int SSTableCache::find(const uint64_t &key, int start, int end)
{
    if(start > end)
        return -1;
    if(start == end) {
        if(indexes[start].key == key)
            return start;
        else
            return -1;
    }
    int mid = (start + end) / 2;
    if(indexes[mid].key == key)
        return mid;
    else if(indexes[mid].key < key)
        return find(key, mid + 1, end);
    else
        return find(key, start, mid - 1);
}

bool timeCmp(SSTableCache *a, SSTableCache *b)
{
    return (a->header).timeStamp > (b->header).timeStamp;
}
