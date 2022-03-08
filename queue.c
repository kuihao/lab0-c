#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <bsd/string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q_head = malloc(sizeof(struct list_head));
    if (q_head) {
        INIT_LIST_HEAD(q_head);
    }
    return q_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        q_release_element(entry);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_node = malloc(sizeof(element_t));
    if (!new_node)
        return false;
    INIT_LIST_HEAD(&new_node->list);
    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);
        return false;
    }
    list_add(&new_node->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_node = malloc(sizeof(element_t));
    if (new_node) {
        INIT_LIST_HEAD(&new_node->list);
        new_node->value = strdup(s);
        list_add_tail(&new_node->list, head);
        return true;
    } else {
        return false;
    }
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && !list_empty(head)) {
        element_t *node = list_entry(head->next, element_t, list);
        list_del(head->next);
        if (sp) {
            int len = strlen(node->value) + 1;
            len = fmin(bufsize, len);
            strncpy(sp, node->value, len);
            sp[len - 1] = '\0';
        }
        return node;
    } else {
        return NULL;
    }
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head) {
        element_t *node = list_entry(head->prev, element_t, list);
        list_del(head->prev);
        if (sp != NULL) {
            int len = strlen(node->value) + 1;
            len = fmin(bufsize, len);
            strncpy(sp, node->value, len);
            sp[len - 1] = '\0';
        }
        return node;
    } else {
        return NULL;
    }
    return NULL;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head->next == NULL) {
        return false;
    } else {
        struct list_head *current = head->next;
        int size = q_size(head);
        for (int i = 0; i < size / 2; i++) {
            current = current->next;
        }
        list_del(current);
        free(list_entry(current, element_t, list)->value);
        free(list_entry(current, element_t, list));
        return true;
    }
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head) {
        return false;
    }

    if (list_empty(head) || list_is_singular(head) ||
        head->prev == head->next) {
        return true;
    }

    /* 指向「指向當前 Node 的指標」的指標 */
    struct list_head **inderect = &head->next;
    /* 一個環狀垃圾桶 */
    struct list_head *GarbageCan = q_new();
    INIT_LIST_HEAD(GarbageCan);

    for (; *inderect != head;) {  // && (*inderect)->next!=head
        char *first_node_str = list_entry(*inderect, element_t, list)->value;
        char *second_node_str =
            list_entry((*inderect)->next, element_t, list)->value;
        /* 垃圾桶最上層將存放最新的副本 (LIFO)，Node 優先與垃圾桶比對 */
        if ((GarbageCan->next != GarbageCan) &&
            (strcmp(list_entry(GarbageCan->next, element_t, list)->value,
                    first_node_str) == 0)) {
            struct list_head *directly_recycle = *inderect;
            list_del(directly_recycle);
            q_release_element(list_entry(directly_recycle, element_t, list));
        } else {
            /* 垃圾桶內找不到才跟下一個 Node 比對 */
            if (!strcmp(first_node_str, second_node_str)) {
                struct list_head *goto_GarbageCan = *inderect;
                struct list_head *directly_recycle = (*inderect)->next;
                list_del(goto_GarbageCan);
                list_add(goto_GarbageCan, GarbageCan);

                list_del(directly_recycle);
                q_release_element(
                    list_entry(directly_recycle, element_t, list));
            } else
                inderect = &(*inderect)->next;
        }
    }
    q_free(GarbageCan);  // 清空垃圾桶
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!(head) || list_empty(head) || list_is_singular(head)) {
        return;
    }
    struct list_head *current = head;
    for (; current->next != head && current->next->next != head;
         current = current->next->next) {
        list_move(current->next->next, current);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head) ||
        head->next == head->prev)
        return;
    // [1]-> [2]-> [3]-> [4]
    struct list_head *current = head;
    do {
        struct list_head *tmp = current->next;
        current->next = current->prev;
        current->prev = tmp;

        current = current->next;
    } while (current != head);
}


struct list_head *Merge2Lists(struct list_head *list1, struct list_head *list2)
{
    struct list_head *head = NULL, **ptr = &head, **node;
    for (node = NULL; list1 && list2; *node = (*node)->next) {
        node =
            signbit((float) strcmp(list_entry(list1, element_t, list)->value,
                                   list_entry(list2, element_t, list)->value))
                ? &list1
                : &list2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) list1 | (uintptr_t) list2);
    return head;
}

struct list_head *Mergesort_list(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *slow = head;
    for (struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;
    struct list_head *mid = slow->next;
    slow->next = NULL;

    struct list_head *left = Mergesort_list(head), *right = Mergesort_list(mid);
    return Merge2Lists(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    head->prev->next = NULL;
    head->next = Mergesort_list(head->next);
    struct list_head *ptr = head;
    for (; ptr->next; ptr = ptr->next) {
        ptr->next->prev = ptr;
    }
    ptr->next = head;
    head->prev = ptr;
}
