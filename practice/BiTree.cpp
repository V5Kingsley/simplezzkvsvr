#include <stdio.h>
#include <utility>
#include <iostream>
#include <queue>
using namespace std;

template <class k, size_t M>
struct BTreeNode
{

  BTreeNode()
      : _size(0), _pParent(NULL)
  {
    for (size_t idx = 0; idx < M + 1; ++idx)
      _pSub[idx] = NULL;
  }

  k _keys[M];              //存储键值的数组
  BTreeNode *_pSub[M + 1]; //存储孩子指针域的指针
  BTreeNode *_pParent;
  size_t _size; //有效键值的个数
};

template <class k, size_t M>
class BTree
{
  typedef BTreeNode<k, M> Node;

public:
  BTree()
      : _pRoot(NULL)
  {
  }

  bool Insert(const k &key)
  {
    if (_pRoot == NULL)
    {
      _pRoot = new Node;
      _pRoot->_keys[0] = key;
      _pRoot->_size = 1;
      return true;
    }

    //查找插入节点的位置
    pair<Node *, int> ret = Find(key);
    if (ret.second > -1)
      return false;
    Node *pNode = ret.first;
    Node *pSub = NULL;
    k valuek = key;

    //将key值插入我们已经找到的节点
    while (true)
    {
      _Insert(pNode, valuek, pSub);

      if (pNode->_size < M)
        return true;
      //需要对节点进行分裂
      Node *pNewNode = new Node;
      size_t mid = M / 2;
      size_t index = 0;
      size_t idx = 0;

      //搬移元素和孩子指针到pNewNode
      for (idx = mid + 1; idx < pNode->_size; ++idx)
      {
        pNewNode->_keys[index] = pNode->_keys[idx];
        pNewNode->_pSub[index] = pNode->_pSub[idx];
        if (pNode->_pSub[idx])
        {
          pNode->_pSub[idx]->_pParent = pNewNode->_pSub[index];
          pNode->_pSub[idx] = NULL;
        }
        pNewNode->_size++;
        pNode->_size--;
        index++;
      }

      pNewNode->_pSub[index] = pNode->_pSub[idx];
      if (pNode->_pSub[idx])
      {
        pNode->_pSub[idx]->_pParent = pNewNode->_pSub[index];
        pNode->_pSub[idx] = NULL;
      }
      //pNode->_size = pNode->_size - pNewNode->_size;
      pNode->_size--;

      //判断该节点是否为根节点
      if (pNode->_pParent == NULL)
      {
        _pRoot = new Node;
        _pRoot->_keys[0] = pNode->_keys[mid];

        _pRoot->_pSub[0] = pNode;
        pNode->_pParent = _pRoot;
        _pRoot->_pSub[1] = pNewNode;
        pNewNode->_pParent = _pRoot;
        _pRoot->_size++;
        return true;
      }
      else
      {
        valuek = pNode->_keys[mid];
        pNode = pNode->_pParent;
        pSub = pNewNode;
      }
    }
  }

  pair<Node *, int> Find(const k &key)
  {
    Node *pCur = _pRoot;
    Node *pParent = NULL;
    while (pCur)
    {
      int index = 0;
      while (index < pCur->_size)
      {
        if (pCur->_keys[index] > key)
        {
          //pCur = pCur->_pSub[index];
          break;
        }
        else if (pCur->_keys[index] < key)
          index++;
        else
          return pair<Node *, int>(pCur, index);
      }

      //if(key > pCur->_keys[index-1])
      pParent = pCur;
      pCur = pCur->_pSub[index]; //当key比数组最后一个key值还大时
    }
    return pair<Node *, int>(pParent, -1);
  }

  void show_BTree()
  {
    if (_pRoot == nullptr)
    {
      printf("Empty tree!\n");
      return;
    }

    queue<Node *> q;
    q.push(_pRoot);

    while (!q.empty())
    {
      Node *top_node = q.front();
      int i;
      for (i = 0; i < top_node->_size; ++i)
      {
        cout << top_node->_keys[i] << " ";
        if (top_node->_pSub[i])
          q.push(top_node->_pSub[i]);
      }
      if (top_node->_pSub[i])
        q.push(top_node->_pSub[i]);
      q.pop();
    }
  }

private:
  void _Insert(Node *&pNode, const k &key, Node *pSub)
  {
    int end = pNode->_size - 1;
    while (end > -1)
    {
      if (pNode->_keys[end] > key)
      {
        pNode->_keys[end + 1] = pNode->_keys[end];
        pNode->_pSub[end + 2] = pNode->_pSub[end + 1];
      }
      else
        break;
      end--;
    }
    pNode->_keys[end + 1] = key;
    pNode->_pSub[end + 2] = pSub;
    if (pSub)
      pSub->_pParent = pNode;
    pNode->_size++;
  }

private:
  Node *_pRoot;
};

void funtest()
{
  BTree<int, 3> b;
  b.Insert(20);
  b.Insert(10);
  b.show_BTree();
}

int main()
{
  funtest();
  getchar();
  return 0;
}