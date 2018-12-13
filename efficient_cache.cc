// by Joe Meyer

#include <stdio.h>
#include <string.h>
#include "cache.hh"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>

using namespace std;
using node_type = tuple<uint32_t,string>;

// this is a functor (implements LRU
class EvictorType {
private:
	std::unordered_map<std::string, uint32_t> keysizes_;
	// keysizes tracks value sizes of all keys in eviction queue
	vector<string> eviction_queue_;

public:
	// returns next key to evict, also removes it from ev. q
	// copies of it may still be in ev. q
	string operator()() {;
		string next_evict = eviction_queue_[0];
		remove_first();
		return next_evict;
	}

	// add an element to eviction queue
	// places it at end of queue
	void add(uint32_t elt_size, string key) {
		eviction_queue_.push_back(key);
		keysizes_[key] = elt_size;
	}

	// remove first item from ev. q.
	void remove_first() {
		eviction_queue_.erase(eviction_queue_.begin());
	}

	// get size of key's val
	uint32_t getsize(string key) {
		return keysizes_[key];
	}
};



// These funcs are necessary bc mandatory interface overwrites
//		``get" func required for tuple element access in Cache class
string get_tuple_key(node_type node) {
	return get<1>(node);
}

uint32_t get_tuple_size(node_type node) {
	return get<0>(node);
}


struct Cache::Impl {



	index_type maxmem_;
	hash_func hasher_;
	index_type memused_;
	mutable EvictorType Evictor_;
	
	std::unordered_map<std::string, void*, hash_func> hashtable_;


	Impl(index_type maxmem, hash_func hasher)
	: 
	maxmem_(maxmem), Evictor_(EvictorType()), hasher_(hasher), memused_(0), hashtable_(0 , hasher_)

	{
		assert(maxmem_>0 && "Cache size must be positive");
		hashtable_.max_load_factor(0.5);
    }


    ~Impl() = default;

	int set(key_type key, val_type val, index_type size) {
		if(size>maxmem_) {
			// don't bother
			return -1;
		}
		// if the key is already in the table...
		if(hashtable_.find(key)!=hashtable_.end()) {
			// remove it from queue (will overwrite it in cache/re-add it to queue later)
			free(hashtable_[key]);
			memused_ -= Evictor_.getsize(key);
		}
		memused_ += size;
		while(memused_ >= maxmem_) {
			// get next_evict (also del.s it from ev. q., copies may remain in q)
			string next_evict_key;
			do {
				next_evict_key = Evictor_();
			} while(hashtable_.find(next_evict_key)==hashtable_.end() );
			uint32_t next_evict_size = Evictor_.getsize(next_evict_key);
			memused_ -= next_evict_size;
			hashtable_.erase(next_evict_key);
		}
		void* newval = new char[size];
		memcpy(newval, val, size);
		hashtable_[key] = newval;
		Evictor_.add(size, key);
		return 0;
	}

	// returns cache[key]
	val_type get(key_type key, index_type& val_size) const {
		if(hashtable_.find(key)!=hashtable_.end()) {
			Evictor_.add(val_size, key);
			return hashtable_.find(key)->second;
		} else {
			return nullptr;
		}
	}

	// removes key:val from cache
	int del(key_type key) {
		if(hashtable_.find(key)!=hashtable_.end()) {
			free(hashtable_[key]);
			hashtable_.erase(key);
			memused_ -= Evictor_.getsize(key);
			return 0;
		}
		return -1;
	}

	// returns num of bytes used by cached values
	index_type space_used() const {
		return memused_;
	}
};

Cache::Cache(index_type maxmem,
    hash_func hasher)
	: pImpl_(new Impl(maxmem, hasher)) {
}

Cache::~Cache() {

}


// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, sufficient values will be removed
// from the cache to accomodate the new value.
int Cache::set(key_type key, val_type val, index_type size) {
	return pImpl_->set(key, val, size);
}

// Retrieve a pointer to the value associated with key in the cache,
// or NULL if not found.
Cache::val_type Cache::get(key_type key, index_type& val_size) const {
	return pImpl_->get(key, val_size);
}

// Delete an object from the cache, if it's still there
int Cache::del(key_type key) {
	return pImpl_->del(key);
}

// Compute the total amount of memory used up by all cache values (not keys)
Cache::index_type Cache::space_used() const {
	return pImpl_->space_used();
}
