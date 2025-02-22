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
  * This file contains test scripts for the library.
  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "acph.h"

char* print_char(uint8_t c) {
    static char buffer[15];
    if (c < 32 || c > 126) {
        sprintf(buffer, "0x%02x", c);
    }
    else {
        sprintf(buffer, "0x%02x (%c)", c, c);
    }
    return buffer;
}

int a_binary_test(BinaryValue *values, size_t num_values) {
    int i;
    int errors = 0;
    Payload payload;
    Payload  *payloads = (Payload *)malloc(num_values * sizeof(Payload));

    // Create payloads
    for (i = 0; i < num_values; i++) {
        payloads[i].integer = i;
    }

    /* Check the binary values and detect any duplicates */
    int has_duplicates = 0;
    for (i = 0; i < num_values; i++) {
        int j;
        for (j = i + 1; j < num_values; j++) {
            if (values[i].length == values[j].length && memcmp(values[i].binary, values[j].binary, values[i].length) == 0) {
                has_duplicates = 1;
            }
        }
    }

    HashNode* root = create_binary_hash(values, payloads, num_values);
    if (root == NULL) {
        if (has_duplicates) {
            return 0;
        }
        else {
            printf("Error creating binary hash - indicated a duplicates in the input but there are no duplicates\n");
            return 1;
        }
    }
    if (has_duplicates) {
        printf("Error hash created despite duplicates\n");
        return 1;
    }
    {
        int slot_efficiency;
        size_t max_comparisons;
        hash_table_efficiency(root, &slot_efficiency, &max_comparisons);
    }

    // Test a some missing values
    {
        char *never_find = "NeverAValidValueInTheseTests";
        BinaryValue search_string;
        search_string.binary = (uint8_t *) never_find;
        search_string.length = strlen(never_find);
        if (lookup_binary(&search_string, root, &payload)) {
            printf("Error '%s' found!\n", never_find);
            errors++;
        }
    }
    {
        char *never_find = "AnotherNeverAValidValueInTheseTests";
        BinaryValue search_string;
        search_string.binary = (uint8_t *) never_find;
        search_string.length = strlen(never_find);
        if (lookup_binary(&search_string, root, &payload)) {
            printf("Error '%s' found!\n", never_find);
            errors++;
        }
    }
    {
        char *never_find = "YetAnotherNeverNotValid";
        BinaryValue search_string;
        search_string.binary = (uint8_t *) never_find;
        search_string.length = strlen(never_find);
        if (lookup_binary(&search_string, root, &payload)) {
            printf("Error '%s' found!\n", never_find);
            errors++;
        }
    }

    // Search for all the values
    for (i = 0; i < num_values; i++) {
        if (!lookup_binary(&values[i], root, &payload)) {
            printf("Error '%.*s' not found!\n", (int)values[i].length, values[i].binary);
            errors++;
        }
        else if (payloads[i].integer != payload.integer) {
            printf("Error found but expected payload %d but got %d\n", (int)payloads[i].integer, (int)payload.integer);
            errors++;
        }
    }

    free_tree(root);
    return errors;
}

// Helper function to convert an array of strings to an array of BinaryValue
BinaryValue *strings_to_binary(char **strings, size_t num_strings) {
    int i;
    BinaryValue *strings_with_length = (BinaryValue *)malloc(num_strings * sizeof(BinaryValue));
    if (strings_with_length == NULL) {
        // Handle memory allocation error - our standard is to exit with a PANIC message
        fprintf(stderr, "PANIC: Memory allocation error\n");
        exit(1);
    }
    for (i = 0; i < num_strings; i++) {
        strings_with_length[i].binary = (uint8_t*)(strings[i]);
        strings_with_length[i].length = strlen(strings[i]);
    }
    return strings_with_length;
}

/* Sanity check */
int test_binaries() {
   int errors = 0;
    BinaryValue *strings_with_length;
    size_t num_strings;

    printf("Testing BinaryValue Hashing\n");

    char *test1[] = {
            "Mr Smith", "Mr Jones", "Ms Leonard", "Ms James", "Mrs Peabody", "Mr Smile"
    };
    num_strings = sizeof(test1) / sizeof(test1[0]);

    // Strings with length
    strings_with_length = strings_to_binary(test1, num_strings);
    errors += a_binary_test(strings_with_length, num_strings);
    free(strings_with_length);

    return errors;
}

/* Full test of binary values */
int full_test_binary() {
    int errors = 0;
    BinaryValue *strings_with_length;
    size_t num_strings;

    printf("Testing BinaryValue Hashing - Edge Cases\n");
    // Edge Cases
    // Test with a single character
    {
        char *test[] = {"A"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with a single string
    {
        char *test[] = {"AB"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with no strings
    {
        char *test[] = {""};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with 2 identical strings
    {
        char *test[] = {"AB", "AB"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with a few strings with two being identical
    {
        char *test[] = {"AB", "ABC", "AB", "ABCD", "ABCDE"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with a few different length  strings
    {
        char *test[] = {"AB", "ABC", "ABCD", "ABCDE", "ABCDEF"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }
    // Test with a lot of values - 1000 strings - but with common prefixes
    {
        char *test[1000];
        int i;
        for ( i = 0; i < 1000; i++) {
            // Create strings with a common prefix
            test[i] = (char *)malloc(100);
            if (test[i] == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            sprintf(test[i], "PrefixString%d", i);

        }
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        // Free the strings
        for (i = 0; i < 1000; i++) {
            free(test[i]);
        }
        free(strings_with_length);
    }
    // Test with a lot of values - 1000 strings - all very different
    {
        char *test[1000];
        int i;
        int j;
        // Seed the random number generator to keep the test consistent
        srand(0); //NOLINT(cert-msc51-cpp)
        for ( i = 0; i < 1000; i++) {
            // Create random strings - each with an appended number to make unique.
            test[i] = (char *)malloc(100);
            if (test[i] == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            /* Add a random number of characters - 1 to 90 in length - first decide the length then add 'a' to 'z'  for each character */
            int length = rand() % 90 + 1;  // NOLINT
            for (j = 0; j < length; j++) {
                test[i][j] =  rand() % 26; // NOLINT
                test[i][j] += 'a';
            }
            test[i][length] = '\0';
            // Now append the number to make it unique
            sprintf(test[i], "%s-%d", test[i], i);
        }
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        // Free the strings
        for (i = 0; i < 1000; i++) {
            free(test[i]);
        }
        free(strings_with_length);
    }

    // Test with a zero length value
    {
        char *test[] = {"Mr Smith", "Mr Jones", "", "Ms James", "Mrs Peabody", "Mr Smile"};
        num_strings = sizeof(test) / sizeof(test[0]);
        strings_with_length = strings_to_binary(test, num_strings);
        errors += a_binary_test(strings_with_length, num_strings);
        free(strings_with_length);
    }

    return errors;
}

int a_character_test(uint8_t * characters, size_t num_chars) {
    int errors = 0;
    uint8_t c;
    HashNode *hash_table;
    size_t i;
    Payload payload;
    Payload *payloads = (Payload *)malloc(num_chars * sizeof(Payload));
    Payload char_payloads[256];

    // Initialize payloads
    for (i = 0; i < 256; i++) {
        char_payloads[i].integer = -1;
    }

    // Create payloads
    for (i = 0; i < num_chars; i++) {
        c = characters[i];
        if (char_payloads[c].integer == -1) {
            char_payloads[c].integer = c;
        }
        payloads[i] = char_payloads[c];
    }

    hash_table = create_character_hash(characters, payloads, num_chars);

    {
        int slot_efficiency;
        size_t max_comparisons;
        hash_table_efficiency(hash_table, &slot_efficiency, &max_comparisons);
    }

    // Test the characters with the hash table
    int passed = 1;
    for (i = 0; i < num_chars; i++) {
        int found = lookup_character(characters[i], hash_table, &payload);
        if (!found) {
            passed = 0;
            printf("Character: %s not found (Error)\n", print_char(characters[i]));
        }
        else if (payloads[i].integer != payload.integer) {
            passed = 0;
            printf("Character: %s found but the expected payload is %d we got %d\n", print_char(characters[i]), (int)payloads[i].integer, (int)payload.integer);
        }
    }

    // Test not-found characters:
    // Test all other characters (0-255) - if char_payloads.index is -1 then it was not in the test set and
    // should not be found
    for (i = 0; i < 256; i++) {
        if (char_payloads[i].integer == -1) {
            int found = lookup_character(i, hash_table, &payload);
            if (found) {
                passed = 0;
                printf("Character: %s found but it should not be found (Error)\n", print_char(i));
            }
        }
    }

    if (!passed) {
        printf("There were character errors\n");
        errors++;
    }

    free(payloads);
    free(hash_table);

    return errors;
}

/* Sanity check */
int test_chars() {
    char *characters;
    int errors = 0;

    printf("Testing Byte Hashing\n");

    characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    errors += a_character_test((uint8_t*)characters, strlen(characters));

    characters = "AXY178bxyTQFpq";
    errors += a_character_test((uint8_t*)characters, strlen(characters));

    characters = "jutsl98";
    errors += a_character_test((uint8_t*)characters, strlen(characters));

    if (errors != 0) {
        printf("There were %d errors\n", errors);
    }
    return errors;
}

// Full Test of characters - edge cases and stress tests
int full_test_characters() {
    char *test;
    int errors = 0;

    printf("Testing Byte Hashing - Edge Cases\n");
    // Test with a single character
    test = "A";
    errors += a_character_test((uint8_t*)test, 1);

    // Test with two characters
    test = "AB";
    errors += a_character_test((uint8_t*)test, 2);

    // Test with no characters
    test = "";
    errors += a_character_test((uint8_t*)test, 0);

    // Test with 256 characters
    // 256 Hex string with 0x00 to 0xff
    test = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
           "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d"
           "\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c"
           "\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b"
           "\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b"
           "\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b"
           "\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b"
           "\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b"
           "\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b"
           "\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b"
           "\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab"
           "\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb"
           "\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb"
           "\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb"
           "\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb"
           "\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb"
           "\xfc\xfd\xfe\xff";

    errors += a_character_test((uint8_t*)test, 256);

    // Test with 100 odd charcters with a few duplicates
    test = "AABCDDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    errors += a_character_test((uint8_t*)test, strlen(test));

    if (!errors) {
        printf("There were %d errors in full character tests\n", errors);
    }
    return errors;
}


// Test a set of null terminated strings
int a_string_test(char **strings, size_t num_strings) {
    int errors = 0;
    HashNode *hash_table;
    int i;
    Payload payload;
    Payload *payloads = (Payload *)malloc(num_strings * sizeof(Payload));

    // Create payloads
    for (i = 0; i < num_strings; i++) {
        payloads[i].integer = i;
    }

    // Check for duplicates
    int has_duplicates = 0;
    for (i = 0; i < num_strings; i++) {
        int j;
        for (j = i + 1; j < num_strings; j++) {
            if (strcmp(strings[i], strings[j]) == 0) {
                has_duplicates = 1;
            }
        }
    }

    hash_table = create_string_hash((uint8_t**)strings, payloads, num_strings);
    if (hash_table == NULL) {
        if (has_duplicates) {
            return 0;
        }
        else {
            printf("Error creating string hash - indicated a duplicates in the input but there are no duplicates\n");
            return 1;
        }
    }
    else if (has_duplicates) {
        printf("Error hash created despite duplicates\n");
        return 1;
    }

    {
        int slot_efficiency;
        size_t max_comparisons;
        hash_table_efficiency(hash_table, &slot_efficiency, &max_comparisons);
    }

    // Test with a not found string
    {
        char *never_find = "NeverAValidValueInTheseTests";
        if (lookup_string((uint8_t*)never_find, hash_table, &payload)) {
            printf("Error '%s' found!\n", never_find);
            errors++;
        }
    }

    // Test the strings with the hash table
    int passed = 1;
    for (i = 0; i < num_strings; i++) {
        int found = lookup_string((uint8_t*)strings[i], hash_table, &payload);
        if (!found) {
            passed = 0;
            printf("String: %s not found (Error)\n", strings[i]);
        }
        else if (payloads[i].integer != payload.integer) {
            passed = 0;
            printf("String: %s found but expected payload %d but got %d\n", strings[i], (int)payloads[i].integer, (int)payload.integer);
        }
    }
    if (!passed) {
        printf("There were errors\n");
        errors++;
    }

    free(hash_table);

    return errors;
}

// Test null terminated strings
int test_strings() {
    char *test1[] = {
            "Mr Smith", "Mr Jones", "Ms Leonard", "Ms James", "Mrs Peabody", "Mr Smile"
    };
    size_t num_strings = sizeof(test1) / sizeof(test1[0]);
    int errors = 0;

    printf("Testing String Hashing\n");

    errors += a_string_test(test1, num_strings);

    if (errors != 0) {
        printf("There were %d errors\n", errors);
    }
    return errors;
}

// Full Test of strings
int full_test_strings() {
    int errors = 0;

    printf("Testing String Hashing - Edge Cases\n");
    // Test with a single character
    {
        char *test[] = {"A"};
        errors += a_string_test(test, 1);
    }
    // Test with a single string
    {
        char *test[] = {"AB"};
        errors += a_string_test(test, 1);
    }
    // Test with 2 identical strings
    {
        char *test[] = {"AB", "AB"};
        errors += a_string_test(test, 2);
    }
    // Test with a few strings with two being identical
    {
        char *test[] = {"AB", "ABC", "AB", "ABCD", "ABCDE"};
        errors += a_string_test(test, 5);
    }
    // Test with a few different length  strings
    {
        char *test[] = {"AB", "ABC", "ABCD", "ABCDE", "ABCDEF"};
        errors += a_string_test(test, 5);
    }
    // Test with a lot of values - 1000 strings - but with common prefixes
    {
        char *test[1000];
        int i;
        for (i = 0; i < 1000; i++) {
            // Create strings with a common prefix
            test[i] = (char *) malloc(100);
            if (test[i] == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            sprintf(test[i], "PrefixString%d", i);

        }
        errors += a_string_test(test, 1000);
        // Free the strings
        for (i = 0; i < 1000; i++) {
            free(test[i]);
        }
    }

    // Test with a lot of values - 1000 strings - all very different
    {
        char *test[1000];
        int i;
        int j;
        // Seed the random number generator to keep the test consistent
        srand(0); //NOLINT
        for (i = 0; i < 1000; i++) {
            // Create random strings - each with an appended number to make unique.
            test[i] = (char *) malloc(100);
            if (test[i] == NULL) {
                // Handle memory allocation error - our standard is to exit with a PANIC message
                fprintf(stderr, "PANIC: Memory allocation error\n");
                exit(1);
            }
            /* Add a random number of characters - 1 to 90 in length - first decide the length then add 'a' to 'z'  for each character */
            int length = rand() % 90 + 1; //NOLINT
            for (j = 0; j < length; j++) {
                test[i][j] =  rand() % 26; //NOLINT
                test[i][j] += 'a';
            }
            test[i][length] = '\0';
            // Now append the number to make it unique
            sprintf(test[i], "%s-%d", test[i], i);
        }
        errors += a_string_test(test, 1000);
        // Free the strings
        for (i = 0; i < 1000; i++) {
            free(test[i]);
        }
    }

    return errors;
}

// Test a set of integers
int a_integer_test(int64_t *integers, size_t num_integers) {
    int errors = 0;
    HashNode *hash_table;
    int i;
    Payload payload;
    Payload *payloads = (Payload *)malloc(num_integers * sizeof(Payload));

    // Create payloads
    for (i = 0; i < num_integers; i++) {
        payloads[i].integer = i;
    }

    // Check for duplicates
    int has_duplicates = 0;
    for (i = 0; i < num_integers; i++) {
        int j;
        for (j = i + 1; j < num_integers; j++) {
            if (integers[i] == integers[j]) {
                has_duplicates = 1;
            }
        }
    }

    hash_table = create_integer_hash(integers, payloads, num_integers);
    if (hash_table == NULL) {
        if (has_duplicates) {
            return 0;
        }
        else {
            printf("Error creating integer hash - indicated a duplicates in the input but there are no duplicates\n");
            return 1;
        }
    }
    else if (has_duplicates) {
        printf("Error hash created despite duplicates\n");
        return 1;
    }

    {
        int slot_efficiency;
        size_t max_comparisons;
        hash_table_efficiency(hash_table, &slot_efficiency, &max_comparisons);
    }

    // Test the integers with the hash table
    int passed = 1;
    for (i = 0; i < num_integers; i++) {
        int found = lookup_integer(integers[i], hash_table, &payload);
        if (!found) {
            passed = 0;
            printf("Integer: %lld not found (Error)\n", (long long)integers[i]);
        }
        else if (payloads[i].integer != payload.integer) {
            passed = 0;
            printf("Integer: %lld found but expected payload %d but got %d\n", (long long)integers[i], (int)payloads[i].integer, (int)payload.integer);
        }
    }
    if (!passed) {
        printf("There were errors\n");
        errors++;
    }

    free(hash_table);

    return errors;
}

// Test integers
int test_integers() {
    int64_t test1[] = {
            1, 2, 3, 4, 5, 6, 7, 8, 9000, 100000
    };
    size_t num_integers = sizeof(test1) / sizeof(test1[0]);
    int errors = 0;

    printf("Testing Integer Hashing\n");

    errors += a_integer_test(test1, num_integers);

    if (errors != 0) {
        printf("There were %d errors\n", errors);
    }
    return errors;
}

// Full Test of integers
int full_test_integers() {
    int errors = 0;

    printf("Testing Integer Hashing - Edge Cases\n");
    // Test with a single integer
    {
        int64_t test[] = {1};
        errors += a_integer_test(test, 1);
    }
    // Test with two integers
    {
        int64_t test[] = {1, 2};
        errors += a_integer_test(test, 2);
    }
    // Test with 2 identical integers
    {
        int64_t test[] = {1, 1};
        errors += a_integer_test(test, 2);
    }
    // Test with a few integers with two being identical
    {
        int64_t test[] = {1, 2, 1, 3, 4};
        errors += a_integer_test(test, 5);
    }
    // Test with a few different integers
    {
        int64_t test[] = {1, 2, 3, 4, 5};
        errors += a_integer_test(test, 5);
    }
    // Test with a lot of values - 1000 integers - all very different
    {
        int64_t test[1000];
        int i;
        // Seed the random number generator to keep the test consistent
        srand(0); //NOLINT
        int64_t max = 0;
        for (i = 0; i < 1000; i++) {
            // Create random integers  - all unique
            test[i] = max + rand(); //NOLINT
            max = test[i];
        }
        errors += a_integer_test(test, 1000);
    }


    return errors;
}

// Test a set of doubles
int a_double_test(double *doubles, size_t num_doubles) {
    int errors = 0;
    HashNode *hash_table;
    int i;
    Payload payload;
    Payload *payloads = (Payload *)malloc(num_doubles * sizeof(Payload));

    // Create payloads
    for (i = 0; i < num_doubles; i++) {
        payloads[i].integer = i;
    }

    // Check for duplicates
    int has_duplicates = 0;
    for (i = 0; i < num_doubles; i++) {
        int j;
        for (j = i + 1; j < num_doubles; j++) {
            if (doubles[i] == doubles[j]) {
                has_duplicates = 1;
            }
        }
    }

    hash_table = create_double_hash(doubles, payloads, num_doubles);
    if (hash_table == NULL) {
        if (has_duplicates) {
            return 0;
        }
        else {
            printf("Error creating double hash - indicated a duplicates in the input but there are no duplicates\n");
            return 1;
        }
    }
    else if (has_duplicates) {
        printf("Error hash created despite duplicates\n");
        return 1;
    }

    {
        int slot_efficiency;
        size_t max_comparisons;
        hash_table_efficiency(hash_table, &slot_efficiency, &max_comparisons);
    }

    // Test the doubles with the hash table
    int passed = 1;
    for (i = 0; i < num_doubles; i++) {
        int found = lookup_double(doubles[i], hash_table, &payload);
        if (!found) {
            passed = 0;
            printf("Double: %f not found (Error)\n", doubles[i]);
        }
        else if (payloads[i].integer != payload.integer) {
            passed = 0;
            printf("Double: %f found but expected payload %d but got %d\n", doubles[i], (int)payloads[i].integer, (int)payload.integer);
        }
    }
    if (!passed) {
        printf("There were errors\n");
        errors++;
    }

    free(hash_table);

    return errors;
}

// Test doubles
int test_doubles() {
    double test1[] = {
            1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9000.9, 100000.1
    };
    size_t num_doubles = sizeof(test1) / sizeof(test1[0]);
    int errors = 0;

    printf("Testing Double Hashing\n");

    errors += a_double_test(test1, num_doubles);

    if (errors != 0) {
        printf("There were %d errors\n", errors);
    }
    return errors;
}

// Full Test of doubles
int full_test_doubles() {
    int errors = 0;

    printf("Testing Double Hashing - Edge Cases\n");
    // Test with a single double
    {
        double test[] = {1.1};
        errors += a_double_test(test, 1);
    }
    // Test with two doubles
    {
        double test[] = {1.1, 2.2};
        errors += a_double_test(test, 2);
    }
    // Test with 2 identical doubles
    {
        double test[] = {1.1, 1.1};
        errors += a_double_test(test, 2);
    }
    // Test with a few doubles with two being identical
    {
        double test[] = {1.1, 2.2, 1.1, 3.3, 4.4};
        errors += a_double_test(test, 5);
    }
    // Test with a few different doubles
    {
        double test[] = {1.1, 2.2, 3.3, 4.4, 5.5};
        errors += a_double_test(test, 5);
    }
    // Test with a lot of values - 1000 doubles - all very different
    {
        double test[1000];
        int i;
        // Seed the random number generator to keep the test consistent
        srand(0); //NOLINT
        double max = 1.1;
        for (i = 0; i < 1000; i++) {
            // Create random doubles  - all unique
            test[i] = (double)rand() * (double)rand() / (double)rand(); //NOLINT
            test[i] = test[i] +  max;
            max = test[i];
        }
        errors += a_double_test(test, 1000);
    }

    return errors;
}

// Main Test Function
int main() {
    int errors = 0;

    /* Sanity check */
    printf("Sanity Testing ACPH\n");
    errors += test_chars();
    errors += test_binaries();
    errors += test_strings();
    errors += test_integers();
    errors += test_doubles();

    // Full Test
    printf("Full Testing ACPH\n");
    errors += full_test_characters();
    errors += full_test_binary();
    errors += full_test_strings();
    errors += full_test_integers();
    errors += full_test_doubles();

    if (errors == 0) {
        printf("All tests passed\n");
    } else {
        printf("There were %d errors\n", errors);
    }
    return errors;
}
