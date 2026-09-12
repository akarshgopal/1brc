# Objectives
Practice + Learning for performance oriented programming with C and Rust.
With no professional experience in either language I just want to learn both, and also solve some performance-oriented challenge while at it.

## Challenge
- [[https://www.morling.dev/blog/one-billion-row-challenge/]]
- input: 1B rows of cities and temperature entries
- output: { city:<min>/<mean>/<max>,...} for each unique city as fast as possible.

## Rules and limits
1. No external library dependencies may be used
2. The computation must happen at application runtime, i.e. you cannot process the measurements file at build time and just bake the result into the binary
3. No LLMs for debugging.
4. Using LLM chat interface for question answering is allowed, but no copy pasting of code.
5. No LLM Agent written code.

### Input value ranges are as follows:
- Station name: non null UTF-8 string of min length 1 character and max length 100 bytes, containing neither ; nor \n characters. (i.e. this could be 100 one-byte characters, or 50 two-byte characters, etc.)
- Temperature value: non null double between -99.9 (inclusive) and 99.9 (inclusive), always with one fractional digit
- There is a maximum of 10,000 unique station names
- Line endings in the file are \n characters on all platforms
- Implementations must not rely on specifics of a given data set, e.g. any valid station name as per the constraints above and any data distribution (number of measurements per station) must be supported
- The rounding of output values must be done using the semantics of IEEE 754 rounding-direction "roundTowardPositive"


## Repo Structure
data/ contains the dataset
c/ contains C solution, gcc 1brc.c -o 1brc && ./1brc to run
rust/ contains rust solution, cargo run to run

## System
Apple M1 Pro 16 GB on arm64-apple-darwin25.6.0

## Baseline
```
$wc data/measurements_1B.txt  
42.09s user 2.95s system 96% cpu 46.732 total
```

## Profiling
- use instruments and run either c or rust binary

## Results
### Pass 1
- C:
(unsorted output) 209.83s user 14.07s system 95% cpu 3:55.18 total
- Rust: 
149.26s user 5.85s system 97% cpu 2:38.60 total
These seem pretty bad from comparing some other naive solutions on the internet. Going to look into that next pass.

### Lessons pass 1:
- first ever from scratch hashmap impl in C.
- basics of Rust (and a nice refresher to C!) some first-hand intro to ownership and borrowing. Working on C first and then rust kinda really dug this in.
- first ever string sorting on hashmap keys in C.