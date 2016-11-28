#ifndef _BTREE_
#define _BTREE_
#define m 3 /* m 路 B-tree*/
#define L 3 /* B+ tree 元素节点最大长度 */
#include <iostream>
#include <math.h>
#include <stack>
using namespace std;

class TreeNode {
  friend class Btree;
  friend class BPlusTree;
  friend class Result;

  public： TreeNode() { //用于区分是否是B+树的叶子节点，即数据节点
    keynum = 0;         //是叶子节点即数据节点
  }

  //构造新节点
  TreeNode(TreeNode *p1, int k, TreeNode *p2) {
    keynum = 1;
    key[1] = k;
    parent = 0;
    fill(ptr, ptr + m, (TreeNode *)0);
    ptr[0] = p1;
    ptr[1] = p2;
    /* 设置子树的父节点 */
    //如果p1存在
    if (p1) {
      p1->parent = this;
    }
    //如果p2存在
    if (p2) {
      p2->parent = this;
    }
  }

  //分裂得到新的节点
  TreeNode(TreeNode *p, int *keyArray, TreeNode **ptrArray) {
    parent = p;
    fill(ptr, ptr + m, (TreeNode *)0);
    int split_loc = ceil(m / 2.0); //向上取整
    ptr[0] = ptrArray[split_loc];

    //发生分裂，父节点需要改变
    if (ptr[0]) {
      ptr->parent = this;
    }

    //更新分裂得到的新节点的属性值
    for (int i = 1; i <= (m - split_loc); i++) {
      key[i] = keyArray[split_loc + i];
      ptr[i] = ptrArray[split_loc + i];
      if (ptr[i]) {
        ptr[i]->parent = this;
      }
      keyArray[split_loc + i] = 0;
      ptrArray[split_loc + i] = 0;
    }

    keyArray[split_loc] = 0;
    ptrArray[split_loc] = 0;
    keynum = m - split_loc;
  }

protected:
  int keynum;           /* 关键字个数， keynum < m-1 */
  TreeNode *parent;     /* 父节点指针 */
  TreeNode *ptr[m + 1]; /* 子树指针 0,1,2,……m */
  int key
      [m +
       1]; /* 1,2,……m
              多一个单元可以保证当溢出时可以先进行插入然后再考虑分裂避免出现overflow的问题
              */
};

class Result {
  friend class Btree;

public:
  Result(TreeNode *p = 0, int i = 0, bool r = 0) {
    ResultPtr = p;
    index = i;
    ResultFlag = r;
  }
  TreeNode *ResultPtr;
  int index;
  bool ResultFlag;
};

class Btree {
public:
  Btree() { root = 0; }
  Result find(int k);
  void InsertTree(int k);
  void deleteTree(int k);
  void Insert(int k, TreeNode *node, TreeNode *p);

protected:
  TreeNode *root;
  void BorrowOrCombine(TreeNode *p, int i, int type, stack<int> &s)
};

class DataNode : public TreeNode {
  friend class BPlusTree;

public:
  // create a new node
  DataNode(int k) {
    data[1] = k;
    pre = next = 0;
    parent = 0;
    datanum = 1;
  }
  // split a node
  DataNode(TreeNode *Parent, DataNode *Pre, DataNode *Next, int *dataArray) {
    parent = Parent;
    pre = Pre;
    next = Next;
    int split_loc = ceil(m / 2.0);
    for (int i = 1; i < L + 1 - split_loc; i++) {
      data[i] = dataArray[i + split_loc];
      dataArray[i + split_loc] = 0;
    }
    datanum = L + 1 - split_loc;
    Pre->datanum = split_loc;
  }

private:
  DataNode *pre;
  DataNode *next;
  int data[L + 2] ； int datanum;
};

class BPluSTree : public Btree {
public:
  BPluSTree() {}
  void InsertBPlusTree(int k);
  void InsertData(int k, DataNode *datanodecurrent);
  void DeleteBPlusTree(int k);

private:
  DataNode *header;
};

#endif
