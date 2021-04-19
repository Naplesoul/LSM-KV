#include "kvstore.h"

int main()
{
//    KVStore kvStore("./data");
//    kvStore.put(1, "1");
//    kvStore.put(2, "2");
//    kvStore.put(3, "3");

    SkipList skipList;
    skipList.put(1, "1");
    skipList.put(2, "2");
    skipList.put(3, "3");
    skipList.save2SSTable(".", 1);
}
