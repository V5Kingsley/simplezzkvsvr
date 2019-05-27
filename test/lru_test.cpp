#include "../LRU.h"
#include <string.h>
#include <iostream>
using namespace kvsvr;

int main()
{
  LRUCache<std::string, long> lru(4);

  lru.set("hello", 11);
  lru.set("a", 1111);
  lru.set("aa", 21);
  lru.set("aaa", 22);
  lru.set("b", 22);
  lru.set("bba", 22);

  long value;
  if(lru.get("hello", value))
  {
    printf("value: %ld\n", value);
  }
  else
  {
    printf("not in lru\n");
  }

  if(lru.get("aa", value))
  {
    printf("value: %ld\n", value);
  }
  else
  {
    printf("not in lru\n");
  }
  

}