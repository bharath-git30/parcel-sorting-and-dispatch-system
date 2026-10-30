#include "bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BSTNode* bst_node_create(const char* city) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for BSTNode.\n");
        return NULL;
    }
    strncpy(node->city, city, sizeof(node->city) - 1);
    node->city[sizeof(node->city) - 1] = '\0';
    node->parcel_queue = queue_create(0); /* Unbounded queue for city parcels */
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void bst_node_free(BSTNode* node) {
    if (!node) return;
    bst_node_free(node->left);
    bst_node_free(node->right);
    if (node->parcel_queue) {
        queue_free(node->parcel_queue);
    }
    free(node);
}

BST* bst_create(void) {
    BST* tree = (BST*)malloc(sizeof(BST));
    if (!tree) {
        fprintf(stderr, "Error: Memory allocation failed for BST.\n");
        return NULL;
    }
    tree->root = NULL;
    tree->city_count = 0;
    return tree;
}

void bst_free(BST* tree) {
    if (!tree) return;
    bst_node_free(tree->root);
    free(tree);
}

static BSTNode* insert_city_recursive(BSTNode* root, const char* city, int* added) {
    int cmp;
    if (root == NULL) {
        *added = 1;
        return bst_node_create(city);
    }
    cmp = strcmp(city, root->city);
    if (cmp < 0) {
        root->left = insert_city_recursive(root->left, city, added);
    } else if (cmp > 0) {
        root->right = insert_city_recursive(root->right, city, added);
    } else {
        /* Already exists */
        *added = 0;
    }
    return root;
}

BSTNode* bst_insert_city(BST* tree, const char* city) {
    int added = 0;
    if (!tree || !city) return NULL;
    tree->root = insert_city_recursive(tree->root, city, &added);
    if (added) {
        tree->city_count++;
    }
    return bst_search_city(tree, city);
}

static BSTNode* search_city_recursive(BSTNode* root, const char* city) {
    int cmp;
    if (root == NULL) return NULL;
    cmp = strcmp(city, root->city);
    if (cmp == 0) return root;
    if (cmp < 0) return search_city_recursive(root->left, city);
    return search_city_recursive(root->right, city);
}

BSTNode* bst_search_city(const BST* tree, const char* city) {
    if (!tree || !city) return NULL;
    return search_city_recursive(tree->root, city);
}

bool bst_insert_parcel(BST* tree, Parcel* parcel) {
    BSTNode* node;
    if (!tree || !parcel) return false;
    node = bst_search_city(tree, parcel->destination);
    if (!node) {
        node = bst_insert_city(tree, parcel->destination);
    }
    if (!node || !node->parcel_queue) return false;
    return queue_enqueue(node->parcel_queue, parcel);
}

Parcel* bst_retrieve_parcel(BST* tree, const char* city) {
    BSTNode* node;
    if (!tree || !city) return NULL;
    node = bst_search_city(tree, city);
    if (!node || !node->parcel_queue || queue_is_empty(node->parcel_queue)) {
        return NULL;
    }
    return queue_dequeue(node->parcel_queue);
}

static int count_nodes_recursive(const BSTNode* root) {
    if (!root) return 0;
    return 1 + count_nodes_recursive(root->left) + count_nodes_recursive(root->right);
}

int bst_count_nodes(const BST* tree) {
    if (!tree) return 0;
    return count_nodes_recursive(tree->root);
}

static int calculate_height_recursive(const BSTNode* root) {
    int left_h;
    int right_h;
    if (!root) return 0;
    left_h = calculate_height_recursive(root->left);
    right_h = calculate_height_recursive(root->right);
    return 1 + (left_h > right_h ? left_h : right_h);
}

int bst_calculate_height(const BST* tree) {
    if (!tree) return 0;
    return calculate_height_recursive(tree->root);
}

static int total_parcels_recursive(const BSTNode* root) {
    int current;
    if (!root) return 0;
    current = root->parcel_queue ? queue_size(root->parcel_queue) : 0;
    return current + total_parcels_recursive(root->left) + total_parcels_recursive(root->right);
}

int bst_total_parcels(const BST* tree) {
    if (!tree) return 0;
    return total_parcels_recursive(tree->root);
}

static void inorder_recursive(const BSTNode* root, BSTNodeVisitor visitor, void* userdata) {
    if (!root) return;
    inorder_recursive(root->left, visitor, userdata);
    if (visitor) {
        visitor(root, userdata);
    }
    inorder_recursive(root->right, visitor, userdata);
}

void bst_inorder_traversal(const BST* tree, BSTNodeVisitor visitor, void* userdata) {
    if (!tree) return;
    inorder_recursive(tree->root, visitor, userdata);
}

static void summary_visitor(const BSTNode* node, void* userdata) {
    int* idx = (int*)userdata;
    int count = node->parcel_queue ? queue_size(node->parcel_queue) : 0;
    printf("  [%d] City: %-15s | Waiting in city queue: %d\n", (*idx)++, node->city, count);
}

void bst_print_summary(const BST* tree) {
    int idx;
    if (!tree || !tree->root) {
        printf("  [Empty Destination Sorter BST]\n");
        return;
    }
    printf("Destination Sorter (BST In-Order Traversal [Alphabetical]):\n");
    printf("Total Cities: %d | BST Height: %d | Total Waiting in City Queues: %d\n",
           bst_count_nodes(tree), bst_calculate_height(tree), bst_total_parcels(tree));
    idx = 1;
    bst_inorder_traversal(tree, summary_visitor, &idx);
}

static void detailed_visitor(const BSTNode* node, void* userdata) {
    int count;
    (void)userdata;
    count = node->parcel_queue ? queue_size(node->parcel_queue) : 0;
    printf("\n  City: %s (Waiting in city queue: %d)\n", node->city, count);
    if (node->parcel_queue && !queue_is_empty(node->parcel_queue)) {
        QueueNode* curr = node->parcel_queue->front;
        int pidx = 1;
        while (curr) {
            printf("    %d. ", pidx++);
            parcel_print(curr->parcel);
            curr = curr->next;
        }
    } else {
        printf("    (No parcels waiting in this city's queue)\n");
    }
}

void bst_print_detailed(const BST* tree) {
    if (!tree || !tree->root) {
        printf("No parcels organized by destination.\n");
        return;
    }
    printf("Parcels Grouped by Destination City (BST Nodes):\n");
    bst_inorder_traversal(tree, detailed_visitor, NULL);
}
