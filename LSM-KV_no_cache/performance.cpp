#include "kvstore.h"
#include <stdlib.h>
#include <ctime>

int main(int argc, char *argv[])
{
    KVStore store("./data");
    for (uint64_t i = 0; i < 10000; ++i) {
        clock_t start = clock();

        clock_t end = clock();
        double latency = (double)(end - start) / CLOCKS_PER_SEC;

    }
    return 0;
}
