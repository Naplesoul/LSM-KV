#pragma once

#include "kvstore_api.h"
#include "SkipList.h"
#include "SSTable.h"
#include <vector>

class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
    SkipList *memTable;
    std::vector<std::vector<SSTableCache*>> cache;
    uint64_t currentTime;
    std::vector<uint32_t> tableCount;
    std::string dataDir;

    void saveMemTable();
    void compact();
public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

};
