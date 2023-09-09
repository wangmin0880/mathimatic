
#ifndef __JA_LIST_H__
#define __JA_LIST_H__

/**
 * double linked list
 *    [ ]-->
 * <--[ ]
 */
struct ja_list_head {
	struct ja_list_head *next, *prev;
};

/**
 * hash table list head
 *
 * [ ]-->
 */
struct ja_hlist_head {
	struct ja_hlist_node *first;
};

/**
 * hash table list node
 *
 *    [ ]-->
 * <--[ ]
 */
struct ja_hlist_node {
	struct ja_hlist_node *next, **pprev;
};


#undef ja_offsetof
#define ja_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * ja_container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define ja_container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - ja_offsetof(type,member) );})


#define JA_READ_ONCE(x) (*(volatile typeof(x) *) &(x))

#define __JA_WRITE_ONCE(x, val)						\
do {									\
	*(volatile typeof(x) *)&(x) = (val);				\
} while (0)

#define JA_WRITE_ONCE(x, val)						\
do {									\
	__JA_WRITE_ONCE(x, val);						\
} while (0)

/*
 * Circular doubly linked list implementation.
 */

#define JA_LIST_HEAD_INIT(name) { &(name), &(name) }

#define JA_LIST_HEAD(name) \
	struct ja_list_head name = JA_LIST_HEAD_INIT(name)

/**
 * JA_INIT_LIST_HEAD - Initialize a ja_list_head structure
 * @list: ja_list_head structure to be initialized.
 *
 * Initializes the ja_list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static inline void JA_INIT_LIST_HEAD(struct ja_list_head *list)
{
	JA_WRITE_ONCE(list->next, list);
	JA_WRITE_ONCE(list->prev, list);
}

static inline bool __ja_list_add_valid(struct ja_list_head *newr,
				struct ja_list_head *prev,
				struct ja_list_head *next)
{
	return true;
}
/*
static inline bool __ja_list_del_entry_valid(struct ja_list_head *entry)
{
	return true;
}
*/

/*
 * Insert a newr entry between two known consecutive entries.
 */
static inline void __ja_list_add(struct ja_list_head *newr,
			      struct ja_list_head *prev,
			      struct ja_list_head *next)
{
	/*
	if (!__ja_list_add_valid(newr, prev, next))
		return;
  */
	next->prev = newr;

	newr->next = next;
	newr->prev = prev;

	JA_WRITE_ONCE(prev->next, newr);
}

/**
 * ja_list_add - add a newr entry
 * @newr: newr entry to be added
 * @head: list head to add it after
 * ����ͷ�ĺ���
 */
static inline void ja_list_add(struct ja_list_head *newr, struct ja_list_head *head)
{
	__ja_list_add(newr, head, head->next);
}


/**
 * ja_list_add_tail - add a newr entry
 * @newr: newr entry to be added
 * @tail: list head to add it before
 *
 * Insert a newr entry before the specified tail.
 * ����β��ǰ��
 */
static inline void ja_list_add_tail(struct ja_list_head *newr, struct ja_list_head *tail)
{
	__ja_list_add(newr, tail->prev, tail);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 * �е� ��һ�� �ڵ����棬���ϵڶ����ڵ�
 */
static inline void __ja_list_del(struct ja_list_head * prev, struct ja_list_head * next)
{
	next->prev = prev;
	JA_WRITE_ONCE(prev->next, next);
}

/*
 * Delete a list entry and clear the 'prev' pointer.
 *
 */
static inline void __ja_list_del_clearprev(struct ja_list_head *entry)
{
	__ja_list_del(entry->prev, entry->next);
	entry->prev = NULL;
}

static inline void __ja_list_del_entry(struct ja_list_head *entry)
{
	/*if (!__ja_list_del_entry_valid(entry))
		return;
  */
	__ja_list_del(entry->prev, entry->next);
}

/**
 * ja_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#define JA_LIST_POISION_POINTER_DELTA 0
#define JA_LIST_POISON1 ((void *)(0x100 + JA_LIST_POISION_POINTER_DELTA))
#define JA_LIST_POISON2 ((void *)(0x122 + JA_LIST_POISION_POINTER_DELTA))
static inline void ja_list_del(struct ja_list_head *entry)
{
	__ja_list_del_entry(entry);
	entry->next = (ja_list_head *)JA_LIST_POISON1;
	entry->prev = (ja_list_head *)JA_LIST_POISON2;
}

/**
 * ja_list_replace - replace old entry by newr one
 * @old : the element to be replaced
 * @newr : the newr element to insert
 *
 */
static inline void ja_list_replace(struct ja_list_head *old,
				struct ja_list_head *newr)
{
	newr->next = old->next;
	newr->next->prev = newr;
	newr->prev = old->prev;
	newr->prev->next = newr;
}

/**
 * ja_list_replace_init - replace old entry by newr one and initialize the old one
 * @old : the element to be replaced
 * @newr : the newr element to insert
 *
 */
static inline void ja_list_replace_init(struct ja_list_head *old,
				     struct ja_list_head *newr)
{
	ja_list_replace(old, newr);
	JA_INIT_LIST_HEAD(old);
}

/**
 * ja_list_swap - replace entry1 with entry2 and re-add entry1 at entry2's position
 * @entry1: the location to place entry2
 * @entry2: the location to place entry1
 */
static inline void ja_list_swap(struct ja_list_head *entry1,
			     struct ja_list_head *entry2)
{
	struct ja_list_head *pos = entry2->prev;

	ja_list_del(entry2);
	ja_list_replace(entry1, entry2);
	if (pos == entry1)
		pos = entry2;
	ja_list_add(entry1, pos);
}

/**
 * ja_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void ja_list_del_init(struct ja_list_head *entry)
{
	__ja_list_del_entry(entry);
	JA_INIT_LIST_HEAD(entry);
}

/**
 * ja_list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void ja_list_move(struct ja_list_head *list, struct ja_list_head *head)
{
	__ja_list_del_entry(list);
	ja_list_add(list, head);
}

/**
 * ja_list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 * ��β�͵�ǰһ����
 */
static inline void ja_list_move_tail(struct ja_list_head *list,
				  struct ja_list_head *tail)
{
	__ja_list_del_entry(list);
	ja_list_add_tail(list, tail);
}

/**
 * ja_list_bulk_move_tail - move a subsection of a list to its tail
 * @head: the head that will follow our entry
 * @first: first entry to move
 * @last: last entry to move, can be the same as first
 *
 * Move all entries between @first and including @last before @head.
 * All three entries must belong to the same linked list.
 */
static inline void ja_list_bulk_move_tail(struct ja_list_head *head,
				       struct ja_list_head *first,
				       struct ja_list_head *last)
{
	first->prev->next = last->next;
	last->next->prev = first->prev;

	head->prev->next = first;
	first->prev = head->prev;

	last->next = head;
	head->prev = last;
}

/**
 * ja_list_is_first -- tests whether @list is the first entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int ja_list_is_first(const struct ja_list_head *list, const struct ja_list_head *head)
{
	return list->prev == head;
}

/**
 * ja_list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int ja_list_is_last(const struct ja_list_head *list, const struct ja_list_head *head)
{
	return list->next == head;
}

/**
 * ja_list_is_head - tests whether @list is the list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int ja_list_is_head(const struct ja_list_head *list, const struct ja_list_head *head)
{
	return list == head;
}

/**
 * ja_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int ja_list_empty(const struct ja_list_head *head)
{
	return JA_READ_ONCE(head->next) == head;
}

#if 0
/**
 * ja_list_del_init_careful - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 *
 * This is the same as list_del_init(), except designed to be used
 * together with list_empty_careful() in a way to guarantee ordering
 * of other memory operations.
 *
 * Any memory operations done before a list_del_init_careful() are
 * guaranteed to be visible after a list_empty_careful() test.
 */
static inline void ja_list_del_init_careful(struct ja_list_head *entry)
{
	__ja_list_del_entry(entry);
	JA_WRITE_ONCE(entry->prev, entry);
	smp_store_release(&entry->next, entry);
}

/**
 * ja_list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using ja_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int ja_list_empty_careful(const struct ja_list_head *head)
{
	struct ja_list_head *next = smp_load_acquire(&head->next);
	return ja_list_is_head(next, head) && (next == READ_ONCE(head->prev));
}
#endif

/**
 * ja_list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static inline void ja_list_rotate_left(struct ja_list_head *head)
{
	struct ja_list_head *first;

	if (!ja_list_empty(head)) {
		first = head->next;
		ja_list_move_tail(first, head);
	}
}

/**
 * ja_list_rotate_to_front() - Rotate list to specific item.
 * @list: The desired newr front of the list.
 * @head: The head of the list.
 *
 * Rotates list so that @list becomes the newr front of the list.
 */
static inline void ja_list_rotate_to_front(struct ja_list_head *list,
					struct ja_list_head *head)
{
	/*
	 * Deletes the list head from the list denoted by @head and
	 * places it as the tail of @list, this effectively rotates the
	 * list so that @list is at the front.
	 */
	ja_list_move_tail(head, list);
}

/**
 * ja_list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int ja_list_is_singular(const struct ja_list_head *head)
{
	return !ja_list_empty(head) && (head->next == head->prev);
}

static inline void __ja_list_cut_position(struct ja_list_head *list,
		struct ja_list_head *head, struct ja_list_head *entry)
{
	struct ja_list_head *newr_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = newr_first;
	newr_first->prev = head;
}

/**
 * ja_list_cut_position - cut a list into two
 * @list: a newr list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 * position - ����һ�� ,����entry������
 */
static inline void ja_list_cut_position(struct ja_list_head *list,
		struct ja_list_head *head, struct ja_list_head *entry)
{
	if (ja_list_empty(head))
		return;
	if (ja_list_is_singular(head) && !ja_list_is_head(entry, head) && (entry != head->next))
		return;
	if (ja_list_is_head(entry, head))
		JA_INIT_LIST_HEAD(list);
	else
		__ja_list_cut_position(list, head, entry);
}

/**
 * ja_list_cut_before - cut a list into two, before given entry
 * @list: a newr list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *
 * This helper moves the initial part of @head, up to but
 * excluding @entry, from @head to @list.  You should pass
 * in @entry an element you know is on @head.  @list should
 * be an empty list or a list you do not care about losing
 * its data.
 * If @entry == @head, all entries on @head are moved to
 * @list.
 *
 * ���� head ֮���� entry ֮ǰ�Ľڵ㣬��list �ϡ�
 * Head ֮��ֱ�� ����entry.
 */
static inline void ja_list_cut_before(struct ja_list_head *list,
				   struct ja_list_head *head,
				   struct ja_list_head *entry)
{
	if (head->next == entry) {
		JA_INIT_LIST_HEAD(list);
		return;
	}
	list->next = head->next;
	list->next->prev = list;

	/* cut before :-)*/
	list->prev = entry->prev;
	list->prev->next = list;

	head->next = entry;
	entry->prev = head;
}

static inline void __ja_list_splice(const struct ja_list_head *list,
				 struct ja_list_head *prev,
				 struct ja_list_head *next)
{
	/* ��һ��������list �ڵ㡣��list ����һ���ڵ�ม�*/
	struct ja_list_head *first = list->next;
	struct ja_list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * ja_list_splice - join two lists, this is designed for stacks
 * @list: the newr list to add.
 * @head: the place to add it in the first list.
 *
 * �µ�˫����ճ���� ����������head ��head->next ֮��
 */
static inline void ja_list_splice(const struct ja_list_head *list,
				struct ja_list_head *head)
{
	if (!ja_list_empty(list))
		__ja_list_splice(list, head, head->next);
}

/**
 * ja_list_splice_tail - join two lists, each list being a queue
 * @list: the newr list to add.
 * @head: the place to add it in the first list.
 *
 * �µ�˫����ճ���� ����������head λ��ǰ�� */
static inline void ja_list_splice_tail(struct ja_list_head *list,
				struct ja_list_head *head)
{
	if (!ja_list_empty(list))
		__ja_list_splice(list, head->prev, head);
}

/**
 * ja_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the newr list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 *
 * ������һ���ڵ��� list����ȥ��һ���ڵ�֮�������нڵ㣬 �嵽head ��head->next ֮�䣬
 * ͬʱ��ʼ��һ��list�����׽ڵ�
 */
static inline void ja_list_splice_init(struct ja_list_head *list,
				    struct ja_list_head *head)
{
	if (!ja_list_empty(list)) {
		__ja_list_splice(list, head, head->next);
		JA_INIT_LIST_HEAD(list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the newr list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void ja_list_splice_tail_init(struct ja_list_head *list,
					 struct ja_list_head *head)
{
	if (!ja_list_empty(list)) {
		__ja_list_splice(list, head->prev, head);
		JA_INIT_LIST_HEAD(list);
	}
}


#define JA_LIST_EXPLAIN_PART1 0x1
/**
 * �������Ĳ��֣����������� ��entry �Ͳ���entry
 * list_for_each :������pos �� head ����list_head ָ��
 * list_for_each_entry :������pos �Ǵ��ṹ��ָ�롣head �� list_head ָ��
 */

/**
 * ja_list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define ja_list_entry(ptr, type, member) \
	ja_container_of(ptr, type, member)

/**
 * ja_list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define ja_list_first_entry(ptr, type, member) \
	ja_list_entry((ptr)->next, type, member)

/**
 * ja_list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define ja_list_last_entry(ptr, type, member) \
	ja_list_entry((ptr)->prev, type, member)

/**
 * ja_list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define ja_list_first_entry_or_null(ptr, type, member) ({ \
	struct ja_list_head *head__ = (ptr); \
	struct ja_list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? ja_list_entry(pos__, type, member) : NULL; \
})

/**
 * ja_list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define ja_list_next_entry(pos, member) \
	ja_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * ja_list_next_entry_circular - get the next element in list
 * @pos:	the type * to cursor.
 * @head:	the list head to take the element from.
 * @member:	the name of the list_head within the struct.
 *
 * Wraparound if pos is the last element (return the first element).
 * Note, that list is expected to be not empty.
 */
#define ja_list_next_entry_circular(pos, head, member) \
	(ja_list_is_last(&(pos)->member, head) ? \
	ja_list_first_entry(head, typeof(*(pos)), member) : ja_list_next_entry(pos, member))

/**
 * ja_list_prev_entry - get the prev element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define ja_list_prev_entry(pos, member) \
	ja_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * ja_list_prev_entry_circular - get the prev element in list
 * @pos:	the type * to cursor.
 * @head:	the list head to take the element from.
 * @member:	the name of the list_head within the struct.
 *
 * Wraparound if pos is the first element (return the last element).
 * Note, that list is expected to be not empty.
 */
#define ja_list_prev_entry_circular(pos, head, member) \
	(ja_list_is_first(&(pos)->member, head) ? \
	ja_list_last_entry(head, typeof(*(pos)), member) : ja_list_prev_entry(pos, member))

/**
 * ja_list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * pos and head all pointer .explaination from wangmin
 *   pos is temporary varialble
 * & head is more global variable compare to pos.
 */
#define ja_list_for_each(pos, head) \
	for (pos = (head)->next; !ja_list_is_head(pos, (head)); pos = pos->next)

/**
 * ja_list_for_each_rcu - Iterate over a list in an RCU-safe fashion
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
#define ja_list_for_each_rcu(pos, head)		  \
	for (pos = rcu_dereference((head)->next); \
	     !list_is_head(pos, (head)); \
	     pos = rcu_dereference(pos->next))
*/

/**
 * ja_list_for_each_continue - continue iteration over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * Continue to iterate over a list, continuing after the current position.
 */
#define ja_list_for_each_continue(pos, head) \
	for (pos = pos->next; !ja_list_is_head(pos, (head)); pos = pos->next)

/**
 * ja_list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define ja_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; !ja_list_is_head(pos, (head)); pos = pos->prev)

/**
 * ja_list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define ja_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; \
	     !ja_list_is_head(pos, (head)); \
	     pos = n, n = pos->next)

/**
 * ja_list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define ja_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     !ja_list_is_head(pos, (head)); \
	     pos = n, n = pos->prev)

/**
 * ja_list_entry_is_head - test if the entry points to the head of the list
 * @pos:	the type * to cursor
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define ja_list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

/**
 * ja_list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * head �ǵ�������������һ���������ڴ��ṹ��Сbitou
 *
 *       [ ]     [ ]     [ ]
 * *<--->[*]<--->[*]<--->[*]
 *		 [ ]	 [ ]	 [ ]
 */
#define ja_list_for_each_entry(pos, head, member)				\
	for (pos = ja_list_first_entry(head, typeof(*pos), member);	\
	     !ja_list_entry_is_head(pos, head, member);			\
	     pos = ja_list_next_entry(pos, member))

/**
 * ja_list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *                          <---
 *       [ ]     [ ]     [ ]
 * *<--->[*]<--->[*]<--->[*]
 *		 [ ]	 [ ]	 [ ]
 */
#define ja_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = ja_list_last_entry(head, typeof(*pos), member);		\
	     !ja_list_entry_is_head(pos, head, member); 			\
	     pos = ja_list_prev_entry(pos, member))

/**
 * ja_list_prepare_entry - prepare a pos entry for use in ja_list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @current:	the address of the list_head within the struct
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in ja_list_for_each_entry_continue().
 *
 * ȷ��һ�½ṹ��λ�ã���ֵpos
 * pos-->[ ]
 *       [ ]
 *       [ ] list_head member
 */
#define ja_list_prepare_entry(pos, current, member) \
	((pos) ? : ja_list_entry(current, typeof(*pos), member))

/**
 * ja_list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 *
 * pos ���ṹ��ָ�� ��
 */
#define ja_list_for_each_entry_continue(pos, head, member) 		\
	for (pos = ja_list_next_entry(pos, member);			\
	     !ja_list_entry_is_head(pos, head, member);			\
	     pos = ja_list_next_entry(pos, member))

/**
 * ja_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define ja_list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = ja_list_prev_entry(pos, member);			\
	     !ja_list_entry_is_head(pos, head, member);			\
	     pos = ja_list_prev_entry(pos, member))

/**
 * ja_list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define ja_list_for_each_entry_from(pos, head, member) 			\
	for (; !ja_list_entry_is_head(pos, head, member);			\
	     pos = ja_list_next_entry(pos, member))

/**
 * ja_list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define ja_list_for_each_entry_from_reverse(pos, head, member)		\
	for (; !ja_list_entry_is_head(pos, head, member);			\
	     pos = ja_list_prev_entry(pos, member))

/**
 * ja_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the address of your list_head with in the struct.
 * @member:	the name of the list_head within the struct.
 */
#define ja_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))

/**
 * ja_list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define ja_list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = ja_list_next_entry(pos, member), 				\
		n = ja_list_next_entry(pos, member);				\
	     !ja_list_entry_is_head(pos, head, member);				\
	     pos = n, n = ja_list_next_entry(n, member))

/**
 * ja_list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define ja_list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = ja_list_next_entry(pos, member);					\
	     !ja_list_entry_is_head(pos, head, member);				\
	     pos = n, n = ja_list_next_entry(n, member))

/**
 * ja_list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define ja_list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = ja_list_last_entry(head, typeof(*pos), member),		\
		n = ja_list_prev_entry(pos, member);			\
	     !ja_list_entry_is_head(pos, head, member); 			\
	     pos = n, n = ja_list_prev_entry(n, member))

/**
 * ja_list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:	the loop cursor used in the list_for_each_entry_safe loop
 * @n:		temporary storage used in list_for_each_entry_safe
 * @member:	the name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define ja_list_safe_reset_next(pos, n, member)				\
	n = ja_list_next_entry(pos, member)

#define JA_LIST_EXPLAIN_PART2 0x2
/**
 * Hash table list
 */

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

#define JA_HLIST_HEAD_INIT { .first = NULL }
#define JA_HLIST_HEAD(name) struct ja_hlist_head name = {  .first = NULL }
#define JA_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void JA_INIT_HLIST_NODE(struct ja_hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

/**
 * ja_hlist_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.  For example, hlist_nulls_del_init_rcu() does leave the
 * node in unhashed state, but hlist_nulls_del() does not.
 */
static inline int ja_hlist_unhashed(const struct ja_hlist_node *h)
{
	return !h->pprev;
}

/**
 * ja_hlist_unhashed_lockless - Version of hlist_unhashed for lockless use
 * @h: Node to be checked
 *
 * This variant of ja_hlist_unhashed() must be used in lockless contexts
 * to avoid potential load-tearing.  The JA_READ_ONCE() is paired with the
 * various JA_WRITE_ONCE() in hlist helpers that are defined below.
 */

static inline int ja_hlist_unhashed_lockless(const struct ja_hlist_node *h)
{
	return !JA_READ_ONCE(h->pprev);
}

/**
 * ja_hlist_empty - Is the specified hlist_head structure an empty hlist?
 * @h: Structure to check.
 */
static inline int ja_hlist_empty(const struct ja_hlist_head *h)
{
	return !JA_READ_ONCE(h->first);
}

static inline void __ja_hlist_del(struct ja_hlist_node *n)
{
	struct ja_hlist_node *next = n->next;
	struct ja_hlist_node **pprev = n->pprev;

	JA_WRITE_ONCE(*pprev, next);
	if (next)
		JA_WRITE_ONCE(next->pprev, pprev);
}

static inline void __jaorg_hlist_del(struct ja_hlist_node *n)
{
	JA_WRITE_ONCE(*(n->pprev), n->next);
	if (n->next)
		JA_WRITE_ONCE(n->next->pprev, n->pprev);
}

/**
 * ja_hlist_del - Delete the specified hlist_node from its list
 * @n: Node to delete.
 *
 * Note that this function leaves the node in hashed state.  Use
 * ja_hlist_del_init() or similar instead to unhash @n.
 */
static inline void ja_hlist_del(struct ja_hlist_node *n)
{
	__ja_hlist_del(n);
	n->next = (ja_hlist_node *)JA_LIST_POISON1;
	n->pprev = (ja_hlist_node **)JA_LIST_POISON2;
}

/**
 * ja_hlist_del_init - Delete the specified ja_hlist_node from its list and initialize
 * @n: Node to delete.
 *
 * Note that this function leaves the node in unhashed state.
 */
static inline void ja_hlist_del_init(struct ja_hlist_node *n)
{
	if (!ja_hlist_unhashed(n)) {
		__ja_hlist_del(n);
		JA_INIT_HLIST_NODE(n);
	}
}

/**
 * ja_hlist_add_head - add a newr entry at the beginning of the hlist
 * @n: newr entry to be added
 * @h: hlist head to add it after
 *
 * Insert a newr entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void hlist_add_head(struct ja_hlist_node *n, struct ja_hlist_head *h)
{
	struct ja_hlist_node *first = h->first;
	JA_WRITE_ONCE(n->next, first);
	if (first)
		JA_WRITE_ONCE(first->pprev, &n->next);
	JA_WRITE_ONCE(h->first, n);
	JA_WRITE_ONCE(n->pprev, &h->first);
}

/**
 * ja_hlist_add_before - add a newr entry before the one specified
 * @n: newr entry to be added
 * @next: hlist node to add it before, which must be non-NULL
 */
static inline void ja_hlist_add_before(struct ja_hlist_node *n,
				    struct ja_hlist_node *next)
{
	JA_WRITE_ONCE(n->pprev, next->pprev);
	JA_WRITE_ONCE(n->next, next);
	JA_WRITE_ONCE(next->pprev, &n->next);
	JA_WRITE_ONCE(*(n->pprev), n);
}

/**
 * ja_hlist_add_behind - add a newr entry after the one specified
 * @n: newr entry to be added
 * @prev: hlist node to add it after, which must be non-NULL
 */
static inline void ja_hlist_add_behind(struct ja_hlist_node *n,
				    struct ja_hlist_node *prev)
{
	JA_WRITE_ONCE(n->next, prev->next);
	JA_WRITE_ONCE(prev->next, n);
	JA_WRITE_ONCE(n->pprev, &prev->next);

	if (n->next)
		JA_WRITE_ONCE(n->next->pprev, &n->next);
}

/**
 * ja_hlist_add_fake - create a fake hlist consisting of a single headless node
 * @n: Node to make a fake list out of
 *
 * This makes @n appear to be its own predecessor on a headless hlist.
 * The point of this is to allow things like hlist_del() to work correctly
 * in cases where there is no list.
 */
static inline void ja_hlist_add_fake(struct ja_hlist_node *n)
{
	n->pprev = &n->next;
}

/**
 * ja_hlist_fake: Is this node a fake hlist?
 * @h: Node to check for being a self-referential fake hlist.
 */
static inline bool ja_hlist_fake(struct ja_hlist_node *h)
{
	return h->pprev == &h->next;
}

/**
 * ja_hlist_is_singular_node - is node the only element of the specified hlist?
 * @n: Node to check for singularity.
 * @h: Header for potentially singular list.
 *
 * Check whether the node is the only node of the head without
 * accessing head, thus avoiding unnecessary cache misses.
 */
static inline bool
ja_hlist_is_singular_node(struct ja_hlist_node *n, struct ja_hlist_head *h)
{
	return !n->next && n->pprev == &h->first;
}

/**
 * ja_hlist_move_list - Move an hlist
 * @old: hlist_head for old list.
 * @newr: hlist_head for newr list.
 *
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void ja_hlist_move_list(struct ja_hlist_head *old,
				   struct ja_hlist_head *newr)
{
	newr->first = old->first;
	if (newr->first)
		newr->first->pprev = &newr->first;
	old->first = NULL;
}

#define ja_hlist_entry(ptr, type, member) ja_container_of(ptr,type,member)

#define ja_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define ja_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

#define ja_hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? ja_hlist_entry(____ptr, type, member) : NULL; \
	})

/**
 * ja_hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define ja_hlist_for_each_entry(pos, head, member)				\
	for (pos = ja_hlist_entry_safe((head)->first, typeof(*(pos)), member);\
	     pos;							\
	     pos = ja_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * ja_hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define ja_hlist_for_each_entry_continue(pos, member)			\
	for (pos = ja_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
	     pos;							\
	     pos = ja_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define ja_hlist_for_each_entry_from(pos, member)				\
	for (; pos;							\
	     pos = ja_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		a &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define ja_hlist_for_each_entry_safe(pos, n, head, member) 		\
	for (pos = ja_hlist_entry_safe((head)->first, typeof(*pos), member);\
	     pos && ({ n = pos->member.next; 1; });			\
	     pos = ja_hlist_entry_safe(n, typeof(*pos), member))


#endif
