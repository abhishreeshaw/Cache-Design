#include "cache.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace std;

cache::cache(unsigned size,
			 unsigned associativity,
			 unsigned line_size,
			 write_policy_t wr_hit_policy,
			 write_policy_t wr_miss_policy,
			 unsigned hit_time,
			 unsigned miss_penalty,
			 unsigned address_width)
{
	c_size = size;
	c_asso = associativity;
	c_hit_time = hit_time;
	c_miss_penalty = miss_penalty;
	a_width = address_width;
	c_line_size = line_size;
	w_hit = wr_hit_policy;
	w_miss = wr_miss_policy;
	sets = (size / (associativity * line_size));
	block_offset_bits = log2(line_size);
	index_bits = log2(sets);
	tag_bits = address_width - index_bits - block_offset_bits;
	writeback.resize(sets, std::vector<CacheLine_dirtybit>(c_asso));
	writethrough.resize(sets, std::vector<CacheLine>(c_asso));
}

void cache::print_configuration()
{
	std::cout << "CACHE CONFIGURATION" << std::endl;
	std::cout << "size =" << " " << (c_size / 1024) << " " << "KB" << std::endl; 
	std::cout << "associativity =" << " " << c_asso << "-way" << std::endl;
	std::cout << "cache line size =" << " " << c_line_size << "B" << std::endl; 
	std::cout << "write hit policy =" << " " << w_hit << std::endl;
	std::cout << "write miss policy =" << " " << w_miss << std::endl; 
	std::cout << "cache hit time =" << " " << c_hit_time << "CLK" << std::endl;
	std::cout << "cache miss penalty ="  << " " << c_miss_penalty << "CLK" << std::endl;
	std::cout << "memory address width ="<< " " << a_width << "bits" << std::endl;
}
cache::~cache()
{
	
}

void cache::load_trace(const char *filename)
{
	stream.open(filename);
}

void cache::run(unsigned num_entries)
{

	first_access = number_memory_accesses;
	string line;
	unsigned line_nr = 0;
	read_count = 0;
	write_count = 0;
	read_miss = 0;
	write_miss = 0;
	memory_access = 0;
	rd_miss = wr_miss = 0;

	// Initialize with 0 the initial cache configuration
	for (unsigned j = 0; j < sets; j++)
	{
		for (unsigned i = 0; i < c_asso; i++)
		{
			writeback[j][i].tag_1 = 0;
			writeback[j][i].valid = 0;
			writeback[j][i].dirty_bit = 0;
			writeback[j][i].lru = 0;
			writethrough[j][i].tag_2 = 0;
			writethrough[j][i].valid = 0;
			writethrough[j][i].lru = 0;
		}
	}

	while (getline(stream, line))
	{

		// Inputing line from file and making it string
		line_nr++;
		char *str = const_cast<char *>(line.c_str());

		// tokenize the instruction
		// Operation
		char *op = strtok(str, " ");
		// Address
		char *addr = strtok(NULL, " ");

		// Convert string adddress to hex integer
		address_t address = strtoull(addr, NULL, 16);

		// you can update the statistics here or in the write/read functions
		// If operation is write
		if (!strcmp(op, "w"))
		{

			write_count++;
			x++;
			access_type_t ans1 = write(address);

			if (ans1 == MISS)
				write_miss++;
		}
		else
		{
			read_count++;
			y++;
			access_type_t ans2 = read(address);
			if (ans2 == MISS)
				read_miss++;
		}

		number_memory_accesses++;
		if (num_entries != 0 && (number_memory_accesses - first_access) == num_entries)
			break;
	}
}

void cache::print_statistics()
{
	cout << "STATISTICS" << endl;
	cout << "num memory accesses ="
		 << " " << std::dec << number_memory_accesses << std::endl;
	cout << "read = " << std::dec << y << std::endl;
	cout << "read misses ="
		 << " " << rd_miss << std::endl;
	cout << "write ="
		 << " " << std::dec << x << std::endl;
	cout << "write misses ="
		 << " " << write_miss << std::endl;
	cout << "evictions "
		 << " " << std::dec << count_evict << std::endl;
	

}
//calculate tag of address
address_t cache::calculate_tag(address_t address1)
{
	return ((address1 >> (block_offset_bits + index_bits)));
}
//calculate index of address
unsigned cache::calculate_index(address_t address2)
{
	return ((address2 >> block_offset_bits) & ((1 << index_bits) - 1));
}

// hit check for  writeback cache
int cache::hit_status1(unsigned set1, address_t tag1, unsigned way1)
{

	if (writeback[set1][way1].tag_1 == tag1 && writeback[set1][way1].valid == 1) // adsin
		return 1;
	else
		return 0;
}

// hit check for write through cache
int cache::hit_status2(unsigned set2, address_t tag2, unsigned way2)
{
	if (writethrough[set2][way2].tag_2 == tag2 && writethrough[set2][way2].valid == 1)
		return 1;
	else
		return 0;
}
//check for empty block in set in writeback cache
unsigned cache::empty_block1(unsigned index_a, unsigned way_a)
{
	if (writeback[index_a][way_a].valid == 0)
		return 1;
	else
		return 0;
}
//check for empty block in set in write through cache
unsigned cache::empty_block2(unsigned index_b, unsigned way_b)
{
	if (writethrough[index_b][way_b].valid == 0)
		return 1;
	else
		return 0;
}
//updat eblock content in case of miss
void cache::update_cache1(unsigned a, unsigned b, address_t t)
{
	writeback[a][b].valid = 1;
	memory_access++;
	writeback[a][b].tag_1 = t;
	writeback[a][b].dirty_bit = 0;
	writeback[a][b].lru = 0;
}
//update block content of write through cache in case of miss
void cache::update_cache2(unsigned c, unsigned d, address_t ta)
{
	writethrough[c][d].valid = 1;
	memory_access++;
	writethrough[c][d].tag_2 = ta;
	writethrough[c][d].lru = 0;
}
//to find least recently used block in writethrough cache
unsigned cache::eviction1(unsigned se, unsigned min)
{
	memory_access++;
	count_evict++;
	int temp = 0;
	for (unsigned i = 0; i < c_asso; i++)
		if (writeback[se][i].lru > writeback[se][temp].lru)
			temp = i;

	return temp;
}
//to find least recently used block in write through cache
unsigned cache::eviction2(unsigned se, unsigned min)
{
	memory_access++;
	count_evict++;

	for (unsigned i = 0; i < c_asso; i++)
		if (writethrough[se][i].lru > writethrough[se][min].lru)
			min = i;

	return min;
}
//to update lru fiels of blocks
void cache::update_lru1(unsigned leave)
{
	for (unsigned i = 0; i < c_asso; i++)
	{
		if (i != leave)
			writeback[index][i].lru++;
	}
}
void cache::update_lru2(unsigned leave)
{
	for (unsigned i = 0; i < c_asso; i++)
	{
		if (i != leave)
			writethrough[index][i].lru++;
	}
}

access_type_t cache::read(address_t address)
{
	debugreads++;
	tag = (address >> (block_offset_bits + index_bits));
	index = calculate_index(address);
	read_hit_flag = 0;
	empty = 0;

	switch (w_hit)
	{
	case WRITE_BACK:
		eviction_set = 0;
		for (unsigned i = 0; i < c_asso; i++)
		{
			read_hit_flag = hit_status1(index, tag, i);
			if (read_hit_flag == 1)
			{
				tryv++;
				// read hit, most recent used lru made 0, then break
				writeback[index][i].lru = 0;
				// write update lru function to increment all other blocks.
				update_lru1(i);
				break;
			}
		}
		if (read_hit_flag == 1)
		{	debugreadhits++; 
			return HIT;}
		else
		{       debugreadmisses++;
			rd_miss++;//read miss block
			for (unsigned i = 0; i < c_asso; i++)
			{
				empty = empty_block1(index, i);

				if (empty == 1)
				{
					update_cache1(index, i, tag);
					update_lru1(i);
					break;//empty block index found
				}
			}

			if (empty == 0)
			{//no empty block present need to evict
				evic_set = eviction1(index, eviction_set);
				update_cache1(index, evic_set, tag);
				update_lru1(evic_set);
			}
			return MISS;
		}

		break;
	case WRITE_THROUGH:
		eviction_set = 0;
		for (unsigned i = 0; i < c_asso; i++)
		{
			read_hit_flag = hit_status2(index, tag, i);
			if (read_hit_flag == 1)
			{
				writethrough[index][i].lru = 0;
				update_lru2(i);
				break;//read hit
			}
		}
		if (read_hit_flag == 1)
			return HIT;
		else
		{
			rd_miss++;//read miss
			for (unsigned i = 0; i < c_asso; i++)
			{
				empty = empty_block2(index, i);
				if (empty == 1)
				{
					update_cache2(index, i, tag);
					update_lru2(i);
					break;//empty cache block
				}
			}

			if (empty == 0)
			{
				evic_set = eviction2(index, eviction_set);
				update_cache2(index, evic_set, tag);
				update_lru2(evic_set);//eviction is needed
			}
			return MISS;
		}
		break;
	}
}

access_type_t cache::write(address_t address)
{
	// tag =  (address >> (block_offset_bits + index_bits)) & ((1 << tag_bits) - 1);

	tag = (address >> (block_offset_bits + index_bits));
	index = calculate_index(address);
	write_hit_flag = 0;
	empty = 0;

	switch (w_hit)
	{
	case WRITE_BACK:
		eviction_set = 0;int send_i;
		for (unsigned k = 0; k < c_asso; k++)
		{	send_i=k;
			write_hit_flag = hit_status1(index, tag, send_i);
			if (write_hit_flag == 1)
			{
				writeback[index][send_i].dirty_bit = 1;
				writeback[index][send_i].lru=0;
				update_lru1(send_i);
				break;//write hit
			}
		}
		if (write_hit_flag == 1)
			return HIT;
		else
		{       wr_miss++;
			for (unsigned k = 0; k < c_asso; k++)
			{       send_i=k;
				empty = empty_block1(index, send_i);
				if (empty == 1)
				{
					update_cache1(index, send_i, tag);
					writeback[index][send_i].lru = 0;
					writeback[index][send_i].dirty_bit = 1;
					update_lru1(send_i);
					break;
				}
			}
			if (empty == 0)
			{    
				evic_set = eviction1(index, eviction_set);
				update_cache1(index, evic_set, tag);
				writeback[index][evic_set].lru = 0;
				writeback[index][evic_set].dirty_bit = 1;
				update_lru1(evic_set);
			}
			return MISS;
		}
		break;

	case WRITE_THROUGH:
		for (unsigned j = 0; j < c_asso; j++)
		{
			write_hit_flag = hit_status2(index, tag, j);
			if (write_hit_flag == 1)
			{
				memory_access++;
				writethrough[index][j].lru=0;
				update_lru2(j);
				break;
			}
		}
		if (write_hit_flag == 1)
			return HIT;
		else
		{
			memory_access++;
			return MISS;
		}

		break;
	}
}

void cache::print_tag_array()
{
	cout << "TAG ARRAY" << endl;
	if (w_hit == WRITE_BACK)
	{
		for (unsigned i = 0; i < c_asso; i++)
		{
			cout << "BLOCKS" << " " << i << std::endl;
			cout << setfill(' ') << setw(7)<<"index" << setw(6) << "dirty"<< setw(4+tag_bits/4)<< "tag" << std::endl;
			for (unsigned j = 0; j < sets; j++)
			{
				if (writeback[j][i].valid != 0)
					cout << setfill(' ') <<setw(7)<< std::dec << j << setw(6) << writeback[j][i].dirty_bit <<setw(4+tag_bits/4)<< "0x" << std::hex << writeback[j][i].tag_1 << std::endl;
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < c_asso; i++)
		{
			cout << "BLOCKS"
				 << " " << i << std::endl;
			cout << " "
				 << "index"
				 << " "
				 << " "
				 << " "
				 << " "
				 << "tag" << std::endl;
			for (unsigned j = 0; j < sets; j++)
			{
				if (writethrough[j][i].valid != 0)
					cout << "t " << std::dec << j << " "
						 << " " << std::hex << writethrough[j][i].tag_2 << std::endl;
			}
		}
	}
}

/*unsigned cache::evict(unsigned index){
	/* edit here */
//	return 0;
//}*/
/*address_t cache::get_index(address_t a)
{
unsigned index= static_cast<uint16_t>((a >> 48) & 0xFFFF);
return(index);
}*/

