/*
 *   humbug-avl.h - Header file of simple AVL tree implementation
 *   Copyright (C) 2012, Humbug Analytics Labs
 *
 *    <http://www.humbuglabs.org> or <support@humbuglabs.org>
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

#ifndef HUMBUG_AVL_H_
#define HUMBUG_AVL_H_

typedef enum {
	LH, EH, RH
} avl_bal_t;

typedef struct avl_node_t{
	void *data;
	void *key;
	struct avl_node_t *left;
	struct avl_node_t *right;
	avl_bal_t bal_f;
} avl_node_t;

typedef struct avl_root_t {
	int count;
	int (*compare)(void *__a, void *__b);
	struct avl_node_t *root;
} avl_root_t;

#endif /* HUMBUG_AVL_H_ */

/* Init AVL tree */
avl_root_t *avl_init(int(*compare)(void *__a, void *__b));

/* Insert new node into AVL tree */
int avl_insert(avl_root_t *__tree_root, void *__key, void *__data);

/* Destroy AVL tree */
void avl_destroy(avl_root_t *__tree_root);

/* Search key in AVL tree */
avl_node_t *avl_search(avl_root_t *__tree_root, void *__key);

/* Returns table size */
int avl_count(avl_root_t *__tree_root);
