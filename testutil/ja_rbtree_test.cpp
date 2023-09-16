#include "stdio.h"

/*1. target lib */
#include "math.h"
#include "area.h"

/*2. util & test*/
#include "test.h"
#include "ja_mem.h"
#include "ja_list.h"
#include "ja_rbtree.h"

#include "ja_type.h"

/* For ja rbtree , Just test the function.*/

#define CEPH_MAX_DIRFRAG_REP 4

struct ceph_inode_frag {
	struct ja_rb_node node;

	/* fragtree state */
	u32 frag;
	int split_by;		  /* i.e. 2^(split_by) children */

	/* delegation and replication info */
	int mds;			  /* -1 if same authority as parent */
	int ndist;			  /* >0 if replicated */
	int dist[CEPH_MAX_DIRFRAG_REP];
};

class Ja_rbtree_test: public Test{
public:
  void test()
  {
    printf("ja rbtree test 100 size, for CEPH_MAX_DIRFRAG_REP, result : %d , time cost : %f nsec\n",
        test_ja_rbtree(100),
        calculate_time_spend_nsec());
  }

  struct ja_rb_root i_fragtree = JA_RB_ROOT;
  static int ceph_frag_compare(u32 va, u32 vb)
  {
	  if (va < vb)
		  return -1;
	  if (va > vb)
		  return 1;
	  return 0;
  }
  
  static inline void * ERR_PTR(long error)
  {
	  return (void *) error;
  }

  static struct ceph_inode_frag *__get_or_create_frag(struct ja_rb_root *p_fragtree,
							  u32 f)
  {
	  struct ja_rb_node **p;
	  struct ja_rb_node *parent = NULL;
	  struct ceph_inode_frag *frag;
	  int c;
  
	  p = &p_fragtree->rb_node;
	  while (*p) {
		  parent = *p;
		  frag = ja_rb_entry(parent, struct ceph_inode_frag, node);
		  c = ceph_frag_compare(f, frag->frag);
		  if (c < 0)
			  p = &(*p)->ja_rb_left;
		  else if (c > 0)
			  p = &(*p)->ja_rb_right;
		  else
			  return frag;
	  }
  
	  frag = (struct ceph_inode_frag *)ja_malloc(sizeof(*frag), 1);
	  if (!frag)
		  return (struct ceph_inode_frag *)ERR_PTR(0x12);
  
	  frag->frag = f;
	  frag->split_by = 0;
	  frag->mds = -1;
	  frag->ndist = 0;
  
	  ja_rb_link_node(&frag->node, parent, p);
	  ja_rb_insert_color(&frag->node, p_fragtree);
  
	  printf("min_debug: add frag f=%d with frag_node %x to rbtree %x \n",
	          f, frag, p_fragtree->rb_node);
	  return frag;
  }
  int test_ja_rbtree(int count){
  	for(int i = 0; i < count; i++){
	  __get_or_create_frag(&i_fragtree, i);
	}
    return 1;
  }
  int mathimatic_call(){
    return test_ja_rbtree(100);
  }
};

int test_ja_rbtree()
{
  Ja_rbtree_test ja_rbtree;
  ja_rbtree.test();
  return 0;
}
