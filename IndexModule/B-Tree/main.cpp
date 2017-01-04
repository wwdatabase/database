#include "Btree.h"
#include <iostream>
#include <math.h>
using namespace std;

int main(int argc, char const *argv[]) {
  int a = 5;
  cout << "let the slices of the tree be :" << ceil(a / 2.0) << endl;

  /* 这是一颗B树 */
  Btree b;
  b.InsertBtree(33);
  b.InsertBtree(44);
  b.InsertBtree(28);
  b.InsertBtree(73);
  b.InsertBtree(52);
  b.InsertBtree(218);
  b.InsertBtree(71);
  b.DeleteBtree(44);
  b.DeleteBtree(33);
  b.DeleteBtree(28);
  b.DeleteBtree(28);
  b.DeleteBtree(52);
  b.DeleteBtree(218);
  b.DeleteBtree(44);
  b.DeleteBtree(71);
  b.DeleteBtree(44);

  /* 这是一棵B+树 */
  BPlusTree bp;
  bp.InsertBplustree(2);
  bp.InsertBplustree(3);
  bp.InsertBplustree(32);
  bp.InsertBplustree(13);
  bp.InsertBplustree(62);
  bp.InsertBplustree(93);
  bp.InsertBplustree(27);
  bp.InsertBplustree(83);
  bp.InsertBplustree(392);
  bp.InsertBplustree(173);
  bp.InsertBplustree(652);
  bp.InsertBplustree(923);
  bp.DeleteBPlustree(13);
  bp.DeleteBPlustree(27);
  bp.DeleteBPlustree(3);
  bp.DeleteBPlustree(2);

  return 0;
}
