#include "cache.h"

int main()
{

  size_t  cache_size  = 0;
  int     el_quantity = 0;
  int     hits        = 0;
  
  cin >> cache_size;
  cin >> el_quantity;
  assert(cin.good());

  caches_lfu::CacheLFU <int> lfu{cache_size};
  hits = lfu.finding_hits(el_quantity);
  cout << hits << endl;

  return 0;
}