/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  (C) 2002  David Woodhouse <dwmw2@infradead.org>
  (C) 2012  Michel Lespinasse <walken@google.com>
  (C) 2023  Min Wang <wangmin0880@163.com>


  Optimized from linux/include/linux/rbtree_augmented.h
*/

#ifndef _JA_RBTREE_AUGMENTED_H_
#define _JA_RBTREE_AUGMENTED_H_

#define JA_READ_ONCE(x) (*(volatile typeof(x) *) &(x))

#define __JA_WRITE_ONCE(x, val)						\
do {									\
	*(volatile typeof(x) *)&(x) = (val);				\
} while (0)

#define JA_WRITE_ONCE(x, val)						\
do {									\
	__JA_WRITE_ONCE(x, val);						\
} while (0)



#include "ja_rbtree.h"

/*
 * Please note - only struct ja_rb_augment_callbacks and the prototypes for
 * ja_rb_insert_augmented() and ja_rb_erase_augmented() are intended to be public.
 * The rest are implementation details you are not expected to depend on.
 */

struct ja_rb_augment_callbacks {
	void (*propagate)(struct ja_rb_node *node, struct ja_rb_node *stop);
	void (*copy)(struct ja_rb_node *old, struct ja_rb_node *newer);
	void (*rotate)(struct ja_rb_node *old, struct ja_rb_node *newer);
};

extern void __ja_rb_insert_augmented(struct ja_rb_node *node, struct ja_rb_root *root,
	void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer));

/*
 * Fixup the rbtree and update the augmented information when rebalancing.
 *
 * On insertion, the user must update the augmented information on the path
 * leading to the inserted node, then call ja_rb_link_node() as usual and
 * ja_rb_insert_augmented() instead of the usual ja_rb_insert_color() call.
 * If ja_rb_insert_augmented() rebalances the rbtree, it will callback into
 * a user provided function to update the augmented information on the
 * affected subtrees.
 */
static inline void
ja_rb_insert_augmented(struct ja_rb_node *node, struct ja_rb_root *root,
		    const struct ja_rb_augment_callbacks *augment)
{
	__ja_rb_insert_augmented(node, root, augment->rotate);
}

static inline void
ja_rb_insert_augmented_cached(struct ja_rb_node *node,
			   struct ja_rb_root_cached *root, bool newleft,
			   const struct ja_rb_augment_callbacks *augment)
{
	if (newleft)
		root->ja_rb_leftmost = node;
	ja_rb_insert_augmented(node, &root->ja_rb_root, augment);
}

/*
 * Template for declaring augmented rbtree callbacks (generic case)
 *
 * RBSTATIC:    'static' or empty
 * RBNAME:      name of the rb_augment_callbacks structure
 * RBSTRUCT:    struct type of the tree nodes
 * RBFIELD:     name of struct rb_node field within RBSTRUCT
 * RBAUGMENTED: name of field within RBSTRUCT holding data for subtree
 * RBCOMPUTE:   name of function that recomputes the RBAUGMENTED data
 */

#define JA_RB_DECLARE_CALLBACKS(RBSTATIC, RBNAME,				\
			     RBSTRUCT, RBFIELD, RBAUGMENTED, RBCOMPUTE)	\
static inline void							\
RBNAME ## _propagate(struct ja_rb_node *rb, struct ja_rb_node *stop)		\
{									\
	while (rb != stop) {						\
		RBSTRUCT *node = ja_rb_entry(rb, RBSTRUCT, RBFIELD);	\
		if (RBCOMPUTE(node, true))				\
			break;						\
		rb = ja_rb_parent(&node->RBFIELD);				\
	}								\
}									\
static inline void							\
RBNAME ## _copy(struct ja_rb_node *rb_old, struct ja_rb_node *rb_new)		\
{									\
	RBSTRUCT *old = ja_rb_entry(rb_old, RBSTRUCT, RBFIELD);		\
	RBSTRUCT *newer = ja_rb_entry(rb_new, RBSTRUCT, RBFIELD);		\
	newer->RBAUGMENTED = old->RBAUGMENTED;				\
}									\
static void								\
RBNAME ## _rotate(struct ja_rb_node *rb_old, struct ja_rb_node *rb_new)	\
{									\
	RBSTRUCT *old = ja_rb_entry(rb_old, RBSTRUCT, RBFIELD);		\
	RBSTRUCT *newer = ja_rb_entry(rb_new, RBSTRUCT, RBFIELD);		\
	newer->RBAUGMENTED = old->RBAUGMENTED;				\
	RBCOMPUTE(old, false);						\
}									\
RBSTATIC const struct ja_rb_augment_callbacks RBNAME = {			\
	.propagate = RBNAME ## _propagate,				\
	.copy = RBNAME ## _copy,					\
	.rotate = RBNAME ## _rotate					\
};

/*
 * Template for declaring augmented rbtree callbacks,
 * computing RBAUGMENTED scalar as max(RBCOMPUTE(node)) for all subtree nodes.
 *
 * RBSTATIC:    'static' or empty
 * RBNAME:      name of the rb_augment_callbacks structure
 * RBSTRUCT:    struct type of the tree nodes
 * RBFIELD:     name of struct rb_node field within RBSTRUCT
 * RBTYPE:      type of the RBAUGMENTED field
 * RBAUGMENTED: name of RBTYPE field within RBSTRUCT holding data for subtree
 * RBCOMPUTE:   name of function that returns the per-node RBTYPE scalar
 */

#define JA_RB_DECLARE_CALLBACKS_MAX(RBSTATIC, RBNAME, RBSTRUCT, RBFIELD,	      \
				 RBTYPE, RBAUGMENTED, RBCOMPUTE)	      \
static inline bool RBNAME ## _compute_max(RBSTRUCT *node, bool exit)	      \
{									      \
	RBSTRUCT *child;						      \
	RBTYPE max = RBCOMPUTE(node);					      \
	if (node->RBFIELD.ja_rb_left) {					      \
		child = ja_rb_entry(node->RBFIELD.rb_left, RBSTRUCT, RBFIELD);   \
		if (child->RBAUGMENTED > max)				      \
			max = child->RBAUGMENTED;			      \
	}								      \
	if (node->RBFIELD.ja_rb_right) {					      \
		child = ja_rb_entry(node->RBFIELD.ja_rb_right, RBSTRUCT, RBFIELD);  \
		if (child->RBAUGMENTED > max)				      \
			max = child->RBAUGMENTED;			      \
	}								      \
	if (exit && node->RBAUGMENTED == max)				      \
		return true;						      \
	node->RBAUGMENTED = max;					      \
	return false;							      \
}									      \
JA_RB_DECLARE_CALLBACKS(RBSTATIC, RBNAME,					      \
		     RBSTRUCT, RBFIELD, RBAUGMENTED, RBNAME ## _compute_max)


#define	JA_RB_RED		0
#define	JA_RB_BLACK	1

#define __ja_rb_parent(pc)    ((struct ja_rb_node *)(pc & ~3))

#define __ja_rb_color(pc)     ((pc) & 1)
#define __ja_rb_is_black(pc)  __ja_rb_color(pc)
#define __ja_rb_is_red(pc)    (!__ja_rb_color(pc))
#define ja_rb_color(rb)       __ja_rb_color((rb)->_ja_rb_parent_color)
#define ja_rb_is_red(rb)      __ja_rb_is_red((rb)->_ja_rb_parent_color)
#define ja_rb_is_black(rb)    __ja_rb_is_black((rb)->_ja_rb_parent_color)

static inline void ja_rb_set_parent(struct ja_rb_node *rb, struct ja_rb_node *p)
{
	rb->_ja_rb_parent_color = ja_rb_color(rb) | (unsigned long)p;
}

static inline void ja_rb_set_parent_color(struct ja_rb_node *rb,
				       struct ja_rb_node *p, int color)
{
	rb->_ja_rb_parent_color = (unsigned long)p | color;
}

static inline void
__ja_rb_change_child(struct ja_rb_node *old, struct ja_rb_node *newer,
		  struct ja_rb_node *parent, struct ja_rb_root *root)
{
	if (parent) {
		if (parent->ja_rb_left == old)
			JA_WRITE_ONCE(parent->ja_rb_left, newer);
		else
			JA_WRITE_ONCE(parent->ja_rb_right, newer);
	} else
		JA_WRITE_ONCE(root->rb_node, newer);
}
/*
static inline void
__ja_rb_change_child_rcu(struct ja_rb_node *old, struct ja_rb_node *newer,
		      struct ja_rb_node *parent, struct ja_rb_root *root)
{
	if (parent) {
		if (parent->ja_rb_left == old)
			ja_rcu_assign_pointer(parent->ja_rb_left, newer);
		else
			ja_rcu_assign_pointer(parent->ja_rb_right, newer);
	} else
		ja_rcu_assign_pointer(root->rb_node, newer);
}
*/
extern void __ja_rb_erase_color(struct ja_rb_node *parent, struct ja_rb_root *root,
	void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer));

static __always_inline struct ja_rb_node *
__ja_rb_erase_augmented(struct ja_rb_node *node, struct ja_rb_root *root,
		     const struct ja_rb_augment_callbacks *augment)
{
	struct ja_rb_node *child = node->ja_rb_right;
	struct ja_rb_node *tmp = node->ja_rb_left;
	struct ja_rb_node *parent, *rebalance;
	unsigned long pc;

	if (!tmp) {
		/*
		 * Case 1: node to erase has no more than 1 child (easy!)
		 *
		 * Note that if there is one child it must be red due to 5)
		 * and node must be black due to 4). We adjust colors locally
		 * so as to bypass __rb_erase_color() later on.
		 */
		pc = node->_ja_rb_parent_color;
		parent = __ja_rb_parent(pc);
		__ja_rb_change_child(node, child, parent, root);
		if (child) {
			child->_ja_rb_parent_color = pc;
			rebalance = NULL;
		} else
			rebalance = __ja_rb_is_black(pc) ? parent : NULL;
		tmp = parent;
	} else if (!child) {
		/* Still case 1, but this time the child is node->rb_left */
		tmp->_ja_rb_parent_color = pc = node->_ja_rb_parent_color;
		parent = __ja_rb_parent(pc);
		__ja_rb_change_child(node, tmp, parent, root);
		rebalance = NULL;
		tmp = parent;
	} else {
		struct ja_rb_node *successor = child, *child2;

		tmp = child->ja_rb_left;
		if (!tmp) {
			/*
			 * Case 2: node's successor is its right child
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (s)  ->  (x) (c)
			 *        \
			 *        (c)
			 */
			parent = successor;
			child2 = successor->ja_rb_right;

			augment->copy(node, successor);
		} else {
			/*
			 * Case 3: node's successor is leftmost under
			 * node's right child subtree
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (y)  ->  (x) (y)
			 *      /            /
			 *    (p)          (p)
			 *    /            /
			 *  (s)          (c)
			 *    \
			 *    (c)
			 */
			do {
				parent = successor;
				successor = tmp;
				tmp = tmp->ja_rb_left;
			} while (tmp);
			child2 = successor->ja_rb_right;
			JA_WRITE_ONCE(parent->ja_rb_left, child2);
			JA_WRITE_ONCE(successor->ja_rb_right, child);
			ja_rb_set_parent(child, successor);

			augment->copy(node, successor);
			augment->propagate(parent, successor);
		}

		tmp = node->ja_rb_left;
		JA_WRITE_ONCE(successor->ja_rb_left, tmp);
		ja_rb_set_parent(tmp, successor);

		pc = node->_ja_rb_parent_color;
		tmp = __ja_rb_parent(pc);
		__ja_rb_change_child(node, successor, tmp, root);

		if (child2) {
			ja_rb_set_parent_color(child2, parent, JA_RB_BLACK);
			rebalance = NULL;
		} else {
			rebalance = ja_rb_is_black(successor) ? parent : NULL;
		}
		successor->_ja_rb_parent_color = pc;
		tmp = successor;
	}

	augment->propagate(tmp, NULL);
	return rebalance;
}

static __always_inline void
ja_rb_erase_augmented(struct ja_rb_node *node, struct ja_rb_root *root,
		   const struct ja_rb_augment_callbacks *augment)
{
	struct ja_rb_node *rebalance = __ja_rb_erase_augmented(node, root, augment);
	if (rebalance)
		__ja_rb_erase_color(rebalance, root, augment->rotate);
}

static __always_inline void
ja_rb_erase_augmented_cached(struct ja_rb_node *node, struct ja_rb_root_cached *root,
			  const struct ja_rb_augment_callbacks *augment)
{
	if (root->ja_rb_leftmost == node)
		root->ja_rb_leftmost = ja_rb_next(node);
	ja_rb_erase_augmented(node, &root->ja_rb_root, augment);
}

#endif	/* _LINUX_RBTREE_AUGMENTED_H */

