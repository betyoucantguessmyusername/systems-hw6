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


const double NANOS_PER_SEC = 1000000000.;


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


	double elapsed_secs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double elapsed_nanosecs = elapsed_secs*NANOS_PER_SEC;
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

	double elapsed_secs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double elapsed_nanosecs = elapsed_secs*NANOS_PER_SEC;
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

	double elapsed_secs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double elapsed_nanosecs = elapsed_secs*NANOS_PER_SEC;
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

	double elapsed_secs = elapsed_ticks/(double)CLOCKS_PER_SEC;
	double elapsed_nanosecs = elapsed_secs*NANOS_PER_SEC;
	double average_nanosecs = elapsed_nanosecs/(double)trials;
	return average_nanosecs;
}



void read_avg(double avg) {
	cout << "avg nanosecs per op: " << avg << "\t\t";
}



//TIMER FUNCS



// insert item into cache, query it, assert both value is unchanged
double test_set_insert(uint32_t trials = 100000) {
	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();

	double average_nanosecs = time_set(myCache, trials);
	free(myCache);
	return average_nanosecs;
}

// fill cache, do insert and query it
double test_set_insert_full(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();
	fill_cache(myCache);
	double average_nanosecs = time_set(myCache, trials);
	free(myCache);
	return average_nanosecs;

}

// set element, overwrite it, query it
double test_set_overwrite(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	Cache* myCache = set_up_cache();
	uint32_t val = 2;
	myCache->set("key", &val, size);
	double average_nanosecs = time_set(myCache, trials, true);
	free(myCache);
	return average_nanosecs;
}

// set element, overwrite it with different size, query it
double test_set_overwrite_dif_size(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	uint16_t small_size = sizeof(uint16_t);
	uint16_t small_value = 1;
	Cache* myCache = set_up_cache();
	myCache->set("key", &small_value, small_size);

	double average_nanosecs = time_set(myCache, trials, true, true);
	free(myCache);
	return average_nanosecs;
}



double test_get_present(uint32_t trials = 100000) {

	uint32_t size = sizeof(uint32_t);
	uint16_t value = 1;
	Cache* myCache = set_up_cache();
	myCache->set("key", &value, size);

	double average_nanosecs = time_to_get(myCache, trials);
	free(myCache);
	return average_nanosecs;
}

//checks that using get on an absent item returns a nullptr
double test_get_absent(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();

	double average_nanosecs = time_to_get(myCache, trials);
	free(myCache);
	return average_nanosecs;
}

//checks that using get on a deleted item returns a nullptr
double test_get_deleted(uint32_t trials = 100000) {
	Cache* myCache = set_up_cache();

	//insert/delete data
	Cache::key_type key = "key";
	Cache::index_type val_size = sizeof(uint32_t);
	uint32_t val = 42;
	myCache->set(key, &val, val_size);
	myCache->del(key);

	// try to get it, time
	double average_nanosecs = time_del(myCache, trials);
	free(myCache);
	return average_nanosecs;
}


double test_delete(uint32_t trials = 100000) {
	Cache* myCache = set_up_cache();
	double average_nanosecs = time_del(myCache, trials);
	free(myCache);
	return average_nanosecs;
}

//checks that we don't crash when we delete something absent
double test_delete_absent(uint32_t trials = 100000) {
	Cache* myCache = set_up_cache();

	// try to get it, time
	double average_nanosecs = time_del(myCache, trials, true);
	free(myCache);
	return average_nanosecs;
}

//checks that initial space used is 0.
double test_space_used_empty(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();

	double average_nanosecs = time_space_used(myCache, trials);
	free(myCache);
	return average_nanosecs;
}

// fill cache, check space used
double test_space_used_full(uint32_t trials = 100000) {

	Cache* myCache = set_up_cache();
	fill_cache(myCache);

	double average_nanosecs = time_space_used(myCache, trials);
	free(myCache);
	return average_nanosecs;
}



uint32_t superscript() {
	Cache* myCache = set_up_cache();
	int operation = rand()%10;
	bool set = operation>6;
	int hit = rand()%10;

	double total_nanosecs = 0;
	uint32_t total_operations;


	const uint32_t A_HUNDRED_MILLISECS = 100000;
	// 100 millisecs = 100000 nanosecs

	while(total_nanosecs<A_HUNDRED_MILLISECS) {
		if (set) {
			total_nanosecs += test_set_insert();
		} else {
			if (hit<9) {
				total_nanosecs += test_get_present();
			} else {
				total_nanosecs += test_get_absent();
			}
		}
		total_operations++;
	}
	return total_operations;

}




int main(){

	cout << "Running test_set_insert() \t\t"; 
	read_avg(test_set_insert());
	cout << "PASS" << endl;

	cout << "Running test_set_insert_full() \t\t"; 
	read_avg(test_set_insert_full());
	cout << "PASS" << endl;

	cout << "Running test_set_overwrite() \t\t"; 
	read_avg(test_set_overwrite());
	cout << "PASS" << endl;

	cout << "Running test_set_overwrite_dif_size() \t"; 
	read_avg(test_set_overwrite_dif_size());
	cout << "PASS" << endl;


	cout << "Running test_get_present() \t\t";
	read_avg(test_get_absent());
	cout << "PASS" << endl;

	cout << "Running test_get_absent() \t\t";
	read_avg(test_get_absent());
	cout << "PASS" << endl;

	cout << "Running test_get_deleted() \t\t"; 
	read_avg(test_get_deleted());
	cout << "PASS" << endl;

	cout << "Running test_delete() \t\t\t"; 
	read_avg(test_delete());
	cout << "PASS" << endl;

	cout << "Running test_delete_absent() \t\t"; 
	read_avg(test_delete_absent());
	cout << "PASS" << endl;

	cout << "Running test_space_used_empty() \t"; 
	read_avg(test_space_used_empty());
	cout << "PASS" << endl;

	cout << "Running test_space_used_full() \t\t"; 
	read_avg(test_space_used_full());
	cout << "PASS" << endl;

	cout << "Running superscript \t\t\t";
	cout << "operations per 100 millisecs: " <<superscript() << endl;

}
