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

