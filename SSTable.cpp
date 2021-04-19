#include "SSTable.h"

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
    bloomFilter = new BloomFilter();
}

int SSTableCache::get(const uint64_t &key)
{
    if(!bloomFilter->contains(key))
        return 0;
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
    else if(indexes[mid].key > key)
        return find(key, mid + 1, end);
    else
        return find(key, start, mid - 1);
}

bool timeCmp(SSTableCache *a, SSTableCache *b)
{
    return (a->header).timeStamp > (b->header).timeStamp;
}
