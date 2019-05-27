#include "../BTree_index.h"

using namespace kvsvr;

int main()
{
  BTreeIndex<int, 12> tree;
  /*tree.insert(33, 1);
  tree.insert(22, 1);
  tree.insert(27, 1);
  tree.insert(30, 1);
  tree.insert(36, 1);
  tree.insert(41, 1);
  tree.insert(23, 1);
  tree.insert(45, 1);
  tree.insert(55, 1);
  tree.insert(42, 1);
  tree.insert(76, 1);
  tree.insert(65, 1);
  tree.show();
  tree.insert(56, 1);
  tree.show();
  tree.insert(34, 1);
  tree.show();
  tree.insert(10, 1);
  tree.show();

  tree.erase(34);
  tree.show();
  tree.erase(27);
  tree.show();
  tree.erase(42);
  tree.show();
  tree.erase(45);
  tree.show();
  tree.erase(41);
  tree.show();
  tree.erase(23);
  tree.show();
  tree.erase(36);
  tree.show();
  tree.erase(22);
  tree.show();
  tree.erase(30);
  tree.show();
  tree.erase(56);
  tree.show();
  tree.erase(10);
  tree.show();
  tree.erase(23);
  tree.show();
  tree.erase(33);
  tree.show();
  tree.erase(55);
  tree.show();
  tree.erase(65);
  tree.show();
  tree.erase(76);
  tree.show();*/

  int tree_size = 10000;

  for(int i = 0; i < tree_size; ++i)
  {
    srand(i);
    int num = rand() % tree_size;
    tree.insert(num, 1);
  }
  //tree.show();

  for(int i = tree_size; i < tree_size * 2; ++i)
  {
    srand(i);
    int num = rand() % tree_size;
    //printf("erase %d ", num);
    tree.erase(num);
  }

  tree.show();
  

}