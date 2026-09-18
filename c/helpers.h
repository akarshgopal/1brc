#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "merge.h"
#include "data.h"

// custom 10x -XX.Y -> -XXY parsing
int parseTemp(char **s)
{
  char *p = *s;
  int neg = (*p == '-');
  p += neg;

  int n = 0;
  // we're guaranteed 1 or 2 digits before '.'
  if (p[1] == '.')
  {
    // X.Y
    n = (p[0] - '0') * 10 + (p[2] - '0');
    p += 3;
  }
  else
  {
    // XX.Y
    n = (p[0] - '0') * 100 + (p[1] - '0') * 10 + (p[3] - '0');
    p += 4;
  }

  *s = p;
  return neg ? -n : n;
}

int printResults(Book *citiesBook, cityEntry *citiesMap[])
{
  FILE *fp = fopen("output/c_sol.txt", "w");
  if (fp == NULL)
  {
    printf("Error opening file!");
    return 1;
  }

  mergeSort(citiesBook->list, citiesBook->count);

  for (size_t i = 0; i < citiesBook->count; i++)
  {
    if (citiesBook->list[i] != NULL)
    {
      Stats *stats = getOrCreateCity(citiesMap, citiesBook->list[i], strlen(citiesBook->list[i]), citiesBook); // -1 to omit null

      // go through linked list to make sure we exhaust collided entries
      fprintf(fp, "%s:%.1f/%.1f/%.1f\n",
              citiesBook->list[i],
              (float)stats->min / MULT_FACTOR,
              (float)stats->sum / stats->n / MULT_FACTOR,
              (float)stats->max / MULT_FACTOR);
    }
  }

  fclose(fp);
  return 0;
}