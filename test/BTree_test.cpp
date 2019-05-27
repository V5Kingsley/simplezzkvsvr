#include "../BTree_index.h"
#include <stdio.h>
#include <string.h>
using namespace kvsvr;
static const int BUFFER_LENGTH = 1024;

int main()
{
  FILE *file = fopen("example/aof.txt", "r");
  BTreeIndex<std::string, 3> tree;

  if (file == nullptr)
  {
    printf("open error!\n");
    return 0;
  }

  long cur = ftell(file);

  char strLine[BUFFER_LENGTH];
  while (!feof(file))
  {
    int offset = ftell(file);
    fgets(strLine, BUFFER_LENGTH, file);
    int i;
    for (i = 0; i < strlen(strLine); ++i)
    {
      if (strLine[i] == '<')
      {
        ++i;
        char key[BUFFER_LENGTH];
        int index = 0;
        while (strLine[i] != '>')
        {
          key[index] = strLine[i++];
          ++index;
        }
        key[index] = '\0';
        std::cout << "got a key: " << key << ", ";
        while (strLine[i] != '<')
          ++i;
        offset += ++i;
        char value[BUFFER_LENGTH];
        index = 0;
        while (strLine[i] != '>')
        {
          value[index] = strLine[i++];
          ++index;
        }
        value[index] = '\0';
        std::cout << "value: " << value << ", offset: " << offset << std::endl;
        if(tree.isInBTree(key))
          tree.setKeyOffset(key, offset);
        else
          tree.insert(key, offset);
      }
    }
  }

  long offset;
  while (true)
  {
    std::cout<<"input key: ";
    std::string key;
    std::cin >> key;
    if (tree.findKeyOffset(key, offset))
    {
      fseek(file, offset, SEEK_SET);
      char value[BUFFER_LENGTH];
      int i = 0;
      char ch;
      while (true)
      {
        ch = fgetc(file);
        if (ch == '>')
          break;
        else
          value[i++] = ch;
      }
      value[i] = '\0';
      std::cout << "key: " << key << ", value: " << value << std::endl;
    }
    else
    {
      std::cout<<"not in the BTree"<<std::endl;
    }
    
  }
}