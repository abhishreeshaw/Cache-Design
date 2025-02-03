#ifndef CACHE_H_
#define CACHE_H_

#include <stdio.h>
#include <stdbool.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

using namespace std;

#define UNDEFINED 0xFFFFFFFFFFFFFFFF //constant used for initialization

typedef enum {WRITE_BACK, WRITE_THROUGH, WRITE_ALLOCATE, NO_WRITE_ALLOCATE} write_policy_t; 

typedef enum {HIT, MISS} access_type_t;

typedef long long address_t; //memory address type

struct CacheLine_dirtybit {
    bool valid;
    address_t tag_1;
    unsigned dirty_bit;
    unsigned lru;
    };

struct CacheLine {
    bool valid;
    address_t tag_2;
    unsigned lru;
 };



class cache{

	/* Add the data members required by your simulator's implementation here */
	// <TODO>
	unsigned sets, block_offset_bits, index_bits, tag_bits;
	unsigned index, min,memory_access,empty,eviction_set,evic_set; int read_hit_flag,write_hit_flag;
	address_t tag;
	unsigned first_access;
	/* number of memory accesses processed */
	unsigned number_memory_accesses;
	int count_evict=0;
int x,y=0;
int read_miss,write_miss=0;
	int rd_miss, wr_miss;
	int tryv =0;//adsin 
	/* trace file input stream */	
	ifstream stream;
        unsigned c_size, c_asso, c_line_size, c_hit_time, c_miss_penalty,a_width;
	write_policy_t w_hit;
	write_policy_t w_miss;
	std::vector<std::vector<CacheLine_dirtybit> > writeback;
	std::vector<std::vector<CacheLine> > writethrough;





public:

	/* 
	* Instantiates the cache simulator 
        */
	int debugreads=0;
	int debugreadhits=0;
	int debugreadmisses=0;
	
	int read_count, write_count;
	cache(unsigned cache_size, 		// cache size (in bytes)
              unsigned cache_associativity,     // cache associativity (fully-associative caches not considered)
	      unsigned cache_line_size,         // cache block size (in bytes)
	      write_policy_t write_hit_policy,  // write-back or write-through
	      write_policy_t write_miss_policy, // write-allocate or no-write-allocate
	      unsigned cache_hit_time,		// cache hit time (in clock cycles)
	      unsigned cache_miss_penalty,	// cache miss penalty (in clock cycles)	
	      unsigned address_width            // number of bits in memory address
	);
// Inside the constructor of the cache class

	// de-allocates the cache simulator
	~cache();

	// loads the trace file (with name "filename") so that it can be used by the "run" function  
	void load_trace(const char *filename);

	// processes "num_memory_accesses" memory accesses (i.e., entries) from the input trace 
	// if "num_memory_accesses=0" (default), then it processes the trace to completion 
	void run(unsigned num_memory_accesses=0);
	
	// processes a read operation and returns hit/miss
	access_type_t read(address_t address);
	
	// processes a write operation and returns hit/miss
	access_type_t write(address_t address);

	// returns the next block to be evicted from the cache
address_t calculate_tag(address_t address1);
unsigned calculate_index(address_t address2);
int hit_status1(unsigned set1, address_t tag1, unsigned way1);
int hit_status2(unsigned set2, address_t tag2, unsigned way2);
unsigned empty_block1(unsigned index_a, unsigned way_a);
unsigned empty_block2(unsigned index_b, unsigned way_b);
void update_cache1(unsigned a, unsigned b, address_t t);
void update_cache2(unsigned c, unsigned d, address_t ta);
unsigned eviction1(unsigned se, unsigned min);
unsigned eviction2(unsigned se, unsigned min);
void update_lru1(unsigned leave);
void update_lru2(unsigned leave);


		
	// prints the cache configuration
        void print_configuration();	// prints the execution statistics
	void print_statistics();

	//prints the metadata information (including "dirty" but, when applicable) for all valid cache entries  
	void print_tag_array();
};

#endif /*CACHE_H_*/

