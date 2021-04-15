#pragma once

#include "kvstore_api.h"
#include "SkipList.h"

class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
    SkipList<uint64_t, std::string> memTable;
public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

};
