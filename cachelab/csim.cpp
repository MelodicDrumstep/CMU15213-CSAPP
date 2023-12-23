#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

//CacheLine is a structure storing the valid bit, the tag, 
//and a variable named time_spent_stamp to store the information about time
typedef struct
{
	int valid;
	size_t tag;
} CacheLine;

int s, E, b;

//Cache is a structure storing the number of sets,
//how many lines per set, how many bytes per block,
//and store cachelines as array of array
typedef struct
{
	int S, E, B;
	std::vector<std::vector<CacheLine>> line;
} Cache;



//variables to record the number of hit / miss / evict 
int hit_cnt = 0;
int miss_cnt = 0;
int eviction_cnt = 0;

int time_spent = 1;

int verbose = 0;

char trace_file_name[1000];
Cache cache;

void HitOrMiss(size_t tag, size_t set_index, std::string & result)
{
//result += ...
	for(int i = 0; i < E; i++)
	{
		if(cache.line[set_index][i].tag == tag)
		{
			result += "hit ";
			hit_cnt++;
			cache.line[set_index][i].valid = time_spent;
			return;
		}
	}
	result += "miss ";
	miss_cnt++;
	for(int i = 0; i < E; i++)
	{
		if(cache.line[set_index][i].valid == 0)
		{
			cache.line[set_index][i].tag = tag;
			cache.line[set_index][i].valid = time_spent;
			return;
		}
	}
	int min_num = INT32_MAX;
	int evit_num = -1;
	for(int i = 0; i < E; i++)
	{
		if(cache.line[set_index][i].valid < min_num)
		{
			min_num = cache.line[set_index][i].valid;
			evit_num = i;
		}
	}
	result += "eviction ";
	eviction_cnt++;
	cache.line[set_index][evit_num].valid = time_spent;
	cache.line[set_index][evit_num].tag = tag;
}

void ActCache()
{
	std::fstream fs(trace_file_name);
	if(!fs.is_open())
	{
		exit(0);
	}
	std::string line;
	while(std::getline(fs, line))
	{
		char operation, comma;
		size_t address, size;
		std::stringstream ss;
		ss << line;
		ss >> operation;
		if(operation == 'I')
		{
			continue;
		}
		ss >> std::hex >> address;
		ss >> comma >> size;
		size_t tag = address >> (s + b);
		size_t set_index = (address >> b) & ~(~0u << s);
		// std::cout << "operation: " << operation << " address: " << address 
		// << comma << "   " << "size : " << size
		// << " tag: " << tag << " set_index: " << set_index << std::endl;

		std::string result;
		HitOrMiss(tag, set_index, result);
		if(operation == 'M')
		{
			HitOrMiss(tag, set_index, result);
		}
		if(verbose)
		{
			std::cout << operation << " " << std::hex << address;
			std::cout << "," << size << " ";
		}
		std::cout << result << std::endl; 
		time_spent++;
	}
	fs.close();
}

void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

void InitCache() {
	//set S and B to 2^s and 2^b
	int S = 1 << s;
	int B = 1 << b;
	cache.S = S;
	cache.E = E;
	cache.B = B;
	cache.line.resize(S);
	for (int i = 0; i < S; i++) 
	{
		cache.line[i].resize(E);
		for (int j = 0; j < E; j++) 
		{
			cache.line[i][j].valid = 0;
			cache.line[i][j].tag = -1;
		}
	}
}

void PrintHelp() {
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("-h         Print this help message.\n");
	printf("-v         Optional verbose flag.\n");
	printf("-s <num>   Number of set index bits.\n");
	printf("-E <num>   Number of lines per set.\n");
	printf("-b <num>   Number of block offset bits.\n");
	printf("-t <file>  Trace file.\n\n\n");
	printf("Examples:\n");
	printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc, char *argv[]) {
	char op;
	while (EOF != (op = getopt(argc, argv, "hvs:E:b:t:"))) {
		switch (op) {
			case 'h':
				PrintHelp();
				exit(0);
			case 'v':
				verbose = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strcpy(trace_file_name, optarg);
				break;
			default:
				PrintHelp();
				exit(-1);
		}
	}
	InitCache();
	
	ActCache();

	printSummary(hit_cnt, miss_cnt, eviction_cnt);
	return 0;
}