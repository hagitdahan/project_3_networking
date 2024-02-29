#pragma once

#include <stdlib.h>

/********************************************************************************
 *
 * A List library.
 *
 * This library provides a List of doubles data structure.
 *
 * This library will fail in unpredictable ways when the system memory runs
 * out.
 *
 ********************************************************************************/

/*
 * List represents a List data structure.
 */
struct _List;
typedef struct _List List;

/*
 * Allocates a new empty List.
 * It's the user responsibility to free it with List_free.
 */
List* List_alloc();

/*
 * Frees the memory and resources allocated to list.
 * If list==NULL does nothing (same as free).
 */
void List_free(List* list);

/*
 * Returns the number of elements in the list.
 */
size_t List_size(const List* list);

/*
 * Inserts an element in the begining of the list.
 */

void List_insertLast(List* list, double data, size_t weight);

/*
 * Prints the list to the standard output.
 */
void List_print(const List* list);

/*
 * Count the list data
 */
float List_count(const List* list);

float List_avarage_time(const List* list);

float List_avarage_bandwidth(const List* list);


