/*
 *	 humbug-list.h
 *   Copyright (C) 2012, Humbug Analytics Labs
 *
 *   <http://www.humbuglabs.org> or <support@humbuglabs.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Igor Ratgauzer <igor@humbuglabs.org>
 *   version 0.1
 */

#include <stdlib.h>

#ifndef HUMBUG_LIST_H_
#define HUMBUG_LIST_H_

typedef struct list_node_t {
	void *data;
	void *key;
	struct list_node_t *next;
} list_node_t;

typedef struct list_root_t {
	int length;
	struct list_node_t *head;
	struct list_node_t *tail;
	int (*compare)(void *__a, void *__b);
} list_root_t;

#endif /* HUMBUG_LIST_H_ */

/* Init linked list */
list_root_t *list_init(int(*compare)(void *__a, void *__b));

/* Add node to list */
int list_insert(list_root_t *__list_root, void *__key, void *__data);

/* Destroy list */
void list_destroy(list_root_t *__list_root);

/* Search key in list */
list_node_t *list_search(list_root_t *__list_root, void *__key);

/* Returns list length */
int list_length(list_root_t *__list_root);
