# Correctness tools

Run from the repository root:

```sh
cc -std=c11 -O2 -Wall -Wextra -Wpedantic tools/reference.c -o /tmp/1brc-reference
/tmp/1brc-reference data/measurements_10K.txt > /tmp/1brc-expected.txt
c/bin/1brc data/measurements_10K.txt
python3 tools/check.py /tmp/1brc-expected.txt output/c_sol.txt
```

Run Rust on the **same input**, then compare its output the same way:

```sh
python3 tools/check.py /tmp/1brc-expected.txt output/rs_sol.txt
```

The C reference reads one line at a time and searches a plain array for each
city. It uses integer tenths and 64-bit sums/counts, independently of your
implementations. It is intentionally naive: start with small inputs; linear
city lookup will be slow for large datasets with many distinct names.

Averages follow this repository's README: round toward positive infinity
to one decimal place. For example, 1.21 becomes 1.3 and -1.29 becomes -1.2.
This is ceiling, rather than rounding to the nearest value. No floating-point
arithmetic is used. Negative zero compares equal to positive zero.

The checker accepts `city:min/mean/max` lines, or `{city:min/mean/max, ...}`
and `{city=min/mean/max, ...}` maps (including Rust's trailing `, `).
It compares cities and numbers independently of order, rejects duplicate
cities and malformed entries, and prints up to 20 differences (`--limit N`
changes that). Exit codes: 0 = match, 1 = differences, 2 = invalid input/error.
Use line output for arbitrary station names: unescaped map separators can
be ambiguous when names themselves contain map entry syntax.
