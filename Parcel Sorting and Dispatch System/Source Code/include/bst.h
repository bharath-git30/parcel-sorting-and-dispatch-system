#ifndef BST_H
#define BST_H

#include "parcel.h"
#include "queue.h"
#include <stdbool.h>

/* BST Node representing a unique destination city with its own parcel FIFO queue */
typedef struct BSTNode {
    char city[MAX_CITY_LEN];
    Queue* parcel_queue;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

/* Binary Search Tree structure */
typedef struct BST {
    BSTNode* root;
    int city_count;
} BST;

/* Callback function type for in-order traversal */
typedef void (*BSTNodeVisitor)(const BSTNode* node, void* userdata);

BST* bst_create(void);
void bst_free(BST* tree);

BSTNode* bst_insert_city(BST* tree, const char* city);
bool bst_insert_parcel(BST* tree, Parcel* parcel);
BSTNode* bst_search_city(const BST* tree, const char* city);
Parcel* bst_retrieve_parcel(BST* tree, const char* city);

int bst_count_nodes(const BST* tree);
int bst_calculate_height(const BST* tree);
int bst_total_parcels(const BST* tree);

void bst_inorder_traversal(const BST* tree, BSTNodeVisitor visitor, void* userdata);
void bst_print_summary(const BST* tree);
void bst_print_detailed(const BST* tree);

#endif /* BST_H */
