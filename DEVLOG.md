# Pass 1:
- We aim for the simplest idiomatic solution in C and Rust respectively.

# Pass 2:
- We moved away from reading single lines to reading chunks at a time.
- Optimized the parseTemp function to maximally use our challenge guarantees
- Eliminated unnecessary string copies, and variable allocation

# Pass 3:
- We improved the hash table performance by using linear probing.
  - quadratic probing actually was suboptimal, since we lose cache locality
  - linked list has no hope of cache locality and even requires additional dereferencing
  - break out some of the loops, optimize some functions of phase of program.
    - main reads -> we read and compute hash simultaneously
    - no dedicated cursor index, we simply increment the buf pointer.

  - Using an array of cityEntry turns out to be slower than an array of cityEntry*, since the table is quite sparse and the cityEntry struct is ~24 bytes, leading to no cache advantage. This points to a two-tiered struct instead of a single hash table.
  - linear probing on a uint16_t index table to a cityEntry array didn't make much of a difference.