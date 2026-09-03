# Objectives
Practice + Learning for performance oriented programming with C and Rust.
With no professional experience in either language I just want to learn both, and also solve some performance-oriented challenge while at it.

## Rules
1. Standard libraries only. 
2. No LLM Agent written code.
2. No LLMs for debugging.
3. Using LLM chat interface for question answering is allowed, but no copy pasting of code.

## Challenge
- [[https://www.morling.dev/blog/one-billion-row-challenge/]]
- input: 1B rows of cities and temperature entries
- output: city; <min>/<mean>/<max> for each unique city as fast as possible.

## Structure
data/ contains the dataset
c/ contains C solution, gcc 1brc.c -o 1brc && ./1brc to run
rust/ contains rust solution, cargo run to run
