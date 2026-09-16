#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "merge.h"
#include "data.h"

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

void printResults(FILE *fp, Book *citiesBook, cityEntry **citiesMap)
{
  mergeSort(citiesBook->list, citiesBook->count);

  fprintf(fp, "{");
  for (size_t i = 0; i < citiesBook->count; i++)
  {
    if (citiesBook->list[i] != NULL)
    {
      Stats *stats = getOrCreateCity(citiesMap, citiesBook->list[i], sizeof(citiesBook->list[i])-1, citiesBook); // -1 to omit null

      // go through linked list to make sure we exhaust collided entries
      fprintf(fp, "%s:%.1f/%.1f/%.1f, ",
              citiesBook->list[i],
              (float)stats->min / MULT_FACTOR,
              (float)stats->sum / stats->n / MULT_FACTOR,
              (float)stats->max / MULT_FACTOR);
    }
  }
  fprintf(fp, "}");
}