#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//-------------------- Hash Map ---------------------
#define HASH_CONST 0x9E3779B97F4A7C15ULL
#define MULT_FACTOR 10 // we deal in int, and assume all numbers are 1 decimal valued.
#define TABLE_SIZE 1024*128 // rules say 10000 max unique cities
#define CAPACITY 1024*16
#define MASK (TABLE_SIZE - 1)


static inline uint64_t rotl64(uint64_t x, unsigned n){
  return (x << n) | (x >> (64 -n));
}
// returns 0..7 if ; in bytes, else returns 8
static inline unsigned find_semicolon_u64(uint64_t x){
  // ';' = 0x3B
  uint64_t y = x ^ 0x3B3B3B3B3B3B3B3BULL; // 8 x ';'
  uint64_t zeros = (y - 0x0101010101010101ULL) & ~y & 0x8080808080808080ULL;
  if (zeros) return __builtin_ctzll(zeros) >> 3;
  return 8;
}

// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_str(char *s, size_t len)
{
  // if len > 8
  uint64_t a=0;
  uint64_t b=0;
  if(len>=8){
      memcpy(&a, s, 8);
      memcpy(&b, s+len-8, 8);
  } else {
      memcpy(&a, s, len);
      b=a;
  }
  uint64_t h = a ^ rotl64(b,21);
  h ^= (uint64_t)len * HASH_CONST;
  h ^= h >> 32;
  return h;
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
  char *line = *p;
  uint64_t a;
  memcpy(&a, line, 8);
  
  char *buf = line;

  for(;;){
    uint64_t x;
    memcpy(&x, buf, 8);
    
    unsigned pos = find_semicolon_u64(x);
    
    if (pos != 8){
      buf+= pos;
      break;
    }
    
    buf+=8;
  }
  
  size_t len = (size_t)(buf-line);
  
  uint64_t b = 0;
  if (len >= 8){
    memcpy(&b, buf - 8, 8);
  } else {
    uint64_t mask = (1ULL << (len * 8 )) - 1;
    a &= mask;
    b = a;
  }

  uint64_t h = a ^ rotl64(b,21);
  h ^= (uint64_t)len * HASH_CONST;
  h ^= h >> 32;

  *p = buf + 1; // move to right after ';'
  
  size_t idx = h & MASK;

  while (1)
  {
    if (ht[idx]==0){
      size_t cityCount = *cityCtr;
      cityEntry *e = &citiesMap[cityCount];
      e->city = malloc(len * sizeof(char) + 1);
      e->city = memcpy(e->city, line, len);
      e->city[len] = '\0'; // don't forget the null terminator!!
      e->hash=h;
      e->len=len;
      
      // dirty magic placeholders for now..
      e->stats = (Stats){0, 100 * MULT_FACTOR, -100 * MULT_FACTOR, 0};

      ht[idx] = cityCount+1;
      (*cityCtr) = cityCount+1;
      return &e->stats;
    }

    cityEntry *e = &citiesMap[ht[idx]-1];
    if (
      e->hash==h &&
      e->len==len &&
      memcmp(e->city, line, len) == 0)
    {
      return &e->stats;
    }
    // collisionCtr++;
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
    // collisionCtr++;
    idx = (idx + 1) & MASK;
  }

}
