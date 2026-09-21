## Pass 1
We need a hashmap basically. 
1. iterate line-by-line
2. store min, max, sum, n per city.
3. on end compute mean.

## Mental Dump on possible solutions.

1B row file ~10GB => we could try and mmap it?

Iteration 1:
- for each line:
  - line -> city, temp
  - get_or_create(hash(city))
  - compare min, max, compute sum
  - update city stats set(hash(city))

- print({ city:min/mean/max,})

1. we want to stop chunk processing at line boundaries.
2. we know lines are max 128 chars, and we stop at some chunk within those 128 chars.
3. we move the leftover chunk chars into the start of the buf, and read into the remainder.

say file is n lines long, each line max 128 bytes
declare buf[chunk + 128]
leftover <= 128
read into buf+leftover chunk or EOF
x*chunk + y bytes = n lines

[-] bitshifting for int parsing
mmap
[-] scan citynames once -> compute hash until we run into ';'
[-] linear / quadratic probing on hashmaps

- hash -> index
- linear probing yes, but we want to be as cache-efficient as possible when probing
=> secondary index table, which stores 0 or index+1 of city in the city arrat
there are max 10k rows, so we need to store each city contiguously

=> hash city bytes => get index
=> check index of occupancy table, 
  if zero: increment citytable index and store citytable index in occupancy table.
  if occupied and 
=>
uint16_t [0|3|0|1|2|8]
->
cityEntry [0|1|2|3|4|5]



-- How do we use parallelism?
--- we've got n threads.
--- 1 thread for I/O
         ---------- parse chunk
        |
--- read ---------- parse chunk
        |
         ---------- parse chunk 

problems? how do we separte the chunks cleanly at \n? -- read X bytes buffer, dump buffer
We can't print results until all values are read.