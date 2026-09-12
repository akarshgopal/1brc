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
