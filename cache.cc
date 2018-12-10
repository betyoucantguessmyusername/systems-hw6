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

// this is a functor (implements Largest-out-first, to maximize number of values stored)
// if all val.s are same size it acts as LRU
class EvictorType {
private:
	vector<node_type> eviction_queue_;
	// eviction_queue_ holds nodes of form (val-size, key)
public:
	// returns next key to evict, also removes it from ev. q
	node_type operator()() {
		node_type next_evict;
		// EvictorType() is never called on an empty eviction_queue
		assert(eviction_queue_.size()>0 && "nothing to evict\n");
		next_evict = eviction_queue_[0];
		string next_evict_key = get<1>(next_evict);
		remove(next_evict_key);
		return next_evict;
	}

	// add an element to eviction queue
	// places it directly after the smallest el. w size >= to it, before smaller el.s
	void add(uint32_t elt_size, string key) {
		uint32_t evq_size = eviction_queue_.size();
		node_type node = make_tuple(elt_size, key);
		uint32_t i = 0;
		for(;i<evq_size; i++) {
			uint32_t i_size = get<0>(eviction_queue_[i]);
			if(elt_size>i_size) {
				eviction_queue_.insert(eviction_queue_.begin()+i, node);
			}
		}
		if(i>=evq_size) {
			eviction_queue_.push_back(node);
		}
	}

	// remove an item from ev. q.
	void remove(string key) {
		// erase-remove_if idiom
		eviction_queue_.erase(std::remove_if(eviction_queue_.begin(), eviction_queue_.end(), [key](node_type node){return get<1>(node)==key;}), eviction_queue_.end());
	}

	// get size of key's val
	uint32_t getsize(string key) {
		uint32_t i = 0;
		for(;i<eviction_queue_.size(); i++) {
			node_type node = eviction_queue_[i];
			if(get<1>(node) == key) {
				return get<0>(node);
			}
		}
		assert(false && "key not found\n");
		return 0;
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
			Evictor_.remove(key);
		}
		memused_ += size;
		while(memused_ >= maxmem_) {
			// get next_evict (also del.s it from ev. q.)
			node_type next_evict = Evictor_();
			string next_evict_key = get_tuple_key(next_evict);
			uint32_t next_evict_size = get_tuple_size(next_evict);
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
			Evictor_.remove(key);
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
			Evictor_.remove(key);
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
