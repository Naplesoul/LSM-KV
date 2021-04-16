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
    SSTable(){bloomFilter = new BloomFilter();}
    SSTable(Node* head);

    bool load(const std::string &dir);
    bool save(const std::string &dir);
    static std::vector<SSTable*> merge(const std::vector<SSTable*> &tables);
};


#endif // SSTABLE_H
