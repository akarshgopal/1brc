#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "merge.h"
#include "data.h"

// custom 10x -XX.Y -> -XXY parsing
// moves s up till after '\n'
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

  *s = p+1; //skip the '\n'
  return neg ? -n : n;
}

// idea here was to load up the max 4 bytes of temp chars into register, reduce branching and then operate.
// only saw a decrease in performance... i guess the temp distribution can take advantage of branching afterall
int fancyParseTemp(char **s)
{
  char *p = *s;
  int neg = (*p == '-');
  p += neg;

  uint32_t x;
  memcpy(&x, p, sizeof(x)); // temp is min 3, max 5 chars. potential out-of-range access on *s here.

  uint32_t a = x & 0x0f;
  uint32_t b = (x >> 8) & 0x0f;
  uint32_t c = (x >> 16) & 0x0f;
  uint32_t d = (x >> 24) & 0x0f;

  uint32_t two = (((x >> 8)) & 0xff) != '.';

  int one_dig = a * 10 + c;
  int two_dig = a * 100 + b * 10 + d;
  int n = two ? two_dig : one_dig;

  p += 3+two+1;
  *s = p; // skip the '\n'
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
      Stats *stats = getCity(citiesMap, citiesBook->list[i], strlen(citiesBook->list[i]));

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