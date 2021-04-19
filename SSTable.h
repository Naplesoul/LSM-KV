#ifndef SSTABLE_H
#define SSTABLE_H

#include "BloomFilter.h"
#include "DataStructs.h"
#include <string>
#include <vector>


class SSTable
{
private:
    Header header;
    BloomFilter *bloomFilter;
    std::vector<Index> indexes;
    std::vector<std::string> values;
public:
    SSTable(): bloomFilter(new BloomFilter()) {}
    SSTable(Node* head);

    bool load(const std::string &dir);
    bool save(const std::string &dir);
    static std::vector<SSTable*> merge(const std::vector<SSTable*> &tables);
};


class SSTableCache
{
public:
    Header header;
    BloomFilter *bloomFilter;
    std::vector<Index> indexes;
    std::string path;
    SSTableCache(): bloomFilter(new BloomFilter()) {}
    SSTableCache(const std::string &dir);

    int get(const uint64_t &key);
    int find(const uint64_t &key, int start, int end);
};

bool timeCmp(SSTableCache *a, SSTableCache *b);


#endif // SSTABLE_H
