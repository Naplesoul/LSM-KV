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
