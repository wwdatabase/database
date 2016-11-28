#include "Btree.h"
#include <iostream>
#include <math.h>
#include <stack>
using namespace std;
void Btree::InsertBtree(int k) {
  if (!root) {
    root = new TreeNode(0, k, 0);
    return;
  }
  TreeNode *a = root; /*当前节点*/
  int i = 1;          /*k关节字 要插入节点a 的位置索引*/
  /*找到插入节点*/
  while (a) {
    i = 1;
    /*在a 中找到第一个比关键字k大的关键字的位置 i */
    while (i <= a->keynum) {
      if (k <= a->key[i])
        break;
      else
        i++;
    }
    /* 判断是否继续向下 还是已经到达子节点 */
    if (!a->ptr[i - 1]) /* 已是叶子节点无需向下 直接插入 */
      break;
    else /*不是叶子节点*/
      a = a->ptr[i - 1];
  }
  if (a->key[i] == k) /*该关键字节点已存在 */
    return;
  Insert(k, 0, a); /*在叶子节点中插入关键字k*/
}
void Btree::Insert(int k, TreeNode *node,
                   TreeNode *a) /*关键字：k  该关键字k的右子树指针  插入节点a */
{
  int i = 1;
  /*在a 中找到第一个比关键字k大的关键字的位置 i */
  while (i <= a->keynum) {
    if (k <= a->key[i])
      break;
    else
      i++;
  }
  /*插入节点为 a  索引为 i */
  for (int j = a->keynum; j >= i; j--) /*向后移动以便插入新关键字*/
  {
    a->key[j + 1] = a->key[j]; /* 关键字*/
    a->ptr[j + 1] = a->ptr[j]; /*子树指针*/
  }
  a->key[i] = k;
  a->ptr[i] = node;
  a->keynum++;
  if (a->keynum <= m - 1)
    return;
  else { /*分裂节点然后插入父节点  |1 2 3  ...|ceil(m)(向上取整)|... m|   */
    int midkey =
        a->key[(int)ceil(m / 2.0)]; /*中间关键字及 NewNode 要插入父节点*/
    TreeNode *NewNode = new TreeNode(a->parent, a->key, a->ptr); /*和a同parent*/
                                                                 /*
                                                                 for (int i=0;i<=NewNode->keynum;i++)
                                                                 {
                                                                 if (NewNode->ptr[i])
                                                                 NewNode->ptr[i]->parent=NewNode;
                 }*/
    a->keynum = m - ceil(m / 2.0);
    TreeNode *tempa = a; /*记录当前节点*/
    a = a->parent;       /*父节点*/
    if (!a)              /*无父节点*/
    {
      TreeNode *NewRoot = new TreeNode(tempa, midkey, NewNode);
      tempa->parent = NewRoot;
      NewNode->parent = NewRoot;
      root = NewRoot;
      return;
    } else
      Insert(midkey, NewNode, a);
  }
}
Result Btree::Find(int k) {
  if (!root) {
    cout << "the tree is null !" << endl;
    return Result(0, 0, 0);
  }
  TreeNode *a = root;
  int i = 1;
  while (a) {
    i = 1;
    while (i <= a->keynum) {
      if (k <= a->key[i]) {
        break;
      } else {
        i++;
      }
    }
    if (k == a->key[i]) {
      return Result(a, i, 1);
    } else {
      if (!a->ptr[i - 1]) {

        return Result(a, i, 0);
      } else {
        a = a->ptr[i - 1];
      }
    }
  }
}
void Btree::DeleteBtree(int k) {
  if (!root) {
    cout << "The tree is null !" << endl;
    return;
  }
  /*转化为删除叶子节点中的关键字 找其右子树的最小关键字*/
  stack<int> s;             /*记录路径上的 所有 index */
  TreeNode *delnode = root; //待删除关键字k所在节点
  int i = 1;

  while (delnode && delnode->keynum) /*delnode->keynum ==0 是对B+树而言*/
  {
    i = 1;
    while (i <= delnode->keynum) {
      if (k <= delnode->key[i]) {
        break;
      } else {
        i++;
      }
    }
    if (k == delnode->key[i]) {
      break; /*找到了*/
    } else {
      if (delnode->ptr[i - 1] == 0) {
        /*无此关键字*/
        cout << "no  this key :" << k << endl;
        return;
      } else {
        /*向下一层*/
        delnode = delnode->ptr[i - 1];
        s.push(i - 1); /*通过该索引的指针向下一层查找*/
      }
    }
  }
  /* delnode  i   parent可以提供回去的路 */
  TreeNode *p = delnode; /*当前节点*/
  if (delnode->ptr[i] &&
      delnode->ptr[i]->keynum) /*delnode  不是叶子节点*/ /*B+tree 的元素节点*/
  {

    s.push(i);
    p = delnode->ptr[i];

    while (p->ptr[0] &&
           p->ptr[0]->keynum) /* p到达delnode 的右子树中最小关键字节点*/
    {
      p = p->ptr[0];
      if (!p->ptr[0]->keynum)
        break;
      s.push(0);
    }
  }
  if (p != delnode) {
    /*将删除操作到对叶子节点的关键字的删除*/
    delnode->key[i] = p->key[1];
    i = 1;
  }
  /* p, i  删除关键字由delnode i 转换为 p i */
  BorrowOrCombine(p, i, 0, s);
}
void Btree::BorrowOrCombine(TreeNode *a, int i, int type,
                            stack<int> &s) /*待处理关键字是a 节点的 i
type 标志此次操作的前一次是 对其左 -1  右 1  无0 孩子进行操作
即这是对叶子节点的操作*/
{
  if (a == root && root->keynum == 1) {

    TreeNode *oldroot = root;
    if (type == -1) {
      if (root->ptr[i])
        root = root->ptr[i];
      else
        root = 0;
    } else if (type == 1) {
      if (root->ptr[i - 1])
        root = root->ptr[i - 1];
      else
        root = 0;
    } else /*不是由下层传递而来*/
    {
      root = 0;
    }
    if (root)
      root->parent = 0;
    delete oldroot;

    return;
  }
  int minnum = ceil(m / 2.0) - 1;
  TreeNode *la, *ra; /*a 的左右兄弟节点*/
  //  if (!a->ptr[0])/*a 为叶子节点*/
  //  {
  TreeNode *pflag = a->ptr[i - 1]; /*对B+树 判断哪个元素节点被合并掉了 指针为0*/

  if (a->keynum > minnum || a == root) {
    for (int j = i; j < a->keynum; j++) {
      a->key[j] = a->key[j + 1];
      if (type == -1) {
        a->ptr[j - 1] = a->ptr[j];
      } else if (type == 1) {
        a->ptr[j] = a->ptr[j + 1];
      } else {
        /*这是对叶子节点的操作  B+树 而言*/
        if (pflag) {
          a->ptr[j] = a->ptr[j + 1];
        } else {
          a->ptr[j - 1] = a->ptr[j];
        }
      }
    }
    if (!type && !pflag) {
      a->ptr[j - 1] = a->ptr[j];
    }
    if (type == -1) {
      a->ptr[j - 1] = a->ptr[j];
    }
    a->key[j] = 0;
    a->ptr[j] = 0;
    a->keynum--;
    return;
  } else { /* aa->keynum=minnum */
    int index = s.top();
    s.pop();
    /*能借则借 借优先*/
    if (index) /*有左兄弟*/
    {
      la = a->parent->ptr[index - 1];
      if (la->keynum > minnum) /*左兄弟关键字足够多可以借*/
      {
        /* 从左兄弟借 */
        /*向后移动覆盖 i */
        for (int j = i; j > 1; j--) {
          a->key[j] = a->key[j - 1];
          if (type == -1) {
            a->ptr[j - 2] = a->ptr[j - 1];
          } else if (type == 1) {
            a->ptr[j - 1] = a->ptr[j];
          } else {
            if (pflag) {
              a->ptr[j - 1] = a->ptr[j];
            } else {
              a->ptr[j - 2] = a->ptr[j - 1];
            }
          }
        }
        if (!type && pflag) {
          a->ptr[j - 1] = a->ptr[j];
        }
        if (type == 1) {
          a->ptr[j - 1] = a->ptr[j];
        }
        a->key[j] = a->parent->key[index];
        a->ptr[0] = la->ptr[la->keynum]; /*左兄弟的最右子树*/
        /*父节点改变*/
        la->ptr[la->keynum]->parent = a;
        a->parent->key[index] = la->key[la->keynum];

        la->key[la->keynum] = 0;
        la->ptr[la->keynum] = 0;
        la->keynum--;

        return;
      }
    }
    if (index < a->keynum) /*有右兄弟index<=a->keynum*/
    {

      ra = a->parent->ptr[index + 1];

      if (ra->keynum > minnum) /*右兄弟关键字足够多可以借*/
      {
        /* 从右兄弟借 */
        /*向前移动覆盖 i */
        for (int j = i; j < a->keynum; j++) {
          a->key[j] = a->key[j + 1];
          if (type == -1) {
            a->ptr[j - 1] = a->ptr[j];
          } else if (type == 1) {
            a->ptr[j] = a->ptr[j + 1];
          } else {
            if (pflag) {
              a->ptr[j] = a->ptr[j + 1];
            } else {
              a->ptr[j - 1] = a->ptr[j];
            }
          }
        }
        if (!type && !pflag) {
          a->ptr[j - 1] = a->ptr[j];
        }
        if (type == -1) {
          a->ptr[j - 1] = a->ptr[j];
        }
        a->key[j] = a->parent->key[index + 1];
        a->ptr[j] = ra->ptr[0]; /*右兄弟的最左子树*/
        if (ra->ptr[0])         /*叶子节点的 -》ptr【0】==0*/
          ra->ptr[0]->parent = a;
        a->parent->key[index + 1] = ra->key[1];
        /*右兄弟关键字去头 前移*/
        for (int t = 1; t < ra->keynum; t++) {
          ra->ptr[t - 1] = ra->ptr[t];
          ra->key[t] = ra->key[t + 1];
        }
        /*t= ra->keynum */
        ra->ptr[t - 1] = ra->ptr[t];
        ra->key[t] = 0;
        ra->ptr[t] = 0;
        ra->keynum--;
        return;
      }
    }
    /*合并可能会使 不完善节点向上传递*/
    if (index) /*有左兄弟*/
    {
      la = a->parent->ptr[index - 1];
      if (la->keynum == minnum) /*左兄弟关键字不够多*/
      {
        /* 合并到左兄弟 */

        la->key[la->keynum + 1] = a->parent->key[index];
        /*a 中的关键字填充到其左兄弟中 0 1 2 .... i-1 | i |  i+1 ...... kyenum
         */

        for (int l = 1; l <= i - 1; l++) {
          la->key[la->keynum + l + 1] = a->key[l];
          la->ptr[la->keynum + l] = a->ptr[l - 1];
          /*子树的父节点改变*/
          if (a->ptr[l - 1])
            a->ptr[l - 1]->parent = la;
        }

        if (type == -1) {
          la->ptr[la->keynum + l] = a->ptr[l];
          if (a->ptr[l])
            a->ptr[l]->parent = la;
        } else if (type == 1) {
          la->ptr[la->keynum + l] = a->ptr[l - 1];
          if (a->ptr[l - 1])
            a->ptr[l - 1]->parent = la;
        } else {
          if (pflag) {
            la->ptr[la->keynum + l] = a->ptr[l - 1];
            if (a->ptr[l - 1])
              a->ptr[l - 1]->parent = la;
          } else {
            la->ptr[la->keynum + l] = a->ptr[l];
            if (a->ptr[l])
              a->ptr[l]->parent = la;
          }
        }

        for (l = i; l < a->keynum; l++) {
          la->key[la->keynum + l + 1] = a->key[l + 1];
          la->ptr[la->keynum + l + 1] = a->ptr[l + 1];
          if (a->ptr[l + 1])
            a->ptr[l + 1]->parent = la;
        }
        la->keynum = m - 1;
        TreeNode *tempp = a->parent;
        tempp->ptr[index] = 0;
        delete a;
        BorrowOrCombine(tempp, index, 1, s);
        return;
      }
    }
    if (index < a->keynum) /*有右兄弟index<=a->keynum*/
    {
      ra = a->parent->ptr[index + 1];
      if (ra->keynum == minnum) /*右兄弟关键字不够多*/
      {
        /*合并到右兄弟 */
        /* 右兄弟关键字右移 让出合并位置*/
        for (int k = ra->keynum; k > 0; k--) {
          ra->key[k + a->keynum] = ra->key[k];
          ra->ptr[k + a->keynum] = ra->ptr[k];
        }
        ra->ptr[a->keynum] = ra->ptr[0];

        ra->key[a->keynum] = a->parent->key[index + 1];
        /*a 中的关键字填充到其右兄弟中 0 1 2 .... i-1 | i |  i+1 ...... kyenum
         */

        for (int l = 1; l <= i - 1; l++) {
          ra->ptr[l - 1] = a->ptr[l - 1];
          ra->key[l] = a->key[l];
          /*子树的父节点改变*/
          if (a->ptr[l - 1])
            a->ptr[l - 1]->parent = ra;
        }
        if (type == -1) {
          ra->ptr[l - 1] = a->ptr[l];
          if (a->ptr[l])
            a->ptr[l]->parent = ra;
        } else if (type == 1) {
          ra->ptr[l - 1] = a->ptr[l - 1];
          if (a->ptr[l - 1])
            a->ptr[l - 1]->parent = ra;
        } else {
          if (pflag) {
            ra->ptr[l - 1] = a->ptr[l - 1];
            if (a->ptr[l - 1])
              a->ptr[l - 1]->parent = ra;
          } else {
            ra->ptr[l - 1] = a->ptr[l];
            if (a->ptr[l])
              a->ptr[l]->parent = ra;
          }
        }
        /*叶子节点无所谓 都是 0 */
        for (l = i + 1; l <= a->keynum; l++) {
          ra->key[l] = a->key[l];
          ra->ptr[l] = a->ptr[l];
          if (a->ptr[l])
            a->ptr[l]->parent = ra;
        }
        ra->keynum = m - 1;
        TreeNode *tempp = a->parent;
        tempp->ptr[index] = 0;
        delete a; /*删除节点a */
        /**/

        BorrowOrCombine(tempp, index + 1, -1, s);
        return;
      }
    }
  }
}
/*
* **************************************************************
*  B+tree
*/

void BPlusTree::InsertBplustree(int k) {
  if (!root) {
    if (!header) {
      header = new DataNode(k);
    } else {
      InsertData(k, header);
    }
    return;
  }
  /*find the data node where to insert the key */
  TreeNode *inode = root;

  while (inode->keynum) /*不是叶子节点就继续向下*/
  {
    int i = 1;
    while (i < inode->keynum) {
      if (k > inode->key[i]) {
        i++;
      } else
        break;
    }

    if (k < inode->key[i]) {
      /*左子树*/
      inode = inode->ptr[i - 1];
    } else {
      /*右子树*/
      inode = inode->ptr[i];
    }
  }
  /*找到相应的叶子节点 inode */
  InsertData(k, (DataNode *)inode);
}
void BPlusTree::InsertData(int k, DataNode *dn) {
  int i = 1;
  while (i <= dn->datanum) {
    if (k > dn->data[i]) {
      i++;
    } else
      break;
  }
  /**/
  if (k == dn->data[i]) {
    /*关键字已存在*/
    return;
  } else {
    /*数据后移以便插入关键字*/
    for (int j = L; j >= i; j--) {
      dn->data[j + 1] = dn->data[j];
    }
    dn->data[i] = k;
    dn->datanum++;
    if (dn->datanum > L) /*溢出需分裂*/
    {
      /*分裂为前后 两段 dn NewNode*/
      DataNode *NewNode = new DataNode(dn->parent, dn, dn->next, dn->data);
      if (dn->next) /*存在下一个数据节点*/
        dn->next->pre = NewNode;
      dn->next = NewNode;
      if (!root) /*第一次分裂*/
      {
        root = new TreeNode(dn, NewNode->data[1],
                            NewNode); /*后半段的第一个关键字放在插入其父节点*/
        dn->parent = root;
        NewNode->parent = root;
      } else {
        Insert(NewNode->data[1], NewNode, NewNode->parent);
      }
    }
  }
}
void BPlusTree::DeleteBPlustree(int k) {
  /*仅有header 无 root*/
  if (!root) {
    if (!header) {
      cout << "B+tree is null !" << endl;
      return;
    } else {
      int i = 1;
      while (i <= header->datanum) {
        if (k == header->data[i]) {
          for (int j = i; j < header->datanum; j++) {
            header->data[j] = header->data[j + 1];
          }
          header->datanum--;
          if (!header->datanum)
            header = 0;
          return;
        } else {
          i++;
          if (i == header->datanum) {
            cout << "No this key :" << k << endl;
            return;
          }
        }
      }
    }
  }
  /*找到关键字k在的元素节点 d（elete）node */
  TreeNode *dnode = root;
  int i = 1;
  while (dnode->keynum) /*不是元素节点则继续向下查找*/
  {
    i = 1;
    while (i <= dnode->keynum) {
      if (k >= dnode->key[i]) {
        i++;
      } else
        break;
    }
    /*左子树*/
    dnode = dnode->ptr[i - 1];
  }
  /* 找到相应的元素节点 dnode */
  int index = i;
  int minnum = ceil(L / 2); /*元素节点最少关键字数目*/
  DataNode *datanode = (DataNode *)dnode;
  DataNode *la, *ra;              /*该元素节点的左右节点*/
  if (datanode->datanum > minnum) /*关键字足够多 ，移动覆盖即可*/
  {
    int i = 1;
    while (i <= datanode->datanum) {
      if (datanode->data[i] == k) {
        for (int j = i; j < datanode->datanum; j++) {
          datanode->data[j] = datanode->data[j + 1];
        }
        datanode->data[j] = 0;
        datanode->datanum--;
        return;
      } else {
        if (i == datanode->datanum) {
          cout << "No this key" << k << endl;
          return;
        } else
          i++;
      }
    }
  } else /*元素不够多 从左右邻居节点借或者合并  由上面查找过程知*/
         /*index 是该元素节点的索引*/
  {
    /*查找该关键字*/
    int i = 1;
    while (i <= datanode->datanum) {
      /*找到并覆盖*/
      if (datanode->data[i] == k) {
        for (int j = i; j < datanode->datanum; j++) {
          datanode->data[j] = datanode->data[j + 1];
        }
        datanode->data[j] = 0;
        datanode->datanum--;
        break;
      } else {
        if (i == datanode->datanum) {
          cout << "No this key" << k << endl;
          return;
        } else
          i++;
      }
    }
    /*删除后节点不完善*/

    /*借*/
    if (index - 1) /*有左兄弟*/
    {
      la = (DataNode *)datanode->parent->ptr[index - 2];
      if (la->datanum > minnum) /*左兄弟的关键字足够多可以借*/
      {
        for (int j = datanode->datanum; j >= 1; j--) {
          datanode->data[j + 1] = datanode->data[j];
        }
        datanode->data[1] = la->data[la->datanum]; /*最大关键字*/
        datanode->parent->key[index - 1] = la->data[la->datanum];
        la->data[la->datanum] = 0;
        la->datanum--;
        datanode->datanum++;
        return;
      }
    }
    if (index <= datanode->parent->keynum) /*有右兄弟*/
    {

      ra = (DataNode *)datanode->parent->ptr[index];
      if (ra->datanum > minnum) /*右兄弟的关键字足够多可以借*/
      {
        /*加在datanode末尾*/
        datanode->data[datanode->datanum + 1] = ra->data[1];
        datanode->datanum++;
        datanode->parent->key[index] = ra->data[2];
        /*右兄弟移动覆盖*/
        for (int j = 1; j < ra->datanum; j++) {
          ra->data[j] = ra->data[j + 1];
        }
        ra->data[j] = 0;
        ra->datanum--;
        return;
      }
    }
    /*邻居节点的关键字不够多  合并*/
    if (index - 1) /*有左兄弟*/
    {
      /*关键字合并到左兄弟尾部 并且删除对应搜索节点的关键字*/
      la = (DataNode *)datanode->parent->ptr[index - 2];
      for (int j = 1; j <= datanode->datanum; j++) {
        la->data[la->datanum + 1] = datanode->data[j];
      }
      la->parent->ptr[index - 1] = 0;

      la->next = datanode->next;
      if (datanode->next) /*是尾节点*/
        datanode->next->pre = la;
      delete datanode;
      DeleteBtree(la->parent->key[index - 1]);
      return;
    }
    if (index <= datanode->parent->keynum) /*有右兄弟*/
    {
      /*关键字合并到右兄弟头部 并且删除对应搜索节点的关键字*/
      ra = (DataNode *)datanode->parent->ptr[index];
      /*右兄弟关键字后移出位置以便合并*/
      for (int j = ra->datanum; j >= 1; j--) {
        ra->data[j + datanode->datanum] = ra->data[j];
      }
      /*填充到右兄弟节点头部*/
      for (int l = 1; l <= datanode->datanum; l++) {
        ra[l] = datanode[l];
      }
      ra->datanum += datanode->datanum;
      /*数据节点链操作 注意首尾的处理*/
      ra->pre = datanode->pre;
      if (datanode->pre) {
        datanode->pre->next = ra;
      } else {
        /*datanode 为 header*/
        header = ra;
      }
      ra->parent->ptr[index - 1] = 0;
      delete datanode;
      DeleteBtree(ra->parent->key[index]);
      return;
    }
  }
}
