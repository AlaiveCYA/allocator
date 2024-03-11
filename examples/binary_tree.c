#include <stdio.h>
#include <stdlib.h>
#include <mylloc.h>

struct Node {
    int data;
    struct Node* left;
    struct Node* right;
};

struct Node* create_node(int data) {
    struct Node* newNode = (struct Node*)mylloc(sizeof(struct Node));
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

struct Node* insert(struct Node* root, int data) {
    if (root == NULL) {
        return create_node(data);
    }

    if (data < root->data) {
        root->left = insert(root->left, data);
    } else if (data > root->data) {
        root->right = insert(root->right, data);
    }

    return root;
}

void inorder_traversal(struct Node* root) {
    if (root != NULL) {
        inorder_traversal(root->left);
        printf("%d ", root->data);
        inorder_traversal(root->right);
    }
}

void free_tree(struct Node* root) {
    if (root != NULL) {
        free_tree(root->left);
        free_tree(root->right);
        myfree(root);
    }
}

int main() {

    initializeAllocator();
    enableOutput();

    struct Node* root = NULL;
    root = insert(root, 50);
    insert(root, 30);
    insert(root, 20);
    insert(root, 40);
    insert(root, 70);
    insert(root, 60);
    insert(root, 80);

    printf("Inorder traversal of the binary search tree: ");
    inorder_traversal(root);
    printf("\n");

    free_tree(root);

    return 0;
}
