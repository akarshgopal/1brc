#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//-------------------- Hash Map ---------------------
// FNV-1a consts
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#define MULT_FACTOR 10 // we deal in int, and assume all numbers are 1 decimal valued.
#define TABLE_SIZE 1024*64 // rules say 10000 max unique cities
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
  uint16_t count;
  char *list[CAPACITY];
} Book;

typedef struct Stats
{
  long sum;
  int min; // -999 but 64bit int should be fast...
  int max; // 999
  int n; // overflow? max_val is 2^31 ~2e9 ig, so should be good...
} Stats;

// Simple hash map with linear probing
typedef struct cityEntry
{
  char *city;
  Stats stats;
  uint64_t hash;
  size_t len;

} cityEntry;

int collisionCtr = 0;

// optimized for the read loop where we compute hash on go.
// Returns the stats pointer, and also moves the read cursor.
Stats *getCityFromLine(char **p, uint16_t ht[TABLE_SIZE], cityEntry citiesMap[CAPACITY],  size_t *cityCtr)
{
  size_t cityCount = *cityCtr;
  char *buf = *p;
  char *line = *p;
  size_t len = 0;
  unsigned long hash = FNV_OFFSET;

  while(*buf!=';')
  {
    hash ^= (uint64_t)(unsigned char)*buf++;
    hash *= FNV_PRIME;
    len++;
  }

  *p = buf+1; // move to right after ';'
  size_t idx = hash & MASK;
  while (1)
  {
    if (ht[idx]==0){    
      cityEntry e = citiesMap[*cityCtr++];
      // manually copy str len of memory
      e.city = malloc(len * sizeof(char) + 1);
      e.city = memcpy(e.city, line, len);
      e.city[len] = '\0'; // don't forget the null terminator!!
      e.hash=hash;
      e.len=len;
      
      // dirty magic placeholders for now..
      e.stats = (Stats){0, 100 * MULT_FACTOR, -100 * MULT_FACTOR, 0};
      citiesMap[cityCount] = e;

      ht[idx] = cityCount+1;
      *cityCtr++;
      return &citiesMap[cityCount].stats;
    }

    uint16_t idx2 = ht[idx]-1;
    cityEntry e = citiesMap[idx2];
    if (
      e.hash==hash &&
      e.len==len &&
      memcmp(e.city, line, len) == 0)
    {
      return &citiesMap[idx2].stats;
    }
    collisionCtr++;
    idx = (idx + 1) & MASK;
  }

}

// optimized for only getting already populated City
Stats *getCity(char *s, uint16_t ht[TABLE_SIZE], cityEntry citiesMap[CAPACITY])
{
  size_t len = strlen(s);
  // we need to take a look at hash_str next.
  uint64_t hash = hash_str(s, len);
  size_t idx = hash & MASK;

  // follow linked list until we hit same city (can we avoid this somehow? probing?)
  while (1)
  {
    // check empty
    // WARN: returns NULL!!!
    if (ht[idx]==0){
      return NULL;
    }
    cityEntry *e = &citiesMap[ht[idx]-1];
    // check collision
    if (
      e->hash == hash &&
      e->len == len &&
      memcmp(e->city, s, len) == 0)
    {
      return &e->stats;
    }
    // linear probe
    collisionCtr++;
    idx = (idx + 1) & MASK;
  }

}
