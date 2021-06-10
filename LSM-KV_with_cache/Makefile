
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -o2 -Wall

all: correctness persistence

correctness: kvstore.o correctness.o BloomFilter.o SSTable.o SkipList.o

persistence: kvstore.o persistence.o BloomFilter.o SSTable.o SkipList.o

clean:
	-rm -f correctness persistence BloomFilter SSTable SkipList *.o
