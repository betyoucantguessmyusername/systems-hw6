// Joe Meyer & Ezra Schwartz


#include <string.h>
#include "cache.hh"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>

//time
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


using namespace std;


const int NANOS_PER_SEC = 1000000000;


// this is a functor
class betterHasher {
private:
	hash<string> hasher_;
	uint32_t bound_;

public:
	// hashes key to uint32_t in range(0, bound)
	uint32_t operator()(string key) {
		return this->hasher_(key)%this->bound_;
	}
	betterHasher(uint32_t bound);
};

betterHasher::betterHasher(uint32_t bound) {
	this->bound_ = bound;
}





// HELPER FUNCTIONS
Cache* set_up_cache(uint32_t size = sizeof(uint32_t), uint32_t cache_length = 3) {
	uint32_t bound = 3;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	return myCache;
}

void fill_cache(Cache* myCache) {
	uint32_t size = sizeof(uint32_t);
	uint32_t val = 1;
	myCache->set("key1", &val, size);
	val = 2;
	myCache->set("key2", &val, size);
	val = 3;
	myCache->set("key3", &val, size);
}

double time_set(
	Cache* myCache,
	uint32_t trials,
	bool time_evict = false,
	bool dif_val_sizes = false,
	Cache::key_type key = "key"
	) {

	uint32_t valsize = sizeof(uint32_t);
	uint32_t val = 1;

	uint16_t small_size = sizeof(uint16_t);
	uint16_t small_val = 1;
	
	// prepare to time many trials
	double elapsed_ticks = 0;
	uint32_t trials_left = trials;

	// time many trials
	for(;trials_left>0;trials_left--) {
		clock_t start = clock();
		myCache->set(key, &val, valsize);
		clock_t end = clock();
		elapsed_ticks += (end-start);
		// start from empty cache each set
		if(!time_evict) {
			myCache->del(key);
		}

		if(dif_val_sizes and !trials_left%2) {
			clock_t start = clock();
			myCache->set(key, &small_val, small_size);
			clock_t end = clock();
			elapsed_ticks += (end-start);
			// start from empty cache each set
			if(!time_evict) {
				myCache->del(key);
			}
			trials_left--;
		}
	}

	double elapsed_nanosecs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double average_nanosecs = elapsed_nanosecs/(double)trials;
	return average_nanosecs;
}


double time_to_get(
	Cache* myCache,
	uint32_t trials,
	Cache::key_type key = "key",
	Cache::index_type val_size = sizeof(uint32_t)
	) {

	// prepare to time many trials
	double elapsed_ticks = 0;
	uint32_t trials_left = trials;

	// time many trials
	for(;trials_left>0;trials_left--) {
		clock_t start = clock();
		myCache->get(key, val_size);
		clock_t end = clock();
		elapsed_ticks += (end-start);
		// start from empty cache each set
	}

	double elapsed_nanosecs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double average_nanosecs = elapsed_nanosecs/(double)trials;
	return average_nanosecs;
}


double time_del(
	Cache* myCache,
	uint32_t trials,
	bool del_absent = false,
	bool time_evict = false,
	bool dif_val_sizes = false,
	Cache::key_type key = "key"
	) {

	uint32_t val_size = sizeof(uint32_t);
	uint32_t val = 1;

	// prepare to time many trials
	double elapsed_ticks = 0;
	uint32_t trials_left = trials;

	// time many trials
	for(;trials_left>0;trials_left--) {
		clock_t start = clock();
		myCache->del(key);
		clock_t end = clock();
		elapsed_ticks += (end-start);
		if(!del_absent) {
			// re-set value
			myCache->set(key, &val, val_size);
		}
	}

	double elapsed_nanosecs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double average_nanosecs = elapsed_nanosecs/(double)trials;
	return average_nanosecs;
}


double time_space_used(
	Cache* myCache,
	uint32_t trials
	) {

	// prepare to time many trials
	double elapsed_ticks = 0;
	uint32_t trials_left = trials;

	// time many trials
	for(;trials_left>0;trials_left--) {
		clock_t start = clock();
		myCache->space_used();
		clock_t end = clock();
		elapsed_ticks += (end-start);
		// start from empty cache each set
	}

	double elapsed_nanosecs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double average_nanosecs = elapsed_nanosecs/(double)trials;
	return average_nanosecs;
}







//TIMER FUNCS



// insert item into cache, query it, assert both value is unchanged
void test_set_insert(uint32_t trials = 100000) {
	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();

	double average_nanosecs = time_set(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

// fill cache, do insert and query it
void test_set_insert_full(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();
	fill_cache(myCache);
	double average_nanosecs = time_set(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);

}

// set element, overwrite it, query it
void test_set_overwrite(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();
	uint32_t val = 2;
	myCache->set("key", &val, size);
	double average_nanosecs = time_set(myCache, trials, true);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

// set element, overwrite it with different size, query it
void test_set_overwrite_dif_size(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	uint16_t small_size = sizeof(uint16_t);
	uint16_t small_value = 1;
	Cache* myCache = set_up_cache();
	myCache->set("key", &small_value, small_size);

	double average_nanosecs = time_set(myCache, trials, true, true);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}



void test_get_present(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	uint16_t value = 1;
	Cache* myCache = set_up_cache();
	myCache->set("key", &value, size);

	double average_nanosecs = time_to_get(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

//checks that using get on an absent item returns a nullptr
void test_get_absent(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();

	double average_nanosecs = time_to_get(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

//checks that using get on a deleted item returns a nullptr
void test_get_deleted(uint32_t trials = 100000) {
	Cache* myCache = set_up_cache();

	//insert/delete data
	Cache::key_type key = "key";
	Cache::index_type val_size = sizeof(uint32_t);
	uint32_t val = 42;
	myCache->set(key, &val, val_size);
	myCache->del(key);

	// try to get it, time
	double average_nanosecs = time_del(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}


//checks that we don't crash when we delete something absent
void test_delete_absent(uint32_t trials = 100000) {
	Cache* myCache = set_up_cache();

	// try to get it, time
	double average_nanosecs = time_del(myCache, trials, true);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

//checks that initial space used is 0.
void test_space_used_empty(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();

	double average_nanosecs = time_space_used(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}

// fill cache, check space used
void test_space_used_full(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();
	fill_cache(myCache);

	double average_nanosecs = time_space_used(myCache, trials);
	cout << "avg nanosecs per op: " << average_nanosecs << "\t\t";
	free(myCache);
}




int main(){

	cout << "Running test_set_insert() \t\t"; 
	test_set_insert();
	cout << "PASS" << endl;

	cout << "Running test_set_insert_full() \t\t"; 
	test_set_insert_full();
	cout << "PASS" << endl;

	cout << "Running test_set_overwrite() \t\t"; 
	test_set_overwrite();
	cout << "PASS" << endl;

	cout << "Running test_set_overwrite_dif_size() \t"; 
	test_set_overwrite_dif_size();
	cout << "PASS" << endl;


	cout << "Running test_get_present() \t\t";
	test_get_absent();
	cout << "PASS" << endl;

	cout << "Running test_get_absent() \t\t";
	test_get_absent();
	cout << "PASS" << endl;

	cout << "Running test_get_deleted() \t\t"; 
	test_get_deleted();
	cout << "PASS" << endl;

	cout << "Running test_delete_absent() \t\t"; 
	test_delete_absent();
	cout << "PASS" << endl;

	cout << "Running test_space_used_empty() \t"; 
	test_space_used_empty();
	cout << "PASS" << endl;

	cout << "Running test_space_used_full() \t\t"; 
	test_space_used_full();
	cout << "PASS" << endl;

}