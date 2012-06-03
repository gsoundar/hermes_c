#ifndef __HMS_LIST_H__
#define __HMS_LIST_H__

#ifdef	__cplusplus
extern "C" {
#endif


/* This file is from Linux Kernel (include/linux/list.h) 
 * and modified by simply removing hardware prefetching of list items. 
 * Here by copyright, credits attributed to wherever they belong.
 * Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
 */

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct hms_list_head {
	struct hms_list_head *next, *prev;
};

#define HMS_LIST_HEAD_INIT(name) { &(name), &(name) }

#define HMS_LIST_HEAD(name) \
	struct hms_list_head name = HMS_LIST_HEAD_INIT(name)

#define HMS_INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a newp entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __hms_list_add(struct hms_list_head *nnew,
			      struct hms_list_head *prev,
			      struct hms_list_head *next)
{
	next->prev = nnew;
	nnew->next = next;
	nnew->prev = prev;
	prev->next = nnew;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void hms_list_add(struct hms_list_head *newp, struct hms_list_head *head)
{
	__hms_list_add(newp, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void hms_list_add_tail(struct hms_list_head *newp, struct hms_list_head *head)
{
	__hms_list_add(newp, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __hms_list_del(struct hms_list_head *prev, struct hms_list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void hms_list_del(struct hms_list_head *entry)
{
	__hms_list_del(entry->prev, entry->next);
	entry->next = (struct hms_list_head *) NULL;
	entry->prev = (struct hms_list_head *) NULL;
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void hms_list_del_init(struct hms_list_head *entry)
{
	__hms_list_del(entry->prev, entry->next);
	HMS_INIT_LIST_HEAD(entry); 
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void hms_list_move(struct hms_list_head *list, struct hms_list_head *head)
{
        __hms_list_del(list->prev, list->next);
        hms_list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void hms_list_move_tail(struct hms_list_head *list,
				  struct hms_list_head *head)
{
        __hms_list_del(list->prev, list->next);
        hms_list_add_tail(list, head);
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int hms_list_empty(struct hms_list_head *head)
{
	return head->next == head;
}

static inline void __hms_list_splice(struct hms_list_head *list,
				 struct hms_list_head *head)
{
	struct hms_list_head *first = list->next;
	struct hms_list_head *last = list->prev;
	struct hms_list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void hms_list_splice(struct hms_list_head *list, struct hms_list_head *head)
{
	if (!hms_list_empty(list))
		__hms_list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void hms_list_splice_init(struct hms_list_head *list,
				    struct hms_list_head *head)
{
	if (!hms_list_empty(list)) {
		__hms_list_splice(list, head);
		HMS_INIT_LIST_HEAD(list);
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define hms_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define hms_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)
/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define hms_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)
        	
/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define hms_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define hms_list_for_each_entry(pos, head, member)				\
	for (pos = hms_list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = hms_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define hms_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = hms_list_entry((head)->next, typeof(*pos), member),	\
		n = hms_list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = hms_list_entry(n->member.next, typeof(*n), member))

#ifdef	__cplusplus
}
#endif


#endif
