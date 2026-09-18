#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//-------------------- Hash Map ---------------------
// FNV-1a consts
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#define MULT_FACTOR 10 // we deal in int, and assume all numbers are 1 decimal valued.
#define TABLE_SIZE 1024*128 // rules say 10000 max unique cities
#define CAPACITY 1024*16
#define MASK (TABLE_SIZE - 1)

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

// Simple hash map with linear probing
typedef struct cityEntry
{
  char *city;
  Stats stats;
} cityEntry;

int collisionCtr = 0;

Stats *getOrCreateCity(cityEntry *citiesMap[TABLE_SIZE], char *city, size_t len, Book *citiesBook)
{
  // we need to take a look at hash_str next.
  size_t idx = hash_str(city, len) & MASK;

  // follow linked list until we hit same city (can we avoid this somehow? probing?)
  while (1)
  {
    cityEntry *e = citiesMap[idx];
    if (e==NULL){
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
      citiesMap[idx] = e;
    
      return &e->stats;
    }

    if (memcmp(e->city, city, len) == 0)
    {
      return &e->stats;
    }
    collisionCtr++;
    idx = (idx + 1) & MASK;
  }

}
