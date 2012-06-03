/*
 * Implementation of the hash table type.
 *
 * Author : Stephen Smalley, <sds@epoch.ncsc.mil>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <hashtable.h>

struct hashtab *hashtab_create(unsigned long (*hash_value)(struct hashtab *h, void *key),
                               int (*keycmp)(struct hashtab *h, void *key1, void *key2),
                               unsigned long size)
{
  struct hashtab *p;
  unsigned long i;

  p = (struct hashtab *) malloc(sizeof(*p));

  if (p == NULL)
    return p;

  p->size = size;
  p->nel = 0;
  p->hash_value = hash_value;
  p->keycmp = keycmp;
  p->htable = malloc(sizeof(*(p->htable)) * size);

  if (p->htable == NULL) {
    perror("malloc");
    free(p);
    return NULL;
  }

  for (i = 0; i < size; i++)
    p->htable[i] = NULL;

  return p;
}

int hashtab_insert(struct hashtab *h, void *key, void *datum)
{
  unsigned long hvalue;
  struct hashtab_node *prev, *cur, *newnode;

  if (!h || h->nel == HASHTAB_MAX_NODES)
    return -EINVAL;

  hvalue = h->hash_value(h, key);
  prev = NULL;
  cur = h->htable[hvalue];

  while (cur && h->keycmp(h, key, cur->key) > 0) {
    //fprintf(stdout, "keycmp: key1: (%x) %s key2: (%x) %s\n", key, key, cur->key, cur->key); 
    prev = cur;
    cur = cur->next;
  }

  if (cur && (h->keycmp(h, key, cur->key) == 0))
    return -EEXIST;
  newnode = malloc(sizeof(*newnode));
  if (newnode == NULL)
    return -ENOMEM;
  newnode->key = key;
  newnode->datum = datum;
  if (prev) {
    newnode->next = prev->next;
    prev->next = newnode;
  } else {
    newnode->next = h->htable[hvalue];
    h->htable[hvalue] = newnode;
  }

  h->nel++;
  return 0;
}

void * hashtab_delete(struct hashtab *h, void *key)
{

  unsigned long hvalue;
  void *d;
  struct hashtab_node *prev;
  struct hashtab_node *cur;

  if(!h)
    return NULL;

  hvalue = h->hash_value(h, key);
  cur = h->htable[hvalue];

  /* if need to rem first node */
  if(cur != NULL && h->keycmp(h, key, cur->key) == 0) {
    h->htable[hvalue] = cur->next;
    cur->next = 0;
    d = cur->datum;
    free(cur);
    h->nel--;
    return d;
  }

  /* some node after first node */
  /* trying to avoid SEGFAULT : Gokul */
  if(cur != NULL) {
    prev = cur;
    cur = cur->next;
    while(cur != NULL) {  
      if(h->keycmp(h, key, cur->key) == 0) {
	prev->next = cur->next;
	cur->next = NULL;
	d = cur->datum;
	h->nel--;
	return d;
	break;
      } else {
	prev = cur; 
	cur = cur->next;
      } 
    } // end while
  } else {
    fprintf(stderr, "cur is null where it shouldn't be !!!!!\n");
  }

  return NULL;
}

void *hashtab_search(struct hashtab *h, void *key)
{
  unsigned long hvalue;
  struct hashtab_node *cur;

  if (!h)
    return NULL;

  hvalue = h->hash_value(h, key);
  cur = h->htable[hvalue];
  while (cur != NULL && h->keycmp(h, key, cur->key) > 0)
    cur = cur->next;

  if (cur == NULL || (h->keycmp(h, key, cur->key) != 0))
    return NULL;

  return cur->datum;
}

void hashtab_destroy(struct hashtab *h, 
		     void (*key_free)(void *ptr),
		     void (*datum_free)(void *ptr)
		     )
{
  unsigned long i;
  struct hashtab_node *cur, *temp;

  if (!h)
    return;

  for (i = 0; i < h->size; i++) {
    cur = h->htable[i];
    while (cur != NULL) {
      temp = cur;
      cur = cur->next;
      free(temp);
    }
    h->htable[i] = NULL;
  }
  free(h->htable);
  h->htable = NULL;
  free(h);
}

int hashtab_map(struct hashtab *h,
		int (*apply)(void *k, void *d, void *args),
		void *args)
{
  unsigned long i;
  int ret;
  struct hashtab_node *cur;

  if (!h)
    return 0;

  for (i = 0; i < h->size; i++) {
    cur = h->htable[i];
    while (cur != NULL) {
      ret = apply(cur->key, cur->datum, args);
      if (ret)
	return ret;
      cur = cur->next;
    }
  }
  return 0;
}


void hashtab_stat(struct hashtab *h, struct hashtab_info *info)
{
  unsigned long i, chain_len, slots_used, max_chain_len;
  struct hashtab_node *cur;

  slots_used = 0;
  max_chain_len = 0;
  for (slots_used = max_chain_len = i = 0; i < h->size; i++) {
    cur = h->htable[i];
    if (cur) {
      slots_used++;
      chain_len = 0;
      while (cur) {
	chain_len++;
	cur = cur->next;
      }

      if (chain_len > max_chain_len)
	max_chain_len = chain_len;
    }
  }

  info->slots_used = slots_used;
  info->max_chain_len = max_chain_len;
}

void hashtab_print(struct hashtab *h, void (*print)(void *key, void *data)) {
  unsigned long i;
  struct hashtab_node *cur, *temp;
  int count = 0;

  if (!h)
    return;
  
  for (i = 0; i < h->size; i++) {
    cur = h->htable[i];
    printf("SLOT [%lu]:", i);
    while (cur != NULL) {
      printf("[%x]", (uintptr_t) cur); count++;
      temp = cur;
      cur = cur->next;
    }
    printf("\n");
  }
  fprintf(stdout, "Total items: %d\n", count);
}


hashtab_iterator * hashtab_iterate(struct hashtab *h, hashtab_iterator *iterator) {

  struct hashtab_node *cur = NULL; 
  if(iterator == NULL) {
    iterator = (hashtab_iterator *) malloc(sizeof(hashtab_iterator));
    iterator->slot_id = 0;
    iterator->node_ptr = NULL;
  }

  /* find the next node */
  if(iterator->node_ptr != NULL)
    cur = iterator->node_ptr->next;
  else
    cur = h->htable[0];
  int level = iterator->slot_id;
  
  while(level < h->size) {
    if(cur != NULL) {
      iterator->slot_id = level;
      iterator->node_ptr = cur;
      return iterator;
    }
    //fprintf(stdout, "checking level %d\n", level);
    level ++;
    if(level < h->size) { cur = h->htable[level]; }
    else cur = NULL;
  }

  free(iterator);
  return NULL;

}
