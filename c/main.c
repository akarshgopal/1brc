#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "helpers.h"

#define CHUNK 1024 * 1024 * 8
#define LEFTOVER_BUF 1024
#define BUF_SIZE CHUNK + LEFTOVER_BUF
#ifndef TABLE_SIZE
#define TABLE_SIZE 10000
#endif

int entryCtr = 0;
int printCtr = 0;

static uint16_t ht[TABLE_SIZE];
static cityEntry citiesMap[CAPACITY]; // only unique cities in here so CAPACITY

Book citiesBook = {
    .count = 0};

int main(int argc, char *argv[])
{
  char *path = "data/measurements_1B.txt";
  if (argc == 2)
  {
    path = argv[1];
  }
  FILE *fp;

  fp = fopen(path, "r");
  if (fp == NULL)
  {
    printf("Error opening file!\n");
    return 1;
  }

  char *buf = malloc(BUF_SIZE * sizeof(char)); // ~ 10000 lines, so safer to heap alloc
  if (!buf)
  {
    return 1;
  }
  size_t leftover = 0;

  while (1)
  {
    size_t n = fread(buf + leftover, 1, CHUNK * sizeof(char), fp);
    size_t total = n + leftover;

    // we've reached EOF basically
    if (n < CHUNK)
    {
      if (ferror(fp))
      {
        perror("fread");
        break;
      }
      if (feof(fp))
      {
        size_t i = 0;
        char *p = buf;
        // this is guaranteed to end at EOF by the input rules
        while (i < total)
        {
          Stats *stats = getCityFromLine(&p, ht, citiesMap, &citiesBook);
          int temp = parseTemp(&p); // moves bufptr up till '\n'
          
          stats->min = stats->min > temp ? temp : stats->min;
          stats->n += 1;
          stats->max = stats->max < temp ? temp : stats->max;
          stats->sum += (long)temp;
          
          i = p - buf;
        }
        break;
      }
    }

    size_t start = 0;
    size_t i = 0;
    char *p = buf;

    // iterate through the read bytes until we're at end of the last line within LEFTOVER_BUF
    while (i < total - LEFTOVER_BUF)
    {
      // read city bytes -> fetch stats, and advance p until after ';'
      Stats *stats = getCityFromLine(&p, ht, citiesMap, &citiesBook);
      int temp = parseTemp(&p); // moves p up till after '\n'
      
      stats->min = stats->min > temp ? temp : stats->min;
      stats->n += 1;
      stats->max = stats->max < temp ? temp : stats->max;
      stats->sum += (long)temp;
      
      i = p - buf;
      start = i;
    }
    leftover = total - start;
    memmove(buf, buf + start, leftover);
  }
  fclose(fp);
  free(buf);
  printResults(ht, citiesMap, &citiesBook);
}
