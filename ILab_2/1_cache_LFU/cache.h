#pragma once

#include <iostream>
#include <unordered_map>
#include <list>
#include <iterator>
#include <cassert>

using std::list;
using std::unordered_map;
using std::cin;
using std::cout;
using std::endl;

namespace caches_lfu {

  template <typename T, typename KeyT = int> 
  class CacheLFU 
  {
    private:
      size_t  size_;
      list<T> cache_;
      int     hits_;

    public:
      using ListIt = typename list<T>::iterator;
      unordered_map <KeyT, ListIt> hash_;

      CacheLFU(size_t size) : size_(size), hits_(0) {}

      bool is_full() const
      { return (cache_.size() == size_); }

      bool lookup_update(KeyT key) 
      {
        return true;
      }

      int finding_hits(int el_quantity)
      {
        for (int current_number = 0; current_number < el_quantity; ++i) 
        {
          int new_key;
          cin >> new_key;
          assert(cin.good());

          if (lookup_update(new_key))
            hits_ += 1;
        }

        return hits_;
      }

};

}
