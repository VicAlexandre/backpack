/*!
 * @file bp_tree.h
 * @author Victor Miranda (victoralexandre499@gmail.com)
 * @brief Implements a generic n-ary tree structure module.
 * @version 0.1.0
 * @date 06/08/2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "bp_tree.h"

/*!
 * @brief The maximum number of children a tree node can have.
 */
#define BP_TREE_MAX_CHILDREN 128

int bp_tree_get_nth_child(struct bp_tree *ref, struct bp_tree **child, uint8_t child_index)
{
    if ((ref == NULL) || (child == NULL)) {
        return -EINVAL;
    }

    if ((ref->children_len <= child_index)) {
        return -EINVAL;
    }

    *child = ref->children[child_index];

    return 0;
}

int bp_tree_get_parent(struct bp_tree *root, struct bp_tree *child, struct bp_tree **parent)
{
    struct bp_tree *stack[BP_TREE_MAX_CHILDREN] = {0};
    struct bp_tree *top_node;
    uint8_t stack_top = 0;
    uint16_t i;

    if ((root == NULL) || (child == NULL) || (parent == NULL)) {
        return -EINVAL;
    }

    if (root == child) {
        return -EINVAL;
    }


    stack[stack_top] = root;
    stack_top += 1;

    while (stack_top > 0) {
        top_node = stack[stack_top - 1];
        stack_top -= 1;

        for (i = 0; i < top_node->children_len; i++) {
            if (top_node->children[i] == child) {
                *parent = top_node;
                return 0;
            }

            stack[stack_top] = top_node->children[i];
            stack_top += 1;
        }
    }

    return -ENOENT;
}

int bp_tree_get_nth_sibling_after(struct bp_tree *root, uint8_t index, struct bp_tree *ref, struct bp_tree **sibling)
{
    struct bp_tree *parent;
    uint8_t idx;
    int err;

    if ((root == NULL) || (ref == NULL) || (sibling == NULL)) {
        return -EINVAL;
    }

    err = bp_tree_get_parent(root, ref, &parent);
    if (err != 0) {
        return err;
    }

    for (idx = 0; idx < parent->children_len; idx++) {
        if (parent->children[idx] == ref) {
            break;
        }
    }

    if (index + idx >= parent->children_len) {
        return -ENOENT;
    }

    *sibling = parent->children[index + idx];

    return 0;
}

int bp_tree_get_nth_sibling_before(struct bp_tree *root, uint8_t index, struct bp_tree *ref, struct bp_tree **sibling)
{
    struct bp_tree *parent;
    uint8_t idx;
    int err;

    if ((root == NULL) || (ref == NULL) || (sibling == NULL)) {
        return -EINVAL;
    }

    err = bp_tree_get_parent(root, ref, &parent);
    if (err != 0) {
        return err;
    }

    for (idx = 0; idx < parent->children_len; idx++) {
        if (parent->children[idx] == ref) {
            break;
        }
    }

    if (idx - index < 0) {
        return -ENOENT;
    }

    *sibling = parent->children[idx - index];

    return 0;
}

int bp_tree_get_value(struct bp_tree *ref, void **value)
{
    if ((ref == NULL) || (value == NULL)) {
        return -EINVAL;
    }

    *value = ref->value;

    return 0;
}

int bp_tree_set_value(struct bp_tree *ref, void *value)
{
    if ((ref == NULL) || (value == NULL)) {
        return -EINVAL;
    }

    ref->value = value;

    return 0;
}
