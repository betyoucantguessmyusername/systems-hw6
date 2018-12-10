// Joe Meyer & Ezra Schwartz

#include <stdio.h>
#include <string.h>
#include "cache.hh"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>

using namespace std;


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


// prints/returns value at key
uint32_t get_interface(Cache* c, Cache::key_type key, Cache::index_type& val_size) {
	Cache::val_type value = c->get(key, val_size);
	uint32_t* data_at_val = new uint32_t[1];
	// data is hard copy (uint32_t) of uint32_t* data_at_val
	int32_t data = -1;
	if(value!=nullptr) {
		memcpy(data_at_val, value, val_size);
		data = *data_at_val;
	}
	free(data_at_val);
	return data;
}

// ensure hash-values are consistent
void test_hasher() {
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);

	uint32_t keyhash = myHasher("key");
	uint32_t newkeyhash = myHasher("key");
	assert(newkeyhash==keyhash);
}



// insert item into cache, query it, assert both value is unchanged
void test_set_insert() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint32_t first_val = 1;
	Cache::key_type key = "key";

	// insert item
	myCache->set(key, &first_val, size);

	// query item and assert value is correct
	assert(get_interface(myCache, "key", size) == 1 && "'key' value is wrong");
	free(myCache);
}

// fill cache, do insert and query it
void test_set_insert_full() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint32_t val = 1;
	myCache->set("key1", &val, size);
	val = 2;
	myCache->set("key2", &val, size);
	val = 3;
	myCache->set("key3", &val, size);
	assert(get_interface(myCache, "key3", size) == 3 && "set fails on full cache");
	free(myCache);
}

// set element, overwrite it, query it
void test_set_overwrite() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint32_t val = 1;
	myCache->set("key", &val, size);
	val = 2;
	myCache->set("key", &val, size);
	assert(get_interface(myCache, "key", size) == 2 && "overwrite fails");
	free(myCache);
}

// set element, overwrite it with different size, query it
void test_set_overwrite_dif_size() {
	uint32_t cache_length = 2;
	uint16_t small_size = sizeof(uint16_t);
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint16_t small_value = 1;
	myCache->set("key", &small_value, small_size);
	uint32_t val = 2;
	myCache->set("key", &val, size);
	assert(get_interface(myCache, "key", size) == 2 && "overwriting w diff size fails");

	free(myCache);
}

// test that evictor uses first in first out policy
void test_evictor_fifo() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint32_t val = 1;
	myCache->set("key1", &val, size);
	val = 2;
	myCache->set("key2", &val, size);
	val = 3;
	myCache->set("key3", &val, size);
	assert(get_interface(myCache, "key1", size) == (uint32_t)-1 && "FIFO-eviction failed");
	free(myCache);
}

// tests that overwrite updates eviction queue position 
void test_evictor_lru() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);
	uint32_t val = 1;
	myCache->set("key1", &val, size);
	val = 2;
	myCache->set("key2", &val, size);
	val = 3;
	myCache->get("key1", size);
	val = 4;
	myCache->set("key3", &val, size);
	assert(get_interface(myCache, "key2", size) == (uint32_t)-1 && "LRU-eviction failed");
	free(myCache);
}



//checks that using get on an absent item returns a nullptr
void test_get_absent() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	Cache::key_type key = "key";
	Cache::index_type val_size = sizeof(uint32_t);
	int32_t get_data = get_interface(myCache, key, val_size);
	assert(get_data == -1 && "Getting an absent variable should return nullptr");
	free(myCache);
}

//checks that using get on a deleted item returns a nullptr
void test_get_deleted() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	uint32_t val = 42;
	Cache::key_type key = "key";
	Cache::index_type val_size = sizeof(uint32_t);
	myCache->set(key, &val, val_size);
	myCache->del(key);
	int32_t get_data = get_interface(myCache, key, val_size);
	assert(get_data == -1 && "Getting a deleted variable should return nullptr");
	free(myCache);
}


//checks that we don't crash when we delete something absent
void test_delete_absent() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	Cache::key_type key = "key";
	uint32_t val_size = sizeof(uint32_t);

	Cache::index_type space_used = myCache->space_used();
	myCache->del(key);
	assert(space_used == myCache->space_used() && "Deleting an absent key doesn't impact the Cache.");
	free(myCache);
}

//checks that initial space used is 0.
void test_space_used() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	Cache::index_type space_used = myCache->space_used();
	assert(space_used == 0 && "Cache should initially be empty");
	free(myCache);
}

// insert element, check space used
void test_space_used_insert() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	uint32_t val = 42;
	Cache::key_type key = "key";
	uint32_t val_size = sizeof(uint32_t);
	myCache->set(key, &val, val_size);
	Cache::index_type space_used = myCache->space_used();
	assert(space_used == val_size && "Space used should be the size.");
	free(myCache);
}

// insert element, delete it, check space used
void test_space_used_delete() {
	uint32_t cache_length = 2;
	uint32_t size = sizeof(uint32_t);
	uint32_t bound = 2;
	betterHasher myHasher = betterHasher(bound);
	Cache* myCache = new Cache(cache_length*size, myHasher);

	uint32_t val = 42;
	Cache::key_type key = "key";
	uint32_t val_size = sizeof(uint32_t);
	myCache->set(key, &val, val_size);
	myCache->del(key);
	Cache::index_type space_used = myCache->space_used();
	assert(space_used == 0 && "Space used should empty now that we've deleted the only element.");
	free(myCache);
}


int main(){
	cout << "Running test_hasher() \t\t\t"; 
	test_hasher();
	cout << "PASS" << endl;

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

	cout << "Running test_evictor_fifo() \t\t";
	test_evictor_fifo();
	cout << "PASS" << endl;

	cout << "Running test_evictor_lru() \t\t";
	test_evictor_lru();
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

	cout << "Running test_space_used() \t\t"; 
	test_space_used();
	cout << "PASS" << endl;

	cout << "Running test_space_used_insert() \t"; 
	test_space_used_insert();
	cout << "PASS" << endl;

	cout << "Running test_space_used_delete() \t"; 
	test_space_used_delete();
	cout << "PASS" << endl;
}