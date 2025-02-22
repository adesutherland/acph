**Adaptive Columnar Perfect Hashing (ACPH)**

**1. Overview**

ACPH is a precomputed, byte-oriented, perfect hashing algorithm designed for efficient binary 
matching and indexing. It constructs a tree-like structure where each node represents a column 
in the input strings. The algorithm dynamically selects columns **within the context of each node** 
and generates perfect hash functions to minimize the size of the hash tables at each level, 
resulting in a compact and efficient representation.

**2. Key Features**

* **Byte-Oriented -** Operates on individual bytes (characters) within the strings.
* **Perfect Hashing -** Guarantees no collisions within each column's hash table.
* **Adaptive Column Selection -** Dynamically selects columns **from the remaining columns of the 
candidate strings within each node** to minimize the size of subsequent hash tables.
* **Tree Structure -** Organizes the hash tables in a tree-like structure for efficient traversal and 
comparison.
* **"Good Enough" Hashing -** Allows for empty slots in the hash tables but 
still tries to optimise for space efficiency.
* **Fallback Mechanism -** Uses a 1-to-1 "natural" hash (character's ASCII value) if a perfect hash with
fewer than 256 slots cannot be found.
* **Simple Hash Function -** Employs a simple XOR and multiplication hash function with prime numbers 
between 2 and 251.

**3. Applications**

ACPH can be applied to various data types, including:

* Strings
* Large integers
* Floating-point numbers
* Binary data

**4. Algorithm Steps**

* **Initialization -** Start with the entire set of values to be indexed.

* **Recursive Node Construction**
    1. **Column Analysis -** For each **remaining column in the candidate strings of the current node 
(excluding the column used by the parent node),** find the best perfect hash function (prime number
and table size) that minimizes collisions.
    2. **Column Selection -** Select the column which maximizes the number of unique hash values
    3. **Node Creation -** Create a node with the chosen hash parameters and construct its hash table.
    4. **Child Node Creation -** For each group of strings that collide in the hash table, recursively 
create a child node using the remaining columns.
    5. **Leaf Node Creation -** If only one binary remains, create a leaf node pointing to that binary.

* **BinaryValue Comparison**
    1. Traverse the tree by hashing the characters of the input binary at each level.
    2. If a leaf node is reached, compare the input binary with the stored binary.
    3. If a mismatch occurs, follow a default branch (if available) or report a non-match.

**5. Advantages**

* **Compactness -** Minimizes memory usage by optimizing hash table sizes.

* **Efficiency -** Provides fast binary matching and lookup.

* **Flexibility -** Adapts to different data types and binary lengths.

**6. Limitations**

* **Precomputation -** Requires precomputation of the tree structure, which could be 
computationally expensive for large datasets.

* **Heuristic Complexity -** The column selection heuristic might not always find the absolute 
optimal solution.

**7. Hash Effectiveness Summary**

The effectiveness of the ACPH algorithm can be evaluated based on two
key metrics: slot efficiency and search depth.

* **Slot Efficiency**: This metric measures the proportion of used slots to the
  total number of slots in the hash table. A higher slot efficiency indicates
  better utilization of the hash table, with fewer empty slots. The observed
  slot efficiency across various tests ranged from about 70% to 100%,
  demonstrating the algorithm's ability to effectively minimize empty slots.

* **Search Depth**: This metric indicates the maximum number of comparisons
  required to find a value in the hash table. Since each search operation
  involves hashing a single byte, the search depth directly correlates to
  the number of comparisons. The observed search depth was generally
  low, with most searches requiring only 1 to 3 comparisons, highlighting
  the algorithm's efficiency in quickly locating values.

For example, with 1,000 randomly generated double precision floats
the slot efficiency was 82% and the max search depth was 2.

Overall, the ACPH algorithm exhibits high slot efficiency and low search
depth, making it a compact and efficient solution for binary matching and indexing.

**8. Previous Work**

While this algorithm presents a novel approach to binary matching and indexing, a comprehensive 
review of existing literature has not been conducted. Therefore, I acknowledge the possibility of 
unintentional overlap with prior work and offer my apologies for any omissions in referencing 
or attribution.

Adrian Sutherland, adrian@sutherlandonline.org, 2025-01-24
