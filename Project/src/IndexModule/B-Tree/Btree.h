#ifndef _BTREE_
#define _BTREE_
#define m 3 /* m · B-tree*/
#define L 3 /* B+ tree Ԫ�ؽڵ���󳤶� */
#include <iostream>
#include <math.h>
#include <stack>
using namespace std;

class TreeNode {
  friend class Btree;
  friend class BPlusTree;
  friend class Result;

  public�� TreeNode() { //���������Ƿ���B+����Ҷ�ӽڵ㣬�����ݽڵ�
    keynum = 0;         //��Ҷ�ӽڵ㼴���ݽڵ�
  }

  //�����½ڵ�
  TreeNode(TreeNode *p1, int k, TreeNode *p2) {
    keynum = 1;
    key[1] = k;
    parent = 0;
    fill(ptr, ptr + m, (TreeNode *)0);
    ptr[0] = p1;
    ptr[1] = p2;
    /* ���������ĸ��ڵ� */
    //���p1����
    if (p1) {
      p1->parent = this;
    }
    //���p2����
    if (p2) {
      p2->parent = this;
    }
  }

  //���ѵõ��µĽڵ�
  TreeNode(TreeNode *p, int *keyArray, TreeNode **ptrArray) {
    parent = p;
    fill(ptr, ptr + m, (TreeNode *)0);
    int split_loc = ceil(m / 2.0); //����ȡ��
    ptr[0] = ptrArray[split_loc];

    //�������ѣ����ڵ���Ҫ�ı�
    if (ptr[0]) {
      ptr->parent = this;
    }

    //���·��ѵõ����½ڵ������ֵ
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
  int keynum;           /* �ؼ��ָ����� keynum < m-1 */
  TreeNode *parent;     /* ���ڵ�ָ�� */
  TreeNode *ptr[m + 1]; /* ����ָ�� 0,1,2,����m */
  int key
      [m +
       1]; /* 1,2,����m
              ��һ����Ԫ���Ա�֤�����ʱ�����Ƚ��в���Ȼ���ٿ��Ƿ��ѱ������overflow������
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
  int data[L + 2] �� int datanum;
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
