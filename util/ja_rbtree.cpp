// SPDX-License-Identifier: GPL-2.0-or-later
/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  (C) 2002  David Woodhouse <dwmw2@infradead.org>
  (C) 2012  Michel Lespinasse <walken@google.com>
  (C) 2023  Min Wang <wangmin0880@163.com>


  Optimized from linux/lib/rbtree.c
*/

#include "ja_rbtree_augmented.h"

#ifndef unlikely
# define unlikely(x)		__builtin_expect(!!(x), 0)
#endif


/*
 * red-black trees properties:  https://en.wikipedia.org/wiki/Rbtree
 *
 *  1) A node is either red or black
 *  2) The root is black
 *  3) All leaves (NULL) are black
 *  4) Both children of every red node are black
 *  5) Every simple path from root to leaves contains the same number
 *     of black nodes.
 *
 *  4 and 5 give the O(log n) guarantee, since 4 implies you cannot have two
 *  consecutive red nodes in a path and every red node is therefore followed by
 *  a black. So if B is the number of black nodes on every simple path (as per
 *  5), then the longest possible path due to 4 is 2B.
 *
 *  We shall indicate color with case, where black nodes are uppercase and red
 *  nodes will be lowercase. Unknown color nodes shall be drawn as red within
 *  parentheses and have some accompanying text comment.
 */

/*
 * Notes on lockless lookups:
 *
 * All stores to the tree structure (rb_left and rb_right) must be done using
 * WRITE_ONCE(). And we must not inadvertently cause (temporary) loops in the
 * tree structure as seen in program order.
 *
 * These two requirements will allow lockless iteration of the tree -- not
 * correct iteration mind you, tree rotations are not atomic so a lookup might
 * miss entire subtrees.
 *
 * But they do guarantee that any such traversal will only see valid elements
 * and that it will indeed complete -- does not get stuck in a loop.
 *
 * It also guarantees that if the lookup returns an element it is the 'correct'
 * one. But not returning an element does _NOT_ mean it's not present.
 *
 * NOTE:
 *
 * Stores to __rb_parent_color are not important for simple lookups so those
 * are left undone as of now. Nor did I check for loops involving parent
 * pointers.
 */

static inline void ja_rb_set_black(struct ja_rb_node *rb)
{
	rb->_ja_rb_parent_color |= JA_RB_BLACK;
}

static inline struct ja_rb_node *ja_rb_red_parent(struct ja_rb_node *red)
{
	return (struct ja_rb_node *)red->_ja_rb_parent_color;
}

/*
 * Helper function for rotations:
 * - old's parent and color get assigned to newer
 * - old gets assigned newer as a parent and 'color' as a color.
 */
static inline void
__ja_rb_rotate_set_parents(struct ja_rb_node *old, struct ja_rb_node *newer,
			struct ja_rb_root *root, int color)
{
	struct ja_rb_node *parent = ja_rb_parent(old);
	newer->_ja_rb_parent_color = old->_ja_rb_parent_color;
	ja_rb_set_parent_color(old, newer, color);
	__ja_rb_change_child(old, newer, parent, root);
}

static __always_inline void
__ja_rb_insert(struct ja_rb_node *node, struct ja_rb_root *root,
	    void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer))
{
	struct ja_rb_node *parent = ja_rb_red_parent(node);
	struct ja_rb_node *gparent;
	struct ja_rb_node *tmp;

	while (true) {
		/*
		 * Loop invariant: node is red.
		 */
		if (unlikely(!parent)) {
			/*
			 * The inserted node is root. Either this is the
			 * first node, or we recursed at Case 1 below and
			 * are no longer violating 4).
			 */
			ja_rb_set_parent_color(node, NULL, JA_RB_BLACK);
			break;
		}

		/*
		 * If there is a black parent, we are done.
		 * Otherwise, take some corrective action as,
		 * per 4), we don't want a red root or two
		 * consecutive red nodes.
		 */
		if(ja_rb_is_black(parent))
			break;

		gparent = ja_rb_red_parent(parent);

		tmp = gparent->ja_rb_right;
		if (parent != tmp) {	/* parent == gparent->rb_left */
			if (tmp && ja_rb_is_red(tmp)) {
				/*
				 * Case 1 - node's uncle is red (color flips).
				 *
				 *       G            g
				 *      / \          / \
				 *     p   u  -->   P   U
				 *    /            /
				 *   n            n
				 *
				 * However, since g's parent might be red, and
				 * 4) does not allow this, we need to recurse
				 * at g.
				 */
				ja_rb_set_parent_color(tmp, gparent, JA_RB_BLACK);
				ja_rb_set_parent_color(parent, gparent, JA_RB_BLACK);
				node = gparent;
				parent = ja_rb_parent(node);
				ja_rb_set_parent_color(node, parent, JA_RB_RED);
				continue;
			}

			tmp = parent->ja_rb_right;
			if (node == tmp) {
				/*
				 * Case 2 - node's uncle is black and node is
				 * the parent's right child (left rotate at parent).
				 *
				 *      G             G
				 *     / \           / \
				 *    p   U  -->    n   U
				 *     \           /
				 *      n         p
				 *
				 * This still leaves us in violation of 4), the
				 * continuation into Case 3 will fix that.
				 */
				tmp = node->ja_rb_left;
				JA_WRITE_ONCE(parent->ja_rb_right, tmp);
				JA_WRITE_ONCE(node->ja_rb_left, parent);
				if (tmp)
					ja_rb_set_parent_color(tmp, parent,
							    JA_RB_BLACK);
				ja_rb_set_parent_color(parent, node, JA_RB_RED);
				augment_rotate(parent, node);
				parent = node;
				tmp = node->ja_rb_right;
			}

			/*
			 * Case 3 - node's uncle is black and node is
			 * the parent's left child (right rotate at gparent).
			 *
			 *        G           P
			 *       / \         / \
			 *      p   U  -->  n   g
			 *     /                 \
			 *    n                   U
			 */
			JA_WRITE_ONCE(gparent->ja_rb_left, tmp); /* == parent->rb_right */
			JA_WRITE_ONCE(parent->ja_rb_right, gparent);
			if (tmp)
				ja_rb_set_parent_color(tmp, gparent, JA_RB_BLACK);
			__ja_rb_rotate_set_parents(gparent, parent, root, JA_RB_RED);
			augment_rotate(gparent, parent);
			break;
		} else {
			tmp = gparent->ja_rb_left;
			if (tmp && ja_rb_is_red(tmp)) {
				/* Case 1 - color flips */
				ja_rb_set_parent_color(tmp, gparent, JA_RB_BLACK);
				ja_rb_set_parent_color(parent, gparent, JA_RB_BLACK);
				node = gparent;
				parent = ja_rb_parent(node);
				ja_rb_set_parent_color(node, parent, JA_RB_RED);
				continue;
			}

			tmp = parent->ja_rb_left;
			if (node == tmp) {
				/* Case 2 - right rotate at parent */
				tmp = node->ja_rb_right;
				JA_WRITE_ONCE(parent->ja_rb_left, tmp);
				JA_WRITE_ONCE(node->ja_rb_right, parent);
				if (tmp)
					ja_rb_set_parent_color(tmp, parent,
							    JA_RB_BLACK);
				ja_rb_set_parent_color(parent, node, JA_RB_RED);
				augment_rotate(parent, node);
				parent = node;
				tmp = node->ja_rb_left;
			}

			/* Case 3 - left rotate at gparent */
			JA_WRITE_ONCE(gparent->ja_rb_right, tmp); /* == parent->rb_left */
			JA_WRITE_ONCE(parent->ja_rb_left, gparent);
			if (tmp)
				ja_rb_set_parent_color(tmp, gparent, JA_RB_BLACK);
			__ja_rb_rotate_set_parents(gparent, parent, root, JA_RB_RED);
			augment_rotate(gparent, parent);
			break;
		}
	}
}

/*
 * Inline version for ja_rb_erase() use - we want to be able to inline
 * and eliminate the dummy_rotate callback there
 */
static __always_inline void
____ja_rb_erase_color(struct ja_rb_node *parent, struct ja_rb_root *root,
	void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer))
{
	struct ja_rb_node *node = NULL;
	struct ja_rb_node *sibling, *tmp1, *tmp2;

	while (true) {
		/*
		 * Loop invariants:
		 * - node is black (or NULL on first iteration)
		 * - node is not the root (parent is not NULL)
		 * - All leaf paths going through parent and node have a
		 *   black node count that is 1 lower than other leaf paths.
		 */
		sibling = parent->ja_rb_right;
		if (node != sibling) {	/* node == parent->rb_left */
			if (ja_rb_is_red(sibling)) {
				/*
				 * Case 1 - left rotate at parent
				 *
				 *     P               S
				 *    / \             / \
				 *   N   s    -->    p   Sr
				 *      / \         / \
				 *     Sl  Sr      N   Sl
				 */
				tmp1 = sibling->ja_rb_left;
				JA_WRITE_ONCE(parent->ja_rb_right, tmp1);
				JA_WRITE_ONCE(sibling->ja_rb_left, parent);
				ja_rb_set_parent_color(tmp1, parent, JA_RB_BLACK);
				__ja_rb_rotate_set_parents(parent, sibling, root,
							JA_RB_RED);
				augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->ja_rb_right;
			if (!tmp1 || ja_rb_is_black(tmp1)) {
				tmp2 = sibling->ja_rb_left;
				if (!tmp2 || ja_rb_is_black(tmp2)) {
					/*
					 * Case 2 - sibling color flip
					 * (p could be either color here)
					 *
					 *    (p)           (p)
					 *    / \           / \
					 *   N   S    -->  N   s
					 *      / \           / \
					 *     Sl  Sr        Sl  Sr
					 *
					 * This leaves us violating 5) which
					 * can be fixed by flipping p to black
					 * if it was red, or by recursing at p.
					 * p is red when coming from Case 1.
					 */
					ja_rb_set_parent_color(sibling, parent,
							    JA_RB_RED);
					if (ja_rb_is_red(parent))
						ja_rb_set_black(parent);
					else {
						node = parent;
						parent = ja_rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/*
				 * Case 3 - right rotate at sibling
				 * (p could be either color here)
				 *
				 *   (p)           (p)
				 *   / \           / \
				 *  N   S    -->  N   sl
				 *     / \             \
				 *    sl  Sr            S
				 *                       \
				 *                        Sr
				 *
				 * Note: p might be red, and then both
				 * p and sl are red after rotation(which
				 * breaks property 4). This is fixed in
				 * Case 4 (in __rb_rotate_set_parents()
				 *         which set sl the color of p
				 *         and set p JA_RB_BLACK)
				 *
				 *   (p)            (sl)
				 *   / \            /  \
				 *  N   sl   -->   P    S
				 *       \        /      \
				 *        S      N        Sr
				 *         \
				 *          Sr
				 */
				tmp1 = tmp2->ja_rb_right;
				JA_WRITE_ONCE(sibling->ja_rb_left, tmp1);
				JA_WRITE_ONCE(tmp2->ja_rb_right, sibling);
				JA_WRITE_ONCE(parent->ja_rb_right, tmp2);
				if (tmp1)
					ja_rb_set_parent_color(tmp1, sibling,
							    JA_RB_BLACK);
				augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/*
			 * Case 4 - left rotate at parent + color flips
			 * (p and sl could be either color here.
			 *  After rotation, p becomes black, s acquires
			 *  p's color, and sl keeps its color)
			 *
			 *      (p)             (s)
			 *      / \             / \
			 *     N   S     -->   P   Sr
			 *        / \         / \
			 *      (sl) sr      N  (sl)
			 */
			tmp2 = sibling->ja_rb_left;
			JA_WRITE_ONCE(parent->ja_rb_right, tmp2);
			JA_WRITE_ONCE(sibling->ja_rb_left, parent);
			ja_rb_set_parent_color(tmp1, sibling, JA_RB_BLACK);
			if (tmp2)
				ja_rb_set_parent(tmp2, parent);
			__ja_rb_rotate_set_parents(parent, sibling, root,
						JA_RB_BLACK);
			augment_rotate(parent, sibling);
			break;
		} else {
			sibling = parent->ja_rb_left;
			if (ja_rb_is_red(sibling)) {
				/* Case 1 - right rotate at parent */
				tmp1 = sibling->ja_rb_right;
				JA_WRITE_ONCE(parent->ja_rb_left, tmp1);
				JA_WRITE_ONCE(sibling->ja_rb_right, parent);
				ja_rb_set_parent_color(tmp1, parent, JA_RB_BLACK);
				__ja_rb_rotate_set_parents(parent, sibling, root,
							JA_RB_RED);
				augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->ja_rb_left;
			if (!tmp1 || ja_rb_is_black(tmp1)) {
				tmp2 = sibling->ja_rb_right;
				if (!tmp2 || ja_rb_is_black(tmp2)) {
					/* Case 2 - sibling color flip */
					ja_rb_set_parent_color(sibling, parent,
							    JA_RB_RED);
					if (ja_rb_is_red(parent))
						ja_rb_set_black(parent);
					else {
						node = parent;
						parent = ja_rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/* Case 3 - left rotate at sibling */
				tmp1 = tmp2->ja_rb_left;
				JA_WRITE_ONCE(sibling->ja_rb_right, tmp1);
				JA_WRITE_ONCE(tmp2->ja_rb_left, sibling);
				JA_WRITE_ONCE(parent->ja_rb_left, tmp2);
				if (tmp1)
					ja_rb_set_parent_color(tmp1, sibling,
							    JA_RB_BLACK);
				augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/* Case 4 - right rotate at parent + color flips */
			tmp2 = sibling->ja_rb_right;
			JA_WRITE_ONCE(parent->ja_rb_left, tmp2);
			JA_WRITE_ONCE(sibling->ja_rb_right, parent);
			ja_rb_set_parent_color(tmp1, sibling, JA_RB_BLACK);
			if (tmp2)
				ja_rb_set_parent(tmp2, parent);
			__ja_rb_rotate_set_parents(parent, sibling, root,
						JA_RB_BLACK);
			augment_rotate(parent, sibling);
			break;
		}
	}
}

/* Non-inline version for rb_erase_augmented() use */
void __ja_rb_erase_color(struct ja_rb_node *parent, struct ja_rb_root *root,
	void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer))
{
	____ja_rb_erase_color(parent, root, augment_rotate);
}
//EXPORT_SYMBOL(__ja_rb_erase_color);

/*
 * Non-augmented rbtree manipulation functions.
 *
 * We use dummy augmented callbacks here, and have the compiler optimize them
 * out of the rb_insert_color() and rb_erase() function definitions.
 */

static inline void ja_dummy_propagate(struct ja_rb_node *node, struct ja_rb_node *stop) {}
static inline void ja_dummy_copy(struct ja_rb_node *old, struct ja_rb_node *newer) {}
static inline void ja_dummy_rotate(struct ja_rb_node *old, struct ja_rb_node *newer) {}

static const struct ja_rb_augment_callbacks ja_dummy_callbacks = {
	.propagate = ja_dummy_propagate,
	.copy = ja_dummy_copy,
	.rotate = ja_dummy_rotate
};

void ja_rb_insert_color(struct ja_rb_node *node, struct ja_rb_root *root)
{
	__ja_rb_insert(node, root, ja_dummy_rotate);
}
//EXPORT_SYMBOL(ja_rb_insert_color);

void ja_rb_erase(struct ja_rb_node *node, struct ja_rb_root *root)
{
	struct ja_rb_node *rebalance;
	rebalance = __ja_rb_erase_augmented(node, root, &ja_dummy_callbacks);
	if (rebalance)
		____ja_rb_erase_color(rebalance, root, ja_dummy_rotate);
}
//EXPORT_SYMBOL(ja_rb_erase);

/*
 * Augmented rbtree manipulation functions.
 *
 * This instantiates the same __always_inline functions as in the non-augmented
 * case, but this time with user-defined callbacks.
 */

void __ja_rb_insert_augmented(struct ja_rb_node *node, struct ja_rb_root *root,
	void (*augment_rotate)(struct ja_rb_node *old, struct ja_rb_node *newer))
{
	__ja_rb_insert(node, root, augment_rotate);
}
//EXPORT_SYMBOL(__ja_rb_insert_augmented);

/*
 * This function returns the first node (in sort order) of the tree.
 */
struct ja_rb_node *ja_rb_first(const struct ja_rb_root *root)
{
	struct ja_rb_node	*n;

	n = root->rb_node;
	if (!n)
		return NULL;
	while (n->ja_rb_left)
		n = n->ja_rb_left;
	return n;
}
//EXPORT_SYMBOL(ja_rb_first);

struct ja_rb_node *ja_rb_last(const struct ja_rb_root *root)
{
	struct ja_rb_node	*n;

	n = root->rb_node;
	if (!n)
		return NULL;
	while (n->ja_rb_right)
		n = n->ja_rb_right;
	return n;
}
//EXPORT_SYMBOL(ja_rb_last);

struct ja_rb_node *ja_rb_next(const struct ja_rb_node *node)
{
	struct ja_rb_node *parent;

	if (JA_RB_EMPTY_NODE(node))
		return NULL;

	/*
	 * If we have a right-hand child, go down and then left as far
	 * as we can.
	 */
	if (node->ja_rb_right) {
		node = node->ja_rb_right;
		while (node->ja_rb_left)
			node = node->ja_rb_left;
		return (struct ja_rb_node *)node;
	}

	/*
	 * No right-hand children. Everything down and left is smaller than us,
	 * so any 'next' node must be in the general direction of our parent.
	 * Go up the tree; any time the ancestor is a right-hand child of its
	 * parent, keep going up. First time it's a left-hand child of its
	 * parent, said parent is our 'next' node.
	 */
	while ((parent = ja_rb_parent(node)) && node == parent->ja_rb_right)
		node = parent;

	return parent;
}
//EXPORT_SYMBOL(ja_rb_next);

struct ja_rb_node *ja_rb_prev(const struct ja_rb_node *node)
{
	struct ja_rb_node *parent;

	if (JA_RB_EMPTY_NODE(node))
		return NULL;

	/*
	 * If we have a left-hand child, go down and then right as far
	 * as we can.
	 */
	if (node->ja_rb_left) {
		node = node->ja_rb_left;
		while (node->ja_rb_right)
			node = node->ja_rb_right;
		return (struct ja_rb_node *)node;
	}

	/*
	 * No left-hand children. Go up till we find an ancestor which
	 * is a right-hand child of its parent.
	 */
	while ((parent = ja_rb_parent(node)) && node == parent->ja_rb_left)
		node = parent;

	return parent;
}
//EXPORT_SYMBOL(ja_rb_prev);

void ja_rb_replace_node(struct ja_rb_node *victim, struct ja_rb_node *newer,
		     struct ja_rb_root *root)
{
	struct ja_rb_node *parent = ja_rb_parent(victim);

	/* Copy the pointers/colour from the victim to the replacement */
	*newer = *victim;

	/* Set the surrounding nodes to point to the replacement */
	if (victim->ja_rb_left)
		ja_rb_set_parent(victim->ja_rb_left, newer);
	if (victim->ja_rb_right)
		ja_rb_set_parent(victim->ja_rb_right, newer);
	__ja_rb_change_child(victim, newer, parent, root);
}
//EXPORT_SYMBOL(ja_rb_replace_node);

#if 0
void ja_rb_replace_node_rcu(struct ja_rb_node *victim, struct ja_rb_node *newer,
			 struct ja_rb_root *root)
{
	struct ja_rb_node *parent = ja_rb_parent(victim);

	/* Copy the pointers and colour from the victim to the replacement */
	/* ½»È¿¶ù */
	*newer = *victim; // size of this time copy is 3 multi (unsigned long)

	/* Set the surrounding nodes to point to the replacement */
	/* ¸ø×óÓÒ×Ó»»µù */
	if (victim->ja_rb_left)
		ja_rb_set_parent(victim->ja_rb_left, newer);
	if (victim->ja_rb_right)
		ja_rb_set_parent(victim->ja_rb_right, newer);

	/* Set the parent's pointer to the newer node last after an RCU barrier
	 * so that the pointers onwards are seen to be set correctly when doing
	 * an RCU walk over the tree.
	 */
	/* ¸øµù»»¶ù */
	__ja_rb_change_child_rcu(victim, newer, parent, root);
}
//EXPORT_SYMBOL(ja_rb_replace_node_rcu);
#endif

static struct ja_rb_node *ja_rb_left_deepest_node(const struct ja_rb_node *node)
{
	for (;;) {
		if (node->ja_rb_left)
			node = node->ja_rb_left;
		else if (node->ja_rb_right)
			node = node->ja_rb_right;
		else
			return (struct ja_rb_node *)node;
	}
}

struct ja_rb_node *ja_rb_next_postorder(const struct ja_rb_node *node)
{
	/* postorder : left, right, parent. */
	const struct ja_rb_node *parent;
	if (!node)
		return NULL;
	parent = ja_rb_parent(node);

	/* If we're sitting on node, we've already seen our children */
	if (parent && node == parent->ja_rb_left && parent->ja_rb_right) {
		/* If we are the parent's left node, go to the parent's right
		 * node then all the way down to the left */
		return ja_rb_left_deepest_node(parent->ja_rb_right);
	} else
		/* Otherwise we are the parent's right node, and the parent
		 * should be next */
		return (struct ja_rb_node *)parent;
}
//EXPORT_SYMBOL(ja_rb_next_postorder);

struct ja_rb_node *ja_rb_first_postorder(const struct ja_rb_root *root)
{
	/* postorder: left, right, parent */
	if (!root->rb_node)
		return NULL;

	return ja_rb_left_deepest_node(root->rb_node);
}
//EXPORT_SYMBOL(ja_rb_first_postorder);

