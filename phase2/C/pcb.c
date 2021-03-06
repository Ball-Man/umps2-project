#include <pcb.e>

#include <lang.e>
#include <const.h>

/* Sentinel for the list of free pcbs */
HIDDEN struct list_head pcbfree_h;

/* Array containing all the pcbs. We can't
 * allocate things on the heap since we are
 * working with extra low level stuff, so we
 * need to allocate all the pcbs statically 
 */
HIDDEN pcb_t pcb_table[MAX_PROC];

/* Initialize list of free pcbs allocating MAX_PROC of them */
extern void initPcbs() {
  uint8_t i;

  INIT_LIST_HEAD(&pcbfree_h);   /* Initialize sentinel */

  /* Add all the pcbs to the list */
  for (i = 0; i < MAX_PROC; i++)
    freePcb((pcb_table + i));
}

/* Put p in the list of free pcbs */
extern void freePcb(pcb_t *p) {
  list_add_tail(&(p->p_next), &pcbfree_h);
}

/* Remove and return one pcb from the free list */
extern pcb_t *allocPcb() {
  struct list_head *alloc_head;
  pcb_t *alloc;

  if (list_empty(&pcbfree_h))
    return NULL;

  /* Remove from list */
  alloc_head = pcbfree_h.next;
  list_del(alloc_head);
  alloc = list_entry(alloc_head, pcb_t, p_next);

  /* Initialize pcb */
  memset(alloc, 0, sizeof(pcb_t));

  return alloc;
}

/* Initialize the list of pcbs, initializing the list dummy */
extern void mkEmptyProcQ(struct list_head *list_head) {
  INIT_LIST_HEAD(list_head);
}

/* Returns true if the list is empty, false otherwise */
extern bool emptyProcQ(struct list_head *list_head) {
  return list_empty(list_head);
}

/* Insert the element in the queue */
extern void insertProcQ(struct list_head *list_head, pcb_t *p) {
  struct list_head *pos;
  list_for_each(pos, list_head) {
    if (list_entry(pos, pcb_t, p_next)->priority < p->priority) {
      list_add_tail(&p->p_next, pos);
      return;
    }
  }
  /* Priority less than every other: the if is never triggered; add at the end */
  list_add_tail(&p->p_next, pos);
}

/* Returns the first element without removing it. Null if list is empty */
extern pcb_t *headProcQ(struct list_head *list_head) {
  if (emptyProcQ(list_head))
    return NULL;

  pcb_t *p = list_entry(list_head->next, pcb_t, p_next);
  return p;
}

/* Returns the first element removing it. Null if list is empty */
extern pcb_t *removeProcQ(struct list_head *list_head) {
  pcb_t *p = headProcQ(list_head);

  if (p)
    list_del(&(p->p_next));
  return p;
}

/* Removes the pcb from the queue, Null if absent */
extern pcb_t *outProcQ(struct list_head *list_head, pcb_t *p) {
  struct list_head *pos;

  list_for_each(pos, list_head) {
    if (list_entry(pos, pcb_t, p_next) == p) {
      list_del(pos);
      return list_entry(pos, pcb_t, p_next);
    }
  }

  return NULL;
}

/* Returns true if p has no child */
extern bool emptyChild(pcb_t * p) {
  return list_empty(&p->p_child) || p->p_child.next == NULL;
}

/* Inserts p as child of prnt */
extern void insertChild(pcb_t *prnt, pcb_t *p) {
  /* Initialize parent's and p's children list if necessary */
  if (!prnt->p_child.next)
    INIT_LIST_HEAD(&prnt->p_child);
  if (!p->p_child.next)
    INIT_LIST_HEAD(&p->p_child);

  list_add_tail(&p->p_sib, &prnt->p_child);  /* Adds as last child */
  p->p_parent = prnt;
}

/* Removes and returns the first child of p. Returns NULL if it has no child */
extern pcb_t *removeChild(pcb_t *p) {
  pcb_t *tmp;

  if (emptyChild(p))
    return NULL;

  tmp = list_entry(p->p_child.next, pcb_t, p_sib);
  list_del(&tmp->p_sib);  /* Remove from parent */
  tmp->p_parent = NULL;
  return tmp;
}

/* Removes p from the list of children of its parent */
extern pcb_t *outChild(pcb_t *p) {
  if (!p->p_parent)
    return NULL;

  list_del(&p->p_sib);
  p->p_parent = NULL;

  return p;
}

/* Gets the pcb unitary offset from the internal pcb table(used to calculate stack pointer) */
extern uint8_t getPcbOffset(pcb_t *p) {
  return ((memaddr) p - (memaddr) pcb_table) / sizeof(pcb_t);
}
