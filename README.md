# Adaptive Columnar Perfect Hashing (ACPH) Demonstrator

## Overview

This project is a demonstrator for the Adaptive Columnar Perfect Hashing (ACPH) algorithm. The purpose of this demonstrator is to showcase the implementation and effectiveness of the ACPH algorithm in efficiently handling binary matching and indexing tasks.

## ALGORITHM DESCRIPTION

Please refer to the `ACPH.md` file for a detailed description of the ACPH algorithm.

## Scope and Purpose

The ACPH demonstrator is designed to:

-   Illustrate the implementation of the ACPH algorithm.
-   Provide a practical example of how the algorithm can be applied to various data types, including strings, integers, and floating-point numbers.
-   Evaluate the effectiveness of the algorithm in terms of slot efficiency and search depth.

## Usage

### Building the Project

To build the project, use the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

### Running Tests

To run the tests, use the following command:

```bash
ctest
```

### Example Usage

The demonstrator includes functions to create hash tables for different data types and to look up values in these hash tables. Here are some examples:

#### Creating and Using a String Hash Table

```c
char *strings[] = {"Mr Smith", "Mr Jones", "Ms Leonard", "Ms James", "Mrs Peabody", "Mr Smile"};
Payload payloads[] = {1, 2, 3, 4, 5, 6};
size_t num_strings = sizeof(strings) / sizeof(strings[0]);

HashNode *string_hash = create_string_hash(strings, payloads, num_strings);

Payload result;
if (lookup_string("Mr Jones", string_hash, &result)) {
    printf("Found: %d\n", result);
} else {
    printf("Not found\n");
}
```

#### Creating and Using an Integer Hash Table

```c
int64_t integers[] = {123, 456, 789, 101112, 131415};
Payload payloads[] = {1, 2, 3, 4, 5};
size_t num_integers = sizeof(integers) / sizeof(integers[0]);

HashNode *integer_hash = create_integer_hash(integers, payloads, num_integers);

Payload result;
if (lookup_integer(456, integer_hash, &result)) {
    printf("Found: %d\n", result);
} else {
    printf("Not found\n");
}
```

#### Evaluating Hash Table Efficiency

To evaluate the efficiency of a hash table, use the `hash_table_efficiency` function:

```c
int slot_efficiency;
size_t max_comparisons;
hash_table_efficiency(string_hash, &slot_efficiency, &max_comparisons);
printf("Slot efficiency: %d%%, Max comparisons: %zu\n", slot_efficiency, max_comparisons);
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Contact

Adrian Sutherland
Email: adrian@sutherlandonline.org
Date: 2025-01-24
