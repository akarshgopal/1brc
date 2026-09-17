#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//-------------------- Hash Map ---------------------
// FNV-1a consts
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#define MULT_FACTOR 10 // we deal in int, and assume all numbers are 1 decimal valued.
#define CAPACITY 10000 // rules say 10000 max unique cities

// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_str(char *s, size_t len)
{
  unsigned long hash = FNV_OFFSET;
  for (size_t i = 0; i < len; ++i)
  {
    hash ^= (uint64_t)(unsigned char)*s++;
    hash *= FNV_PRIME;
  }
  return hash;
}

// bookkeeping for sorting etc.
typedef struct Book
{
  char *list[CAPACITY];
  size_t count;
} Book;

typedef struct Stats
{
  long sum;
  int min;
  int max;
  int n; // overflow? max_val is 2^31 ~2e9 ig, so should be good...
} Stats;

// Simple hash map with linked list collision resolution (not the best for our usage..., we try linear or quadratic probing next)
typedef struct cityEntry
{
  char *city;
  Stats stats;
  struct cityEntry *next;
} cityEntry;

Stats *getOrCreateCity(cityEntry *citiesMap[CAPACITY], char *city, size_t len, Book *citiesBook)
{
  size_t idx = hash_str(city, len) % CAPACITY;
  cityEntry *e = citiesMap[idx];

  // follow linked list until we hit same city (can we avoid this somehow? probing?)
  while (e != NULL)
  {
    if (memcmp(e->city, city, len) == 0)
    {
      return &e->stats;
    }
    e = e->next;
  }

  // new if entry doesn't exist
  e = malloc(sizeof *e);

  // manually copy str len of memory
  e->city = malloc(len * sizeof(char) + 1);
  e->city = memcpy(e->city, city, len);
  e->city[len] = '\0'; // don't forget the null terminator!!

  // we point to the keys directly instead of another copy.
  citiesBook->list[citiesBook->count++] = e->city;

  // dirty magic placeholders for now..
  e->stats = (Stats){0, 100 * MULT_FACTOR, -100 * MULT_FACTOR, 0};
  e->next = citiesMap[idx];
  citiesMap[idx] = e;

  return &e->stats;
}
