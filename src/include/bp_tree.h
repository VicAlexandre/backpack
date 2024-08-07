/*!
 * @file bp_tree.h
 * @author Victor Miranda (victoralexandre499@gmail.com)
 * @brief Defines a generic n-ary tree structure module. The tree operates by defining a list of pointers to children, a
 * length of children, and a generic value. Operations such as getting the nth child, getting the parent of a node, and
 * getting and setting the value of a node are supported.
 * @version 0.1.0
 * @date 06/08/2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BACKPACK_TREE_H
#define BACKPACK_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/*!
 * @brief Defines a leaf node in the n-ary tree.
 *
 * @param type The data type of the leaf node.
 * @param name The name of the leaf node variable.
 * @param init_value The initial value of the leaf node.
 */
#define BP_TREE_LEAF_DEF(type, name, init_value)                                                                       \
    type name##_value   = init_value;                                                                                  \
    struct bp_tree name = {                                                                                            \
        .children     = NULL,                                                                                          \
        .children_len = 0,                                                                                             \
        .value        = &name##_value,                                                                                 \
    }

/*!
 * @brief Defines a tree node in the n-ary tree with multiple children.
 *
 * @param name The name of the tree node variable.
 * @param length The number of children in the tree node.
 * @param ... The children of the tree node.
 */
#define BP_TREE_DEF(name, length, ...)                                                                                 \
    struct bp_tree *name##_array[] = {__VA_ARGS__};                                                                    \
    struct bp_tree name            = {                                                                                 \
                   .children     = name##_array,                                                                       \
                   .children_len = length,                                                                             \
                   .value        = NULL,                                                                               \
    }

/*!
 * @brief Defines a leaf node and appends it to the parent node.
 */
#define BP_TREE_LEAF_APPEND(parent, type, name, init_value)                                                            \
    BP_TREE_LEAF_DEF(type, name, init_value);                                                                          \
    parent->children[parent->children_len++] = &name;

/*!
 * @brief A generic n-ary tree structure.
 * @note While the theoretical limit of children is 2^8 (uint8_t), the practical limit is 128 and all functions in this
 * module are designed to handle this limit.
 */
struct bp_tree {
    struct {
        struct bp_tree **children; /*!< The children of this node. */
        uint8_t children_len;      /*!< The number of children. */
    };
    void *value; /*!< The value of this node. */
};

/*!
 * @brief Retrieves the nth child of a given parent node.
 *
 * @param ref Pointer to the parent node of the nth child to be retrieved.
 * @param[out] child Reference to the pointer that will store the nth child.
 * @param child_index Index of the child to be retrieved.
 * @retval 0 Success.
 * @retval -EINVAL Null pointer or invalid child index.
 */
int bp_tree_get_nth_child(struct bp_tree *ref, struct bp_tree **child, uint8_t child_index);

/*!
 * @brief Retrieves the parent of a given node.
 *
 * @param root The root of the tree.
 * @param child Pointer to the child node.
 * @param[out] parent Pointer to the pointer that will store the parent node.
 * @return 0 on success, -ERRNO otherwise.
 */
int bp_tree_get_parent(struct bp_tree *root, struct bp_tree *child, struct bp_tree **parent);

/*!
 * @brief Retrieves the nth sibling after a given node.
 * @note If the given index is 0, the function will return the node itself.
 * @param root The root of the tree.
 * @param index Index of the sibling to be retrieved.
 * @param ref Pointer to the reference node.
 * @param[out] sibling Pointer to the pointer that will store the nth sibling.
 * @return 0 on success, -ERRNO otherwise.
 */
int bp_tree_get_nth_sibling_after(struct bp_tree *root, uint8_t index, struct bp_tree *ref, struct bp_tree **sibling);

/*!
 * @brief Retrieves the nth sibling before a given node.
 * @note If the given index is 0, the function will return the node itself.
 * @param root The root of the tree.
 * @param index Index of the sibling to be retrieved.
 * @param ref Pointer to the reference node.
 * @param[out] sibling Pointer to the pointer that will store the nth sibling.
 * @return 0 on success, -ERRNO otherwise.
 */
int bp_tree_get_nth_sibling_before(struct bp_tree *root, uint8_t index, struct bp_tree *ref, struct bp_tree **sibling);


/*!
 * @brief Retrieves the value of a given node.
 *
 * @param ref Pointer to the node.
 * @param[out] value Reference to the pointer that will store the value.
 * @retval 0 Success.
 * @retval -EINVAL Null pointer.
 */
int bp_tree_get_value(struct bp_tree *ref, void **value);

/*!
 * @brief Sets the value of a given node.
 *
 * @param ref Pointer to the node.
 * @param value New address of the value.
 * @retval 0 Success.
 * @retval -EINVAL Null pointer.
 */
int bp_tree_set_value(struct bp_tree *ref, void *value);


#ifdef __cplusplus
}
#endif

#endif /* BACKPACK_TREE_H */
