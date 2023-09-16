#include "stdio.h"

extern int test_ja_list();
extern int test_ja_rbtree();

int main()
{
  test_ja_list();
  test_ja_rbtree();
  return 0;
}
