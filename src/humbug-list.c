/*
 *	 humbug-list.c
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

#include <stdio.h>
#include "include/humbug-list.h"

//******************************************************************************
/*
 *
 */
list_root_t *list_init(int(*compare)(void *__a, void *__b)){
	list_root_t *list = (list_root_t *)malloc(sizeof(list_root_t));
	if(!list)
		return (NULL);

	list->length  = 0;
	list->head    = NULL;
	list->tail    = NULL;
	list->compare = compare;
	return (list);
}

//******************************************************************************
/*
 *
 */
void list_destroy(list_root_t *list){
	if(!list)
		return;
	list_node_t *next;
	list_node_t *curr = list->head;
	while(curr){
		next = curr->next;
		if(curr->data) free(curr->data);
		free(curr);
		curr = next;
	}
	free(list);
	list = NULL;
	return;
}

//******************************************************************************
/*
 *
 */
int list_insert(list_root_t *list, void *key, void *data){
	if(!list) return (0);
	if(!key ) return (0);

	list_node_t *new_node = (list_node_t *)malloc(sizeof(list_node_t));
	if(!new_node) return (0);

	new_node->key  = key;
	new_node->data = data;
	new_node->next = NULL;

	if(!list->head){
		list->head   = new_node;
		list->tail   = list->head;
		list->length = 1;
		return (1);
	}

	list->tail->next = new_node;
	list->tail = new_node;
	list->length++;
	return (1);
}

//******************************************************************************
/*
 *
 */
list_node_t *list_search(list_root_t *list, void *key){
	if(!list){
		return (NULL);
	}
	list_node_t *curr = list->head;
	if(!curr){
		return (NULL);
	}
	while(curr){
		if(list->compare(curr->key, key) == 0){
			return (curr);
		}
		curr = curr->next;
	}
	return (NULL);
}

//******************************************************************************
/*
 *
 */
int list_length(list_root_t *list){
	if(!list)
		return (-1);
	return (list->length);
}
