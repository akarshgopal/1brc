#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merge.h"

#define CAPACITY 10000 // rules say 10000 max unique cities
#define MULT_FACTOR 10 // we deal in int, and assume all numbers are 1 decimal valued.

//-------------------- Hash Map ---------------------
// FNV-1a consts
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

int entryCtr = 0;
int printCtr = 0;

// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_str(char *s)
{
  unsigned long hash = FNV_OFFSET;
  while (*s)
  {
    hash ^= (uint64_t)(unsigned char)*s++;
    hash *= FNV_PRIME;
  }
  return hash;
}

typedef struct Entry
{
  char *city;
  int temperature;
} Entry;

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

cityEntry *citiesMap[CAPACITY];

// bookkeeping for sorting etc.
char *citiesList[CAPACITY];
size_t citiesCount = 0;

Stats *getOrCreateCity(char *city)
{
  size_t idx = hash_str(city) % CAPACITY;
  cityEntry *e = citiesMap[idx];

  // follow linked list until we hit same city (can we avoid this somehow? probing?)
  while (e != NULL)
  {
    if (strcmp(e->city, city) == 0)
      return &e->stats;
    e = e->next;
  }

  // new if entry doesn't exist
  e = malloc(sizeof *e);
  entryCtr++;
  e->city = strdup(city);

  // we point to the keys directly instead of another copy.
  citiesList[citiesCount++] = e->city;

  // dirty magic placeholders for now..
  e->stats = (Stats){0, 100 * MULT_FACTOR, -100 * MULT_FACTOR, 0};
  e->next = citiesMap[idx];
  citiesMap[idx] = e;
  // printf("created entry for city %s\n", city);
  return &e->stats;
}

//--------------------- Helpers -----------------------

// adapted atoi, we just ignore the '.', effectively 10x and converting to int.
int parseTemp(const char *s)
{
  int n = 0;
  int neg = *s == '-' ? 1 : 0;
  s += neg;

  while ((*s >= '0' && *s <= '9') || *s == '.')
  {
    if (*s == '.')
    {
      s++;
    }
    n = n * 10 - (int)(*s++ - '0');
  }
  return neg ? n : -n;
}

// parses line into Entry struct
void parseLine(const char *line, Entry *entry)
{
  int len = 0, i = 0;
  int start_idx = 0;

  // first parse city
  while (1)
  {
    if (line[i] == ';')
    {
      len = i - start_idx;

      entry->city = malloc((len + 1) * sizeof(char));
      if (entry->city == NULL)
        return;

      memcpy(entry->city, &line[start_idx], len);
      entry->city[len] = '\0';
      start_idx = i + 1;
      break;
    }
    i++;
  }

  // parse temperature
  // we can boldy assume rest of the str is temp, parser stops at \0
  entry->temperature = parseTemp(line + start_idx);
  return;
}

void printResults(FILE *fp)
{
  mergeSort(citiesList, citiesCount);

  fprintf(fp, "{");
  for (size_t i = 0; i < citiesCount; i++)
  {
    if (citiesList[i] != NULL)
    {
      Stats *stats = getOrCreateCity(citiesList[i]);

      // go through linked list to make sure we exhaust collided entries
      fprintf(fp, "%s:%.1f/%.1f/%.1f, ",
              citiesList[i],
              (float)stats->min / MULT_FACTOR,
              (float)stats->sum / stats->n / MULT_FACTOR,
              (float)stats->max / MULT_FACTOR);
      printCtr++;
    }
  }
  fprintf(fp, "}");
}

int main()
{
  FILE *fp;
  char line[128]; // longest name's 100 chars, temp longest: 4 chars
  fp = fopen("data/measurements_1B.txt", "r");

  if (fp == NULL)
  {
    printf("Error opening file!\n");
    return 1;
  }

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    Entry entry;
    parseLine(line, &entry);

    Stats *stats = getOrCreateCity(entry.city);
    int temp = entry.temperature;

    stats->min = stats->min > temp ? temp : stats->min;
    stats->n += 1;
    stats->max = stats->max < temp ? temp : stats->max;
    stats->sum += (long)temp;
  }
  fclose(fp);

  fp = fopen("output/c_sol.txt", "w");
  if (fp == NULL)
  {
    printf("Error opening file!");
    return 1;
  }
  printResults(fp);
  fclose(fp);
  printf("%d %d", entryCtr, printCtr);
}
