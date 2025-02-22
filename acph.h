/*
 * Adaptive Columnar Perfect Hashing (ACPH)
 *
 * MIT License
 *
 * Copyright (c) 2025 Adrian Sutherland
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ACPH_H
#define ACPH_H

#include <stdint.h>
#include <stddef.h>

// Binary structure with length
typedef struct BinaryValue {
    uint8_t *binary;  // Pointer to the binary data
    size_t length;    // Length of the binary data
} BinaryValue;

// Payload structure for the hash table - this is what is returned when a value is found
typedef union {
    void *pointer;    // Pointer payload
    int64_t integer;  // Integer payload
    double real;      // Real payload
    uint8_t character; // Character payload
} Payload;

typedef struct HashNode HashNode;

/**
 * @brief Frees the tree structure.
 *
 * @param node Pointer to the root node of the tree.
 */
void free_tree(HashNode *node);

/**
 * @brief Typedef for helper functions that print leaf nodes.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
typedef void (*print_leaf_func)(const HashNode *node, int slot);

/**
 * @brief Recursively prints the tree structure using the provided print_leaf function.
 *
 * @param node Pointer to the root node of the tree.
 * @param level Current level in the tree.
 * @param print_leaf Function to print a leaf node.
 */
void print_tree(HashNode *node, int level, print_leaf_func print_leaf);

/**
 * @brief Helper function to print a string leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_string_leaf(const HashNode *node, int slot);

/**
 * @brief Helper function to print an integer leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_int_leaf(const HashNode *node, int slot);

/**
 * @brief Helper function to print a double leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_double_leaf(const HashNode *node, int slot);

/**
 * @brief Helper function to print a character leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_char_leaf(const HashNode *node, int slot);

/**
 * @brief Helper function to print a binary leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_binary_leaf(const HashNode *node, int slot);

/**
 * @brief Creates a hash table for characters/bytes provided as a byte buffer & length.
 *
 * @param characters Pointer to the array of characters.
 * @param payloads Pointer to the array of payloads.
 * @param num_chars Number of characters.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_character_hash(uint8_t *characters, Payload *payloads, size_t num_chars);

/**
 * @brief Looks up a character in the hash node.
 *
 * @param character Character to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the character is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_character(uint8_t character, const HashNode *node, Payload *payload_out);

/**
 * @brief Creates the tree structure from a set of binary buffers.
 *
 * @param values Pointer to the array of binary values.
 * @param payloads Pointer to the array of payloads.
 * @param num_values Number of binary values.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_binary_hash(BinaryValue *values, Payload *payloads, size_t num_values);

/**
 * @brief Compares a binary against the tree structure.
 *
 * @param str Pointer to the binary value to compare.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the binary is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_binary(const BinaryValue *str, const HashNode *node, Payload *payload_out);

/**
 * @brief Creates a hash table for a set of null-terminated strings.
 *
 * @param strings Pointer to the array of strings.
 * @param payloads Pointer to the array of payloads.
 * @param num_strings Number of strings.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_string_hash(uint8_t **strings, Payload *payloads, size_t num_strings);

/**
 * @brief Looks up a string in the hash node.
 *
 * @param str Pointer to the string to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the string is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_string(const uint8_t *str, const HashNode *node, Payload *payload_out);

/**
 * @brief Creates a hash table for a set of integers.
 *
 * @param integers Pointer to the array of integers.
 * @param payloads Pointer to the array of payloads.
 * @param num_integers Number of integers.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_integer_hash(int64_t *integers, Payload *payloads, size_t num_integers);

/**
 * @brief Looks up an integer in the hash node.
 *
 * @param integer Integer to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the integer is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_integer(int64_t integer, const HashNode *node, Payload *payload_out);

/**
 * @brief Creates a hash table for a set of real numbers (doubles).
 *
 * @param reals Pointer to the array of real numbers.
 * @param payloads Pointer to the array of payloads.
 * @param num_doubles Number of real numbers.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_double_hash(double *reals, Payload *payloads, size_t num_doubles);

/**
 * @brief Looks up a double in the hash node.
 *
 * @param real Real number to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the real number is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_double(double real, const HashNode *node, Payload *payload_out);

/**
 * @brief Prints and returns the efficiency of the hash table.
 *
 * @param node Pointer to the root node of the hash table.
 * @param slot_efficiency Pointer to the variable to store slot efficiency.
 * @param max_comparisons Pointer to the variable to store the maximum number of comparisons.
 */
void hash_table_efficiency(const HashNode *node, int *slot_efficiency, size_t *max_comparisons);

#endif // ACPH_H