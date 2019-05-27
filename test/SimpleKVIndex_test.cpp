#include "../SimpleKVIndex.cpp"
#include <string.h>
using namespace kvsvr;

int main()
{
  SimpleKVIndex<std::string, std::string, 3> kv("kingsley", 4);
  kv.set("ssss", "fddd");
  //kv.setup();
}