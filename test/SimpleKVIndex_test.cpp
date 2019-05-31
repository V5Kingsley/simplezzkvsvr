#include "../SimpleKVIndex.cpp"
#include <string.h>
using namespace kvsvr;

int main()
{
  SimpleKVIndex<std::string, double, 3> kv("kingsley", 4);
  kv.set("kingsley", 123.88);
  double value;
  kv.get("kingsley", value);
  std::cout<<value<<std::endl;

  kv.set("lincoln", 234.99);
  kv.get("lincoln", value);
  kv.erase("lincoln");
  kv.set("king", 12);
  if(kv.get("lincoln", value) == false) 
    std::cout<<"erase succeeded"<<std::endl;
  

}