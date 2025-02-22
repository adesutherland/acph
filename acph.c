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
    * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    * SOFTWARE.
    *
    * This file implements the core functionality of the Adaptive Columnar Perfect Hashing (ACPH) algorithm.
    * The ACPH algorithm is designed to create efficient hash tables for various data types, including strings,
    * integers, doubles, and binary data. The hash tables are built to minimize collisions and optimize lookup
    * performance.
    *
    * The file includes the following key components:
    *
    * 1. Data Structures:
    *    - HashSlot: Represents a slot in the hash table, containing a character, count, payload, and a union for
    *      either a child node or binary data.
    *    - HashNode: Represents a node in the hash table tree, containing column position, prime number for hashing,
    *      number of slots, and an array of HashSlot structures.
    *    - SLOT: A helper structure used to measure slots used by the find_best_hash function.
    *
    * 2. Hash Functions:
    *    - hash_function: Calculates the hash value for a given character.
    *    - calculate_character_distribution: Calculates the distribution of characters in an array.
    *    - find_best_hash: Generates the best hash table for the given characters.
    *    - create_character_hash: Builds a hash table for characters/bytes provided as binary & length.
    *    - lookup_character: Looks up a character in the hash node.
    *    - create_binary_hash: Builds the tree structure recursively from a set of binary buffers.
    *    - compare_binaries: Compares two binary values.
    *    - lookup_binary: Compares a binary against the tree structure.
    *
    * 3. Utility Functions:
    *    - free_tree: Frees the tree structure recursively.
    *    - print_tree: Recursively prints the tree structure using a provided print_leaf function.
    *    - print_string_leaf, print_int_leaf, print_double_leaf, print_char_leaf, print_binary_leaf: Helper functions
    *      to print different types of leaf nodes.
    *    - create_string_hash, create_integer_hash, create_double_hash: Functions to create hash tables for strings,
    *      integers, and doubles respectively.
    *    - lookup_string, lookup_integer, lookup_double: Functions to look up strings, integers, and doubles in the
    *      hash node.
    *    - hash_efficiency: Utility function to return the efficiency of the hash table.
    *    - hash_table_efficiency: Prints and returns the efficiency of the hash table.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "acph.h"

#define HASHNODE_SIZEFORNUMSLOTS(num_slots) sizeof(HashNode) + (((int)(num_slots) + 1) * sizeof(HashSlot))

// Slot structure for the hash table
typedef struct HashSlot {
    uint8_t character;     // Character in the slot
    int count;                               // Number of occurrences of the character (0 for empty slots, 1 for unique characters, >1 for a child node)
    union {
        struct HashNode *child;  // Pointer to child node (for next column)
        BinaryValue *binary;        // Pointer to binary - this is used to check if the binary is found
    } next_node;
    Payload payload;                 // Payload for the slot
} HashSlot;

// Node structure for the tree
struct HashNode {
    size_t column;         // Column position
    uint8_t prime;          // Prime number for hashing
    uint8_t num_slots;   // Number of slots in the hash table; zero based 0 = 1 slot, 255 = 256 slots
    HashSlot slot[];       // Slots in the hash table
};

/**
 * @brief Calculates the hash value for a given character.
 *
 * @param character The character to hash.
 * @param a The prime number used in the hash function.
 * @param num_slots The number of slots in the hash table (zero-based, so num_slots = 255 means 256 slots).
 * @return The calculated hash value.
 */
static uint8_t hash_function(uint8_t character, uint8_t a, uint8_t num_slots) {
    if (num_slots == 255) {
        return character; // Natural hash function for num_slots = 255
    }
    return (((a - 1) ^ character) * a) % (num_slots + 1); // Using XOR and multiplication
}

/**
 * @brief Calculates the distribution of characters in the given array.
 *
 * This function calculates two distribution values:
 * - The maximum number of occurrences of a single character.
 * - The number of different/unique characters.
 *
 * @param characters Pointer to the array of characters.
 * @param num_chars Number of characters in the array.
 * @param unique_chars Pointer to the variable to store the number of unique characters.
 * @param max_occurrence Pointer to the variable to store the maximum number of occurrences of a single character.
 */
static void calculate_character_distribution(const uint8_t *characters, size_t num_chars, size_t *unique_chars, size_t *max_occurrence) {
    size_t i;
    size_t char_counts[256] = {0};

    *max_occurrence = 0;

    // Count occurrences of each character
    for (i = 0; i < num_chars; i++) {
        uint8_t c = characters[i];
        char_counts[c]++;
        if (char_counts[c] > *max_occurrence) {
            *max_occurrence = char_counts[c];
        }
    }

    // Count the number of unique characters
    *unique_chars = 0;
    for (i = 0; i < 256; i++) {
        if (char_counts[i] > 0) {
            (*unique_chars)++;
        }
    }
}

// Work Structure to measure slots used by find_best_hash
typedef struct SLOT {
    uint8_t character; // Character in the slot
    int count;          // Number of occurrences of the character
} SLOT;

/**
 * @brief Generates the best hash table for the given characters.
 *
 * The hash table is stored in a dynamically allocated buffer.
 *
 * @param characters Pointer to the array of characters.
 * @param num_chars Number of characters in the array.
 * @param best_possible_score The best possible score for the hash table.
 * @param min_unique_chars The minimum number of unique characters.
 * @return Pointer to the root node of the created hash table.
 */
static HashNode* find_best_hash(uint8_t *characters, size_t num_chars,  size_t best_possible_score, size_t min_unique_chars) {

    // Prime number list for 'a'
    uint8_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
                    103, 107, 113, 127, 131, 137, 149, 151, 157, 163, 167, 173, 211, 223, 227, 229, 233, 239, 241, 251};
    int num_primes = sizeof(primes) / sizeof(primes[0]);
    uint8_t best_a = 0;
    uint8_t best_m = 0;
    size_t best_score;
    SLOT slot_table[256];
    SLOT best_slot_table[256];
    int i, j;
    uint8_t num_slots; // Zero based number of slots
    HashNode* hash_table;

    best_score = num_chars + 1; // Initialize with a high score
    num_slots = min_unique_chars - 1; // Initialize with the minimum number of unique characters
    do {
        num_slots++;
        for (i = 0; i < num_primes; i++) {
            int a = primes[i];
            // Initialize the slot table
            for (j = 0; j < num_slots + 1; j++) {
                slot_table[j].count = 0;
            }

            size_t differential_score = best_possible_score;
            int false_positive = 0;

            for (j = 0; j < num_chars; j++) {
                int slot = hash_function(characters[j], a, num_slots);
                if (slot_table[slot].count == 0) {
                    slot_table[slot].character = characters[j];
                    slot_table[slot].count = 1;
                } else {
                    if (slot_table[slot].character != characters[j]) {
                        false_positive = 1;
                        break;
                    } else {
                        slot_table[slot].count++;
                        if (slot_table[slot].count > differential_score) {
                            differential_score = slot_table[slot].count;
                        }
                    }
                }
            }

            if (!false_positive) {
                if (differential_score < best_score) {
                    best_score = differential_score;
                    best_a = a;
                    best_m = num_slots;
                    // Copy with a memcpy
                    memcpy(best_slot_table, slot_table, sizeof(SLOT) * (best_m + 1));
                }
                if (best_score == best_possible_score) {
                    // Found the best possible score
                    goto found_hash;
                }
            }
        } // end of for loop for 'a'
    } while (num_slots < 255);  // end of for loop for num_slots

    found_hash:

    hash_table = (HashNode *)malloc(HASHNODE_SIZEFORNUMSLOTS(best_m));
    if (hash_table == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }

    hash_table->prime = best_a;
    hash_table->num_slots = best_m;
    for (i = 0; i <= hash_table->num_slots; i++) {
        if (best_slot_table[i].count > 0) {
            hash_table->slot[i].count = best_slot_table[i].count;
            hash_table->slot[i].character = best_slot_table[i].character;
            hash_table->slot[i].payload = (Payload){0};
            hash_table->slot[i].next_node.child = NULL;
        } else {
            hash_table->slot[i].count = 0;
            hash_table->slot[i].character = 0;
            hash_table->slot[i].payload = (Payload){0};
            hash_table->slot[i].next_node.child = NULL;
        }
    }

    return hash_table;
}

/**
 * @brief Builds a hash table for characters/bytes provided as a binary & length.
 *
 * This function returns the best (smallest) hash table for the given characters.
 *
 * @param characters Pointer to the array of characters.
 * @param payloads Pointer to the array of payloads.
 * @param num_chars Number of characters in the array.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_character_hash(uint8_t *characters, Payload *payloads, size_t num_chars) {
    size_t best_possible_score;
    size_t min_unique_chars;
    HashNode *node;
    size_t i;

    calculate_character_distribution(characters, num_chars, &min_unique_chars, &best_possible_score);

    node = find_best_hash(characters, num_chars, best_possible_score, min_unique_chars);

    // For a character hash it is always perfect so any counts > 1 just means duplicate inputs - we set count to 1
    // Note the binary hash will need to know the counts > 1, which is why we clear them here and not in find_best_hash()
    for (i = 0; i <= node->num_slots; i++) {
        if (node->slot[i].count > 1) {
            node->slot[i].count = 1;
        }
    }

    // We need to set the payload for the slot
    // Loop through the characters, find the slot and set the payload
    for (i = 0; i < num_chars; i++) {
        int slot = hash_function(characters[i], node->prime, node->num_slots);
        if (node->slot[slot].count == 1 && node->slot[slot].character == characters[i]) {
            node->slot[slot].payload = payloads[i];
        }
    }

    return node;
}

/**
 * @brief Looks up a character in the hash node.
 *
 * This function returns the slot index if the character is found, -1 otherwise.
 *
 * @param character The character to look up.
 * @param node Pointer to the hash node.
 * @param payload_out Pointer to the variable to store the payload if the character is found.
 * @return The slot index if the character is found, -1 otherwise.
 */
int lookup_character(uint8_t character, const HashNode *node, Payload *payload_out) {
    int slot = hash_function(character, node->prime, node->num_slots);
    if (node->slot[slot].count > 0 && node->slot[slot].character == character) {
        if (payload_out != NULL) {
            *payload_out = node->slot[slot].payload;
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Builds the tree structure recursively from a set of binary buffers.
 *
 * This function creates the tree structure for the given binary values and payloads.
 *
 * @param values Pointer to the array of binary values.
 * @param payloads Pointer to the array of payloads.
 * @param num_values Number of binary values.
 * @return Pointer to the root node of the created hash table.
 */
HashNode *create_binary_hash(BinaryValue *values, Payload *payloads, size_t num_values) { // NOLINT
    if (num_values < 1) {
        return NULL; // No values to process
    }

    // Find the best column with the lowest 'num_slots' values
    uint8_t *column_chars = (uint8_t *)malloc(num_values * sizeof(uint8_t));
    if (column_chars == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    uint8_t *best_column_chars = (uint8_t *)malloc(num_values * sizeof(uint8_t));
    if (best_column_chars == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    size_t best_column = 0;
    size_t best_num_slots = num_values + 1; // Initialize with a high value
    size_t num_slots;
    size_t unique_chars, best_unique_chars;
    int c;
    size_t i, j;
    size_t last_column = 0;
    for (c = 0; !last_column; c++) {
        // Extract characters from the current column
        last_column = 1;
        uint8_t character;
        for (i = 0; i < num_values; i++) {
            if (values[i].length > c) {
                last_column = 0;
                character = values[i].binary[c];
            }
            else {
                character = 0;
            }
            column_chars[i] = character;
        }
        calculate_character_distribution(column_chars, num_values, &unique_chars, &num_slots);
        if (num_slots < best_num_slots) {
            best_column = c;
            best_num_slots = num_slots;
            best_unique_chars = unique_chars;
            memcpy(best_column_chars, column_chars, num_values * sizeof(uint8_t));
        }
    }
    free(column_chars);

    if (best_unique_chars == 1 && num_values > 1) {
        // All the characters in the best column are the same - so there must be a duplicate
        // Return NULL to signal the duplicate - which is an input error
        return NULL;
    }

    // Create a new node for the best column
    HashNode *node = find_best_hash(best_column_chars, num_values, best_num_slots, best_unique_chars);
    node->column = best_column;

    // Process values by hash values and create child nodes recursively
    for (i = 0; i <= node->num_slots; i++) {
        if (node->slot[i].count == 0) {
            node->slot[i].next_node.child = NULL;
        }
        else if (node->slot[i].count == 1) {
            int found = 0;
            // Find the binary that hashes to this slot
            for (j = 0; j < num_values; j++) {
                uint8_t character;
                if (node->column >= values[j].length) {
                    character = 0;
                }
                else {
                    character = (uint8_t) values[j].binary[node->column];
                }

                if (character == node->slot[i].character) {
                    // Found the binary that hashes to this slot
                    found = 1;
                    node->slot[i].next_node.binary = malloc(sizeof(BinaryValue));
                    if (node->slot[i].next_node.binary == NULL) {
                        // Handle memory allocation error - our standard is to exit with a PANIC message
                        fprintf(stderr, "PANIC: Memory allocation error\n");
                        exit(1);
                    }
                    memcpy(node->slot[i].next_node.binary, &values[j], sizeof(BinaryValue));
                    // Set the payload
                    node->slot[i].payload = payloads[j];
                    break;
                }
            }
            if (!found) {
                // Panic the binary is not found
                fprintf(stderr, "PANIC: Binary not found\n");
                exit(1);
            }
        }
        else if (node->slot[i].count > 0) {
            // Create a list of values that hash to this slot
            BinaryValue *grouped_strings = (BinaryValue *)malloc(node->slot[i].count * sizeof(BinaryValue));
            if (grouped_strings == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            // Create a list of payloads that hash to this slot
            Payload *grouped_payloads = (Payload *)malloc(node->slot[i].count * sizeof(Payload));
            if (grouped_payloads == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            int count = 0;
            for (j = 0; j < num_values; j++) {
                if ((uint8_t)values[j].binary[node->column] == node->slot[i].character) {
                    grouped_strings[count] = values[j];
                    grouped_payloads[count++] = payloads[j];
                }
            }

            // Recursively build the child node for this group
            node->slot[i].next_node.child = create_binary_hash(grouped_strings, grouped_payloads,count);
            free(grouped_strings);

            if (node->slot[i].next_node.child == NULL) {
                // NULL - means a duplicate has been found - an input error
                // Free any mallocs and return NULL
                free(grouped_payloads);
                for (j = 0; j <= i; j++) {
                    if (node->slot[j].count == 1) {
                        if (node->slot[j].next_node.binary) free(node->slot[j].next_node.binary);
                    }
                }
                free(node);
                return NULL;
            }

        }
    }

    free(best_column_chars);
    return node;
}

/**
 * @brief Compares two binary values.
 *
 * This function compares two binary values and returns 1 if they are equal, 0 otherwise.
 *
 * @param str1 Pointer to the first binary value.
 * @param str2 Pointer to the second binary value.
 * @return 1 if the binary values are equal, 0 otherwise.
 */
int compare_binaries(const BinaryValue *str1, const BinaryValue *str2) {
    if (str1->length != str2->length) {
        return 0;
    }
    return memcmp(str1->binary, str2->binary, str1->length) == 0;
}

/**
 * @brief Compares a binary against the tree structure.
 *
 * This function looks up a binary value in the hash node and returns 1 if found (and sets payload), 0 otherwise.
 *
 * @param str Pointer to the binary value to compare.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the binary is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_binary(const BinaryValue *str, const HashNode  *node, Payload *payload_out) { // NOLINT

    // Create hash value for the binary
    uint8_t character;
    size_t effective_column = node->column;
    if (effective_column >= str->length) {
        character = 0;
    }
    else {
        character = str->binary[effective_column];
    }
    int slot = hash_function(character, node->prime, node->num_slots);

    if (node->slot[slot].count == 0) {
        return 0; // No match
    }
    else if (node->slot[slot].count == 1) {
        // Leaf node, compare with the stored binary
        if (compare_binaries(str, node->slot[slot].next_node.binary)) {
            if (payload_out != NULL) {
                *payload_out = node->slot[slot].payload;
            }
            return 1;
        }
        else {
            return 0;
        }
    }
    else {
        // Traverse to the child node
        return lookup_binary(str, node->slot[slot].next_node.child, payload_out);
    }
}

/**
 * @brief Frees the tree structure.
 *
 * This function recursively frees the memory allocated for the hash table tree structure.
 *
 * @param node Pointer to the root node of the tree.
 */
void free_tree(HashNode *node) { // NOLINT
    int i;
    if (node == NULL) {
        return;
    }

    for (i = 0; i <= node->num_slots; i++) {
        if (node->slot[i].count > 1) {
            free_tree(node->slot[i].next_node.child);
        }
        else if (node->slot[i].count == 1) {
            free(node->slot[i].next_node.binary);
        }
    }
    free(node);
}

/**
 * @brief Recursively prints the tree structure using the provided print\_leaf function.
 *
 * @param node Pointer to the root node of the tree.
 * @param level Current level in the tree.
 * @param print_leaf Function to print a leaf node.
 */
void print_tree(HashNode *node, int level, print_leaf_func print_leaf) { // NOLINT
    int i, j;
    // Priint Column and Prime
    for (j = 0; j < level; j++) {
        printf("   ");
    }
    printf("Slots %d, Column: %d, Prime: %d\n", node->num_slots, (int)node->column, node->prime);
    // Print Slots
    for (i = 0; i <= node->num_slots; i++) {
        for (j = 0; j < level; j++) {
            printf("   ");
        }
        if (node->slot[i].count == 0) {
            printf("Slot %d: Empty\n", i);
        }
        else if (node->slot[i].count == 1) {

            if (node->slot[i].character < 32 || node->slot[i].character > 126) {
                printf("Slot %d: 0x%02x -> ", i, node->slot[i].character);
            } else {
                printf("Slot %d: 0x%02x ('%c') -> ", i, node->slot[i].character, node->slot[i].character);
            }
            print_leaf(node, i);
            printf("\n");
        }
        else {
            if (node->slot[i].character < 32 || node->slot[i].character > 126) {
                printf("Slot %d: 0x%02x ->\n", i, node->slot[i].character);
            }
            else {
                printf("Slot %d: 0x%02x ('%c') ->\n", i, node->slot[i].character, node->slot[i].character);
            }
            print_tree(node->slot[i].next_node.child, level + 1, print_leaf);
        }
    }
}

/**
 * @brief Helper function to print a string leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_string_leaf(const HashNode *node, int slot) {
    printf("'%.*s'", (int)node->slot[slot].next_node.binary->length, node->slot[slot].next_node.binary->binary);
}

/**
 * @brief Helper function to print an integer leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_int_leaf(const HashNode *node, int slot) {
    int64_t integer;
    integer = *(int64_t *)node->slot[slot].next_node.binary->binary;
    printf("%lld", integer);
}

/**
 * @brief Helper function to print a double leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_double_leaf(const HashNode *node, int slot) {
    double real;
    real = *(double *)node->slot[slot].next_node.binary->binary;
    printf("%f", real);
}

/**
 * @brief Helper function to print a character leaf node.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_char_leaf(const HashNode *node, int slot) {
    // Print the character as a character if it is printable
    if (node->slot[slot].character < 32 || node->slot[slot].character > 126) {
        printf("0x%02x", node->slot[slot].character);
    } else {
        printf("%c", (uint8_t)node->slot[slot].character);
    }
}

/**
 * @brief Helper function to print a binary leaf node - first 20 bytes in hex.
 *
 * @param node Pointer to the hash node.
 * @param slot Slot index in the hash table.
 */
void print_binary_leaf(const HashNode *node, int slot) {
    int i;
    printf("0x");
    for (i = 0; i < 20 && i < node->slot[slot].next_node.binary->length; i++) {
        printf("%02x", (uint8_t)node->slot[slot].next_node.binary->binary[i]);
    }
    if (node->slot[slot].next_node.binary->length > 20) {
        printf("...");
    }
}

/**
 * @brief Creates a hash table for a set of null-terminated strings.
 *
 * @param strings Pointer to the array of strings.
 * @param payloads Pointer to the array of payloads.
 * @param num_strings Number of strings.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_string_hash(uint8_t **strings, Payload *payloads, size_t num_strings) {
    // Convert the strings to binary values
    BinaryValue *values = malloc(num_strings * sizeof(BinaryValue));
    if (values == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    size_t i;
    for (i = 0; i < num_strings; i++) {
        values[i].binary = strings[i];
        values[i].length = strlen((char*)strings[i]);
    }
    HashNode *hash = create_binary_hash(values, payloads, num_strings);
    free(values);
    return hash;
}

/**
 * @brief Looks up a string in the hash node.
 *
 * @param str Pointer to the string to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the string is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_string(const uint8_t *str, const HashNode *node, Payload *payload_out) {
    BinaryValue value;
    value.binary = (uint8_t*)str;
    value.length = strlen((char*)str);
    return lookup_binary(&value, node, payload_out);
}

/**
 * @brief Creates a hash table for a set of integers.
 *
 * @param integers Pointer to the array of integers.
 * @param payloads Pointer to the array of payloads.
 * @param num_integers Number of integers.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_integer_hash(int64_t *integers, Payload *payloads, size_t num_integers) {
    // Convert the integers to binary values
    BinaryValue *values = malloc(num_integers * sizeof(BinaryValue));
    if (values == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    size_t i;
    for (i = 0; i < num_integers; i++) {
        values[i].binary = (uint8_t*)&integers[i];
        values[i].length = sizeof(integers[i]);
    }
    HashNode *hash = create_binary_hash(values, payloads, num_integers);
    free(values);
    return hash;
}

/**
 * @brief Looks up an integer in the hash node.
 *
 * @param integer Integer to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the integer is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_integer(int64_t integer, const HashNode *node, Payload *payload_out) {
    BinaryValue value;
    value.binary = (uint8_t*)&integer;
    value.length = sizeof(integer);
    return lookup_binary(&value, node, payload_out);
}

/**
 * @brief Creates a hash table for a set of doubles.
 *
 * @param doubles Pointer to the array of doubles.
 * @param payloads Pointer to the array of payloads.
 * @param num_doubles Number of doubles.
 * @return Pointer to the root node of the created hash table.
 */
HashNode* create_double_hash(double *doubles, Payload *payloads, size_t num_doubles) {
    // Convert the doubles to binary values
    BinaryValue *values = malloc(num_doubles * sizeof(BinaryValue));
    if (values == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    size_t i;
    for (i = 0; i < num_doubles; i++) {
        values[i].binary = (uint8_t*)&doubles[i];
        values[i].length = sizeof(doubles[i]);
    }
    HashNode *hash = create_binary_hash(values, payloads, num_doubles);
    free(values);
    return hash;
}

/**
 * @brief Looks up a double in the hash node.
 *
 * @param real Real number to look up.
 * @param node Pointer to the root node of the hash table.
 * @param payload_out Pointer to the payload to be set if the real number is found.
 * @return 1 if found (and sets payload), 0 otherwise.
 */
int lookup_double(double real, const HashNode *node, Payload *payload_out) {
    BinaryValue value;
    value.binary = (uint8_t*)&real;
    value.length = sizeof(real);
    return lookup_binary(&value, node, payload_out);
}

/**
 * @brief Utility function to return the efficiency of the hash table.
 *
 * This function returns the number of slots used, the number of empty slots, and the maximum number of comparisons needed to find a value.
 *
 * @param node Pointer to the root node of the hash table.
 * @param slots_used Pointer to the variable to store the number of slots used.
 * @param empty_slots Pointer to the variable to store the number of empty slots.
 * @param max_comparisons Pointer to the variable to store the maximum number of comparisons.
 */
static void hash_efficiency(const HashNode *node, size_t *slots_used, size_t *empty_slots, size_t *max_comparisons) { // NOLINT
    size_t i;
    size_t total_comparisons = 0;
    size_t max_comparisons_needed = 0;
    size_t total_slots = 0;
    size_t total_empty_slots = 0;
    for (i = 0; i <= node->num_slots; i++) {
        total_slots++;
        if (node->slot[i].count == 0) {
            total_empty_slots++;
        } else if (node->slot[i].count == 1) {
            total_comparisons++;
        } else {
            size_t child_slots_used, child_empty_slots, child_max_comparisons;
            hash_efficiency(node->slot[i].next_node.child, &child_slots_used, &child_empty_slots, &child_max_comparisons);
            total_slots += child_slots_used;
            total_empty_slots += child_empty_slots;
            total_comparisons += child_max_comparisons;
            if (child_max_comparisons > max_comparisons_needed) {
                max_comparisons_needed = child_max_comparisons;
            }
        }
    }
    *slots_used = total_slots;
    *empty_slots = total_empty_slots;
    *max_comparisons = max_comparisons_needed + 1;
}

/**
 * @brief Prints and returns the efficiency of the hash table.
 *
 * @param node Pointer to the root node of the hash table.
 * @param slot_efficiency Pointer to the variable to store slot efficiency.
 * @param max_comparisons Pointer to the variable to store the maximum number of comparisons.
 */
void hash_table_efficiency(const HashNode *node, int *slot_efficiency, size_t *max_comparisons) {
    size_t slots_used, empty_slots;
    hash_efficiency(node, &slots_used, &empty_slots, max_comparisons);
    *slot_efficiency = (int)(slots_used * 100 / (slots_used + empty_slots));
    printf("Slots used: %d, Slot efficiency: %d%%, Max comparisons: %d\n", (int)slots_used, *slot_efficiency, (int)*max_comparisons);
}