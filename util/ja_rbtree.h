#ifndef __JA_RBTREE_H_
#define __JA_RBTREE_H_
/*
#include <linux/kernel.h>
#include <linux/stddef.h>
*/
#undef NULL
//#define NULL ((void *)0)
#define NULL 0

#ifndef __always_inline
# define __always_inline	inline __attribute__((always_inline))
#endif

/* clean code */
struct ja_rb_node {
	unsigned long  _ja_rb_parent_color;
	struct ja_rb_node *ja_rb_right;
	struct ja_rb_node *ja_rb_left;
} __attribute__((aligned(sizeof(unsigned long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct ja_rb_root {
	struct ja_rb_node *rb_node;
};

#define ja_rb_parent(r)   ((struct ja_rb_node *)((r)->_ja_rb_parent_color & ~3))

#define JA_RB_ROOT	(struct ja_rb_root) { NULL, }
#define	ja_rb_entry(ptr, type, member) ja_container_of(ptr, type, member)

#define JA_RB_EMPTY_ROOT(root)  (JA_READ_ONCE((root)->ja_rb_node) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define JA_RB_EMPTY_NODE(node)  \
	((node)->_ja_rb_parent_color == (unsigned long)(node))
#define JA_RB_CLEAR_NODE(node)  \
	((node)->_ja_rb_parent_color = (unsigned long)(node))


extern void ja_rb_insert_color(struct ja_rb_node *, struct ja_rb_root *);
extern void ja_rb_erase(struct ja_rb_node *, struct ja_rb_root *);


/* Find logical next and previous nodes in a tree */
extern struct ja_rb_node *ja_rb_next(const struct ja_rb_node *);
extern struct ja_rb_node *ja_rb_prev(const struct ja_rb_node *);
extern struct ja_rb_node *ja_rb_first(const struct ja_rb_root *);
extern struct ja_rb_node *ja_rb_last(const struct ja_rb_root *);

/* Postorder iteration - always visit the parent after its children */
extern struct ja_rb_node *ja_rb_first_postorder(const struct ja_rb_root *);
extern struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void ja_rb_replace_node(struct ja_rb_node *victim, struct ja_rb_node *newer,
			    struct ja_rb_root *root);

static inline void ja_rb_link_node(struct ja_rb_node *node, struct ja_rb_node *parent,
				struct ja_rb_node **rb_link)
{
	node->_ja_rb_parent_color = (unsigned long)parent;
	node->ja_rb_left = node->ja_rb_right = NULL;

	*rb_link = node;
}

#define ja_rb_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? ja_rb_entry(____ptr, type, member) : NULL; \
	})

/**
 * ja_rbtree_postorder_for_each_entry_safe - iterate in post-order over rb_root of
 * given type allowing the backing memory of @pos to be invalidated
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @n:		another 'type *' to use as temporary storage
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 *
 * ja_rbtree_postorder_for_each_entry_safe() provides a similar guarantee as
 * ja_list_for_each_entry_safe() and allows the iteration to continue independent
 * of changes to @pos by the body of the loop.
 *
 * Note, however, that it cannot handle other modifications that re-order the
 * rbtree it is iterating over. This includes calling ja_rb_erase() on @pos, as
 * rb_erase() may rebalance the tree, causing us to miss some nodes.
 */
#define ja_rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
	for (pos = ja_rb_entry_safe(ja_rb_first_postorder(root), typeof(*pos), field); \
	     pos && ({ n = ja_rb_entry_safe(ja_rb_next_postorder(&pos->field), \
			typeof(*pos), field); 1; }); \
	     pos = n)

static inline void ja_rb_erase_init(struct ja_rb_node *n, struct ja_rb_root *root)
{
	ja_rb_erase(n, root);
	JA_RB_CLEAR_NODE(n);
}

/*
 * Leftmost-cached rbtrees.
 *
 * We do not cache the rightmost node based on footprint
 * size vs number of potential users that could benefit
 * from O(1) rb_last(). Just not worth it, users that want
 * this feature can always implement the logic explicitly.
 * Furthermore, users that want to cache both pointers may
 * find it a bit asymmetric, but that's ok.
 * 最左边缓存的红黑树
 * 我们不会缓存右边的节点，基于根打印的尺寸对比潜在用户从rb_last的收益
 * 仅仅是不值得，用户可以明确的实现这个特性。
 * 另外，用户想要存储两个指针，会发现他有点不对称。但是那没关系
 */
struct ja_rb_root_cached {
	struct ja_rb_root ja_rb_root;
	struct ja_rb_node *ja_rb_leftmost;
};

#define JA_RB_ROOT_CACHED (struct ja_rb_root_cached) { {NULL, }, NULL }

/* Same as rb_first(), but O(1) */
#define ja_rb_first_cached(root) (root)->ja_rb_leftmost

static inline void ja_rb_insert_color_cached(struct ja_rb_node *node,
					  struct ja_rb_root_cached *root,
					  bool leftmost)
{
	if (leftmost)
		root->ja_rb_leftmost = node;
	ja_rb_insert_color(node, &root->ja_rb_root);
}

static inline void ja_rb_erase_cached(struct ja_rb_node *node,
				   struct ja_rb_root_cached *root)
{
	if (root->ja_rb_leftmost == node)
		root->ja_rb_leftmost = ja_rb_next(node);
	ja_rb_erase(node, &root->ja_rb_root);
}

static inline void ja_rb_replace_node_cached(struct ja_rb_node *victim,
					  struct ja_rb_node *newer,
					  struct ja_rb_root_cached *root)
{
	if (root->ja_rb_leftmost == victim)
		root->ja_rb_leftmost = newer;
	ja_rb_replace_node(victim, newer, &root->ja_rb_root);
}

/*
 * The below helper functions use 2 operators with 3 different
 * calling conventions. The operators are related like:
 *
 *	comp(a->key,b) < 0  := less(a,b)
 *	comp(a->key,b) > 0  := less(b,a)
 *	comp(a->key,b) == 0 := !less(a,b) && !less(b,a)
 *
 * If these operators define a partial order on the elements we make no
 * guarantee on which of the elements matching the key is found. See
 * rb_find().
 *
 * The reason for this is to allow the find() interface without requiring an
 * on-stack dummy object, which might not be feasible due to object size.
 */

/**
 * ja_rb_add_cached() - insert @node into the leftmost cached tree @tree
 * @node: node to insert
 * @tree: leftmost cached tree to insert @node into
 * @less: operator defining the (partial) node order
 */
static __always_inline void
ja_rb_add_cached(struct ja_rb_node *node, struct ja_rb_root_cached *tree,
	      bool (*less)(struct ja_rb_node *, const struct ja_rb_node *))
{
	struct ja_rb_node **link = &tree->ja_rb_root.rb_node;
	struct ja_rb_node *parent = NULL;
	bool leftmost = true;

	while (*link) {
		parent = *link;
		if (less(node, parent)) {
			link = &parent->ja_rb_left;
		} else {
			link = &parent->ja_rb_right;
			leftmost = false;
		}
	}

	ja_rb_link_node(node, parent, link);
	ja_rb_insert_color_cached(node, tree, leftmost);
}

/**
 * rb_add() - insert @node into @tree
 * @node: node to insert
 * @tree: tree to insert @node into
 * @less: operator defining the (partial) node order
 */
static __always_inline void
ja_rb_add(struct ja_rb_node *node, struct ja_rb_root *tree,
       bool (*less)(struct ja_rb_node *, const struct ja_rb_node *))
{
	struct ja_rb_node **link = &tree->rb_node;
	struct ja_rb_node *parent = NULL;

	while (*link) {
		parent = *link;
		if (less(node, parent))
			link = &parent->ja_rb_left;
		else
			link = &parent->ja_rb_right;
	}

	ja_rb_link_node(node, parent, link);
	ja_rb_insert_color(node, tree);
}

/**
 * ja_rb_find_add() - find equivalent @node in @tree, or add @node
 * @node: node to look-for / insert
 * @tree: tree to search / modify
 * @cmp: operator defining the node order
 *
 * Returns the rb_node matching @node, or NULL when no match is found and @node
 * is inserted.
 */
static __always_inline struct ja_rb_node *
ja_rb_find_add(struct ja_rb_node *node, struct ja_rb_root *tree,
	    int (*cmp)(struct ja_rb_node *, const struct ja_rb_node *))
{
	struct ja_rb_node **link = &tree->rb_node;
	struct ja_rb_node *parent = NULL;
	int c;

	while (*link) {
		parent = *link;
		c = cmp(node, parent);

		if (c < 0)
			link = &parent->ja_rb_left;
		else if (c > 0)
			link = &parent->ja_rb_right;
		else
			return parent;
	}

	ja_rb_link_node(node, parent, link);
	ja_rb_insert_color(node, tree);
	return NULL;
}

/**
 * ja_rb_find() - find @key in tree @tree
 * @key: key to match
 * @tree: tree to search
 * @cmp: operator defining the node order
 *
 * Returns the rb_node matching @key or NULL.
 */
static __always_inline struct ja_rb_node *
ja_rb_find(const void *key, const struct ja_rb_root *tree,
	int (*cmp)(const void *key, const struct ja_rb_node *))
{
	struct ja_rb_node *node = tree->rb_node;

	while (node) {
		int c = cmp(key, node);

		if (c < 0)
			node = node->ja_rb_left;
		else if (c > 0)
			node = node->ja_rb_right;
		else
			return node;
	}

	return NULL;
}

/**
 * ja_rb_find_first() - find the first @key in @tree
 * @key: key to match
 * @tree: tree to search
 * @cmp: operator defining node order
 *
 * Returns the leftmost node matching @key, or NULL.
 */
static __always_inline struct ja_rb_node *
ja_rb_find_first(const void *key, const struct ja_rb_root *tree,
	      int (*cmp)(const void *key, const struct ja_rb_node *))
{
	struct ja_rb_node *node = tree->rb_node;
	struct ja_rb_node *match = NULL;

	while (node) {
		int c = cmp(key, node);

		if (c <= 0) {
			if (!c)
				match = node;
			node = node->ja_rb_left;
		} else if (c > 0) {
			node = node->ja_rb_right;
		}
	}

	return match;
}

/**
 * ja_rb_next_match() - find the next @key in @tree
 * @key: key to match
 * @tree: tree to search
 * @cmp: operator defining node order
 *
 * Returns the next node matching @key, or NULL.
 */
static __always_inline struct ja_rb_node *
ja_rb_next_match(const void *key, struct ja_rb_node *node,
	      int (*cmp)(const void *key, const struct ja_rb_node *))
{
	node = ja_rb_next(node);
	if (node && cmp(key, node))
		node = NULL;
	return node;
}

/**
 * ja_rb_for_each() - iterates a subtree matching @key
 * @node: iterator
 * @key: key to match
 * @tree: tree to search
 * @cmp: operator defining node order
 */
#define ja_rb_for_each(node, key, tree, cmp) \
	for ((node) = ja_rb_find_first((key), (tree), (cmp)); \
	     (node); (node) = ja_rb_next_match((key), (node), (cmp)))

#endif	/* __TOOLS_LINUX_PERF_RBTREE_H */

