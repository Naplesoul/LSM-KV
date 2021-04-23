#ifndef SSTABLE_H
#define SSTABLE_H

#include "BloomFilter.h"
#include "DataStructs.h"
#include <string>
#include <vector>
#include <list>

#define MAX_TABLE_SIZE 2097152


class SSTableCache
{
public:
    Header header;
    BloomFilter *bloomFilter;
    std::vector<Index> indexes;
    std::string path;

    ~SSTableCache(){delete bloomFilter;}
    SSTableCache(): bloomFilter(new BloomFilter()) {}
    SSTableCache(const std::string &dir);

    int get(const uint64_t &key);
    int find(const uint64_t &key, int start, int end);
};

class SSTable
{
public:
    uint64_t timeStamp;
    std::string path;
    uint64_t size;
    uint64_t length;
    std::list<Entry> entries;


    SSTable(SSTableCache *cache);
    SSTable(): size(10272), length(0) {}
    static void merge(std::vector<SSTable> &tables);
    static SSTable merge2(SSTable &a, SSTable &b);

    std::vector<SSTableCache*> save(const std::string &dir);
    SSTableCache *saveSingle(const std::string &dir, const uint64_t &currentTime, const uint64_t &num);

    void add(const Entry &entry);
};

bool cacheTimeCompare(SSTableCache *a, SSTableCache *b);
bool tableTimeCompare(SSTable &a, SSTable &b);

#endif // SSTABLE_H
