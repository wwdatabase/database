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
  TreeNode *a = root; /*��ǰ�ڵ�*/
  int i = 1;          /*k�ؽ��� Ҫ����ڵ�a ��λ������*/
  /*�ҵ�����ڵ�*/
  while (a) {
    i = 1;
    /*��a ���ҵ���һ���ȹؼ���k��Ĺؼ��ֵ�λ�� i */
    while (i <= a->keynum) {
      if (k <= a->key[i])
        break;
      else
        i++;
    }
    /* �ж��Ƿ�������� �����Ѿ������ӽڵ� */
    if (!a->ptr[i - 1]) /* ����Ҷ�ӽڵ��������� ֱ�Ӳ��� */
      break;
    else /*����Ҷ�ӽڵ�*/
      a = a->ptr[i - 1];
  }
  if (a->key[i] == k) /*�ùؼ��ֽڵ��Ѵ��� */
    return;
  Insert(k, 0, a); /*��Ҷ�ӽڵ��в���ؼ���k*/
}
void Btree::Insert(int k, TreeNode *node,
                   TreeNode *a) /*�ؼ��֣�k  �ùؼ���k��������ָ��  ����ڵ�a */
{
  int i = 1;
  /*��a ���ҵ���һ���ȹؼ���k��Ĺؼ��ֵ�λ�� i */
  while (i <= a->keynum) {
    if (k <= a->key[i])
      break;
    else
      i++;
  }
  /*����ڵ�Ϊ a  ����Ϊ i */
  for (int j = a->keynum; j >= i; j--) /*����ƶ��Ա�����¹ؼ���*/
  {
    a->key[j + 1] = a->key[j]; /* �ؼ���*/
    a->ptr[j + 1] = a->ptr[j]; /*����ָ��*/
  }
  a->key[i] = k;
  a->ptr[i] = node;
  a->keynum++;
  if (a->keynum <= m - 1)
    return;
  else { /*���ѽڵ�Ȼ����븸�ڵ�  |1 2 3  ...|ceil(m)(����ȡ��)|... m|   */
    int midkey =
        a->key[(int)ceil(m / 2.0)]; /*�м�ؼ��ּ� NewNode Ҫ���븸�ڵ�*/
    TreeNode *NewNode = new TreeNode(a->parent, a->key, a->ptr); /*��aͬparent*/
                                                                 /*
                                                                 for (int i=0;i<=NewNode->keynum;i++)
                                                                 {
                                                                 if (NewNode->ptr[i])
                                                                 NewNode->ptr[i]->parent=NewNode;
                 }*/
    a->keynum = m - ceil(m / 2.0);
    TreeNode *tempa = a; /*��¼��ǰ�ڵ�*/
    a = a->parent;       /*���ڵ�*/
    if (!a)              /*�޸��ڵ�*/
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
  /*ת��Ϊɾ��Ҷ�ӽڵ��еĹؼ��� ��������������С�ؼ���*/
  stack<int> s;             /*��¼·���ϵ� ���� index */
  TreeNode *delnode = root; //��ɾ���ؼ���k���ڽڵ�
  int i = 1;

  while (delnode && delnode->keynum) /*delnode->keynum ==0 �Ƕ�B+������*/
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
      break; /*�ҵ���*/
    } else {
      if (delnode->ptr[i - 1] == 0) {
        /*�޴˹ؼ���*/
        cout << "no  this key :" << k << endl;
        return;
      } else {
        /*����һ��*/
        delnode = delnode->ptr[i - 1];
        s.push(i - 1); /*ͨ����������ָ������һ�����*/
      }
    }
  }
  /* delnode  i   parent�����ṩ��ȥ��· */
  TreeNode *p = delnode; /*��ǰ�ڵ�*/
  if (delnode->ptr[i] &&
      delnode->ptr[i]->keynum) /*delnode  ����Ҷ�ӽڵ�*/ /*B+tree ��Ԫ�ؽڵ�*/
  {

    s.push(i);
    p = delnode->ptr[i];

    while (p->ptr[0] &&
           p->ptr[0]->keynum) /* p����delnode ������������С�ؼ��ֽڵ�*/
    {
      p = p->ptr[0];
      if (!p->ptr[0]->keynum)
        break;
      s.push(0);
    }
  }
  if (p != delnode) {
    /*��ɾ����������Ҷ�ӽڵ�Ĺؼ��ֵ�ɾ��*/
    delnode->key[i] = p->key[1];
    i = 1;
  }
  /* p, i  ɾ���ؼ�����delnode i ת��Ϊ p i */
  BorrowOrCombine(p, i, 0, s);
}
void Btree::BorrowOrCombine(TreeNode *a, int i, int type,
                            stack<int> &s) /*������ؼ�����a �ڵ�� i
type ��־�˴β�����ǰһ���� ������ -1  �� 1  ��0 ���ӽ��в���
�����Ƕ�Ҷ�ӽڵ�Ĳ���*/
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
    } else /*�������²㴫�ݶ���*/
    {
      root = 0;
    }
    if (root)
      root->parent = 0;
    delete oldroot;

    return;
  }
  int minnum = ceil(m / 2.0) - 1;
  TreeNode *la, *ra; /*a �������ֵܽڵ�*/
  //  if (!a->ptr[0])/*a ΪҶ�ӽڵ�*/
  //  {
  TreeNode *pflag = a->ptr[i - 1]; /*��B+�� �ж��ĸ�Ԫ�ؽڵ㱻�ϲ����� ָ��Ϊ0*/

  if (a->keynum > minnum || a == root) {
    for (int j = i; j < a->keynum; j++) {
      a->key[j] = a->key[j + 1];
      if (type == -1) {
        a->ptr[j - 1] = a->ptr[j];
      } else if (type == 1) {
        a->ptr[j] = a->ptr[j + 1];
      } else {
        /*���Ƕ�Ҷ�ӽڵ�Ĳ���  B+�� ����*/
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
    /*�ܽ���� ������*/
    if (index) /*�����ֵ�*/
    {
      la = a->parent->ptr[index - 1];
      if (la->keynum > minnum) /*���ֵܹؼ����㹻����Խ�*/
      {
        /* �����ֵܽ� */
        /*����ƶ����� i */
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
        a->ptr[0] = la->ptr[la->keynum]; /*���ֵܵ���������*/
        /*���ڵ�ı�*/
        la->ptr[la->keynum]->parent = a;
        a->parent->key[index] = la->key[la->keynum];

        la->key[la->keynum] = 0;
        la->ptr[la->keynum] = 0;
        la->keynum--;

        return;
      }
    }
    if (index < a->keynum) /*�����ֵ�index<=a->keynum*/
    {

      ra = a->parent->ptr[index + 1];

      if (ra->keynum > minnum) /*���ֵܹؼ����㹻����Խ�*/
      {
        /* �����ֵܽ� */
        /*��ǰ�ƶ����� i */
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
        a->ptr[j] = ra->ptr[0]; /*���ֵܵ���������*/
        if (ra->ptr[0])         /*Ҷ�ӽڵ�� -��ptr��0��==0*/
          ra->ptr[0]->parent = a;
        a->parent->key[index + 1] = ra->key[1];
        /*���ֵܹؼ���ȥͷ ǰ��*/
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
    /*�ϲ����ܻ�ʹ �����ƽڵ����ϴ���*/
    if (index) /*�����ֵ�*/
    {
      la = a->parent->ptr[index - 1];
      if (la->keynum == minnum) /*���ֵܹؼ��ֲ�����*/
      {
        /* �ϲ������ֵ� */

        la->key[la->keynum + 1] = a->parent->key[index];
        /*a �еĹؼ�����䵽�����ֵ��� 0 1 2 .... i-1 | i |  i+1 ...... kyenum
         */

        for (int l = 1; l <= i - 1; l++) {
          la->key[la->keynum + l + 1] = a->key[l];
          la->ptr[la->keynum + l] = a->ptr[l - 1];
          /*�����ĸ��ڵ�ı�*/
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
    if (index < a->keynum) /*�����ֵ�index<=a->keynum*/
    {
      ra = a->parent->ptr[index + 1];
      if (ra->keynum == minnum) /*���ֵܹؼ��ֲ�����*/
      {
        /*�ϲ������ֵ� */
        /* ���ֵܹؼ������� �ó��ϲ�λ��*/
        for (int k = ra->keynum; k > 0; k--) {
          ra->key[k + a->keynum] = ra->key[k];
          ra->ptr[k + a->keynum] = ra->ptr[k];
        }
        ra->ptr[a->keynum] = ra->ptr[0];

        ra->key[a->keynum] = a->parent->key[index + 1];
        /*a �еĹؼ�����䵽�����ֵ��� 0 1 2 .... i-1 | i |  i+1 ...... kyenum
         */

        for (int l = 1; l <= i - 1; l++) {
          ra->ptr[l - 1] = a->ptr[l - 1];
          ra->key[l] = a->key[l];
          /*�����ĸ��ڵ�ı�*/
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
        /*Ҷ�ӽڵ�����ν ���� 0 */
        for (l = i + 1; l <= a->keynum; l++) {
          ra->key[l] = a->key[l];
          ra->ptr[l] = a->ptr[l];
          if (a->ptr[l])
            a->ptr[l]->parent = ra;
        }
        ra->keynum = m - 1;
        TreeNode *tempp = a->parent;
        tempp->ptr[index] = 0;
        delete a; /*ɾ���ڵ�a */
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

  while (inode->keynum) /*����Ҷ�ӽڵ�ͼ�������*/
  {
    int i = 1;
    while (i < inode->keynum) {
      if (k > inode->key[i]) {
        i++;
      } else
        break;
    }

    if (k < inode->key[i]) {
      /*������*/
      inode = inode->ptr[i - 1];
    } else {
      /*������*/
      inode = inode->ptr[i];
    }
  }
  /*�ҵ���Ӧ��Ҷ�ӽڵ� inode */
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
    /*�ؼ����Ѵ���*/
    return;
  } else {
    /*���ݺ����Ա����ؼ���*/
    for (int j = L; j >= i; j--) {
      dn->data[j + 1] = dn->data[j];
    }
    dn->data[i] = k;
    dn->datanum++;
    if (dn->datanum > L) /*��������*/
    {
      /*����Ϊǰ�� ���� dn NewNode*/
      DataNode *NewNode = new DataNode(dn->parent, dn, dn->next, dn->data);
      if (dn->next) /*������һ�����ݽڵ�*/
        dn->next->pre = NewNode;
      dn->next = NewNode;
      if (!root) /*��һ�η���*/
      {
        root = new TreeNode(dn, NewNode->data[1],
                            NewNode); /*���εĵ�һ���ؼ��ַ��ڲ����丸�ڵ�*/
        dn->parent = root;
        NewNode->parent = root;
      } else {
        Insert(NewNode->data[1], NewNode, NewNode->parent);
      }
    }
  }
}
void BPlusTree::DeleteBPlustree(int k) {
  /*����header �� root*/
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
  /*�ҵ��ؼ���k�ڵ�Ԫ�ؽڵ� d��elete��node */
  TreeNode *dnode = root;
  int i = 1;
  while (dnode->keynum) /*����Ԫ�ؽڵ���������²���*/
  {
    i = 1;
    while (i <= dnode->keynum) {
      if (k >= dnode->key[i]) {
        i++;
      } else
        break;
    }
    /*������*/
    dnode = dnode->ptr[i - 1];
  }
  /* �ҵ���Ӧ��Ԫ�ؽڵ� dnode */
  int index = i;
  int minnum = ceil(L / 2); /*Ԫ�ؽڵ����ٹؼ�����Ŀ*/
  DataNode *datanode = (DataNode *)dnode;
  DataNode *la, *ra;              /*��Ԫ�ؽڵ�����ҽڵ�*/
  if (datanode->datanum > minnum) /*�ؼ����㹻�� ���ƶ����Ǽ���*/
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
  } else /*Ԫ�ز����� �������ھӽڵ����ߺϲ�  ��������ҹ���֪*/
         /*index �Ǹ�Ԫ�ؽڵ������*/
  {
    /*���Ҹùؼ���*/
    int i = 1;
    while (i <= datanode->datanum) {
      /*�ҵ�������*/
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
    /*ɾ����ڵ㲻����*/

    /*��*/
    if (index - 1) /*�����ֵ�*/
    {
      la = (DataNode *)datanode->parent->ptr[index - 2];
      if (la->datanum > minnum) /*���ֵܵĹؼ����㹻����Խ�*/
      {
        for (int j = datanode->datanum; j >= 1; j--) {
          datanode->data[j + 1] = datanode->data[j];
        }
        datanode->data[1] = la->data[la->datanum]; /*���ؼ���*/
        datanode->parent->key[index - 1] = la->data[la->datanum];
        la->data[la->datanum] = 0;
        la->datanum--;
        datanode->datanum++;
        return;
      }
    }
    if (index <= datanode->parent->keynum) /*�����ֵ�*/
    {

      ra = (DataNode *)datanode->parent->ptr[index];
      if (ra->datanum > minnum) /*���ֵܵĹؼ����㹻����Խ�*/
      {
        /*����datanodeĩβ*/
        datanode->data[datanode->datanum + 1] = ra->data[1];
        datanode->datanum++;
        datanode->parent->key[index] = ra->data[2];
        /*���ֵ��ƶ�����*/
        for (int j = 1; j < ra->datanum; j++) {
          ra->data[j] = ra->data[j + 1];
        }
        ra->data[j] = 0;
        ra->datanum--;
        return;
      }
    }
    /*�ھӽڵ�Ĺؼ��ֲ�����  �ϲ�*/
    if (index - 1) /*�����ֵ�*/
    {
      /*�ؼ��ֺϲ������ֵ�β�� ����ɾ����Ӧ�����ڵ�Ĺؼ���*/
      la = (DataNode *)datanode->parent->ptr[index - 2];
      for (int j = 1; j <= datanode->datanum; j++) {
        la->data[la->datanum + 1] = datanode->data[j];
      }
      la->parent->ptr[index - 1] = 0;

      la->next = datanode->next;
      if (datanode->next) /*��β�ڵ�*/
        datanode->next->pre = la;
      delete datanode;
      DeleteBtree(la->parent->key[index - 1]);
      return;
    }
    if (index <= datanode->parent->keynum) /*�����ֵ�*/
    {
      /*�ؼ��ֺϲ������ֵ�ͷ�� ����ɾ����Ӧ�����ڵ�Ĺؼ���*/
      ra = (DataNode *)datanode->parent->ptr[index];
      /*���ֵܹؼ��ֺ��Ƴ�λ���Ա�ϲ�*/
      for (int j = ra->datanum; j >= 1; j--) {
        ra->data[j + datanode->datanum] = ra->data[j];
      }
      /*��䵽���ֵܽڵ�ͷ��*/
      for (int l = 1; l <= datanode->datanum; l++) {
        ra[l] = datanode[l];
      }
      ra->datanum += datanode->datanum;
      /*���ݽڵ������� ע����β�Ĵ���*/
      ra->pre = datanode->pre;
      if (datanode->pre) {
        datanode->pre->next = ra;
      } else {
        /*datanode Ϊ header*/
        header = ra;
      }
      ra->parent->ptr[index - 1] = 0;
      delete datanode;
      DeleteBtree(ra->parent->key[index]);
      return;
    }
  }
}
