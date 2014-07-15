/*
 *   humbug-avl.c - simple AVL tree implementation
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

#include "include/humbug-avl.h"

/*******************************************************************************
 * Initialize a new AVL tree
 * arg1 - address of comparison function
 * returns pointer to a new AVL tree head
 */
avl_root_t *avl_init(int (*compare)(void *__a, void *__b)){
	avl_root_t *tree = (avl_root_t *)malloc(sizeof(avl_root_t));
	if(!tree)
		return NULL;
	tree->root    = NULL;
	tree->count   = 0;
	tree->compare = compare;
	return tree;
}

/*******************************************************************************
 * Destroying AVL tree
 * arg1 - AVL tree head
 */
void avl_destroy(avl_root_t *tree){
	void _destroy_avl(avl_node_t *root);

	if(tree)
		_destroy_avl(tree->root);
	free(tree);
	return;
}

/*******************************************************************************
 * Private function
 */
void _destroy_avl(avl_node_t *root){
	if(root){
		_destroy_avl(root->left);
		free(root->data);
		_destroy_avl(root->right);
		free(root);
	}
	return;
}

/*******************************************************************************
 * Insert a new table node into AVL tree
 * arg1 - AVL tree head
 * arg2 - key (part of table node data)
 * arg3 - pointer to table node data
 */
int avl_insert(avl_root_t *tree, void *key, void *data){
	avl_node_t *_insert(avl_root_t *tree, avl_node_t *root, avl_node_t *new_p,
			int *taller);

	int for_taller;
	avl_node_t *new_p = (avl_node_t *)malloc(sizeof(avl_node_t));
	if(!new_p)
		return (0);

	new_p->bal_f = EH;
	new_p->right = NULL;
	new_p->left  = NULL;
	new_p->key   = key;
	new_p->data  = data;

	tree->root = _insert(tree, tree->root, new_p, &for_taller);
	(tree->count)++;

	return (1);
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_insert(avl_root_t *tree, avl_node_t *root, avl_node_t *new_p,
		int *taller){
	avl_node_t *_left_balance (avl_node_t *root, int *taller);
	avl_node_t *_right_balance(avl_node_t *root, int *taller);

	if(!root){
		root    = new_p;
		*taller = 1; // TRUE
		return (root);
	}

	if(tree->compare((void *)new_p->key, (void *)root->key) < 0){
		root->left = _insert(tree, root->left, new_p, taller);
		if(*taller){
			switch (root->bal_f) {
				case LH:
					root = _left_balance(root, taller);
					break;
				case EH:
					root->bal_f = LH;
					break;
				case RH:
					root->bal_f = EH;
					*taller = 0; // FALSE
					break;
			} /* switch */
			return (root);
		}
	}else{
		root->right = _insert(tree, root->right, new_p, taller);
		if(*taller){
			switch (root->bal_f) {
				case LH:
					root->bal_f = EH;
					*taller = 0; // FALSE
					break;
				case EH:
					root->bal_f = RH;
					break;
				case RH:
					root = _right_balance(root, taller);
					break;
			} /* switch */
			return (root);
		}
		return (root);
	}
	return (root);
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_left_balance(avl_node_t *root, int *taller){
	avl_node_t *_rotate_left (avl_node_t *root);
	avl_node_t *_rotate_right(avl_node_t *root);

	avl_node_t *left_tree;
	avl_node_t *right_tree;

	left_tree = root->left;
	switch (left_tree->bal_f) {
		case LH:
			root->bal_f      = EH;
			left_tree->bal_f = EH;
			root = _rotate_right(root);
			*taller = 0; // FALSE
			break;
		case EH:
			// TODO: acception
//			printf("\n\a\aERROR in leftBalance");
			exit(EXIT_FAILURE);
			break;
		case RH:
			right_tree = left_tree->right;
			switch (right_tree->bal_f){
				case LH:
					root->bal_f      = RH;
					left_tree->bal_f = EH;
					break;
				case EH:
					root->bal_f      = EH;
					left_tree->bal_f = EH;
					break;
				case RH:
					root->bal_f      = EH;
					left_tree->bal_f = LH;
					break;
			} /* switch */

			right_tree->bal_f = EH;
			root->left = _rotate_left(left_tree);
			root = _rotate_right(root);
			*taller = 0; // FALSE
			break;
	} /* switch */
	return (root);
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_right_balance(avl_node_t *root, int *taller){
	avl_node_t *_rotate_left (avl_node_t *root);
	avl_node_t *_rotate_right(avl_node_t *root);

	avl_node_t *left_tree;
	avl_node_t *right_tree;

	right_tree = root->right;
	switch (right_tree->bal_f) {
		case RH:
			root->bal_f       = EH;
			right_tree->bal_f = EH;
			root = _rotate_left(root);
			*taller = 0; // FALSE
			break;
		case EH:
			// TODO: acception
//			printf("\n\a\aERROR in rightBalance");
			exit(EXIT_FAILURE);
			break;
		case LH:
			left_tree = right_tree->left;
			switch (left_tree->bal_f){
				case RH:
					root->bal_f       = LH;
					right_tree->bal_f = EH;
					break;
				case EH:
					root->bal_f       = EH;
					right_tree->bal_f = EH;
					break;
				case LH:
					root->bal_f       = EH;
					right_tree->bal_f = RH;
					break;
			} /* switch */

			left_tree->bal_f = EH;
			root->right = _rotate_right(right_tree);
			root = _rotate_left(root);
			*taller = 0; // FALSE
			break;
	}
	return (root);
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_rotate_right(avl_node_t *root){
	avl_node_t *p_tmp;
	p_tmp        = root->left;
	root->left   = p_tmp->right;
	p_tmp->right = root;
	return (p_tmp);
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_rotate_left(avl_node_t *root){
	avl_node_t *p_tmp;
	p_tmp       = root->right;
	root->right = p_tmp->left;
	p_tmp->left = root;
	return (p_tmp);
}

/*******************************************************************************
 * Search key in AVL tree
 * arg1 - AVL tree head
 * arg2 - search key
 * returns pointer to table node
 */
avl_node_t *avl_search(avl_root_t *tree, void *key){
	avl_node_t *_search(avl_root_t *tree, void *key, avl_node_t *root);
	if(!tree){
		return NULL;
	}
	if(tree->root){
		return _search(tree, key, tree->root);
	}
	return NULL;
}

/*******************************************************************************
 * Private function
 */
avl_node_t *_search(avl_root_t *tree, void *key, avl_node_t *root){
	if(root){
		if(tree->compare((void *)key, (void *)root->key) < 0){
			return _search(tree, key, root->left);
		}else if (tree->compare((void *)key, (void *)root->key) > 0){
			return _search(tree, key, root->right);
		}else{
			return (root);
		}
	}
	return NULL;
}

/*******************************************************************************
 * Returns tree size
 */
int avl_count(avl_root_t *tree){
	if(!tree)
		return 0;
	return (tree->count);
}
