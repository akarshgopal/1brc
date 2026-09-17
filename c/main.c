#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "helpers.h"

#define CHUNK 1024*10
#define LEFTOVER_BUF 128
#define BUF_SIZE CHUNK+LEFTOVER_BUF
#ifndef CAPACITY
  #define CAPACITY 10000
#endif

int entryCtr = 0;
int printCtr = 0;

cityEntry *citiesMap[CAPACITY];

Book citiesBook = {
  .count=0
};

int main(int argc, char *argv[])
{
  // expect the first arg to be the filepath, default to measurements_1B
  char *path = "data/measurements_1B.txt";
  if(argc==2){
    path = argv[1];
  }
  FILE *fp;
  
  fp = fopen(path, "r");
  if (fp == NULL)
  {
    printf("Error opening file!\n");
    return 1;
  }
  
  char *buf= malloc(BUF_SIZE*sizeof(char)); // ~ 10000 lines, so safer to heap alloc
  if(!buf){
    return 1;
  }
  size_t leftover = 0;

  while (1){
    size_t n = fread(buf+leftover, 1, CHUNK*sizeof(char), fp);
    size_t total = n+leftover;
    
    // we've reached EOF basically
    if(n<CHUNK){
      if (ferror(fp)){
        perror("fread");
        break;
      }

      if (feof(fp) && leftover>0){
        size_t start = 0;
        size_t i=0;

        // this is guaranteed to end at EOF by the input rules
        while (i<leftover){
          char *bufptr = buf;
          
          if(buf[i]==';'){
            int len = i - start;
            i++;

            Stats *stats = getOrCreateCity(citiesMap, &buf[start], len, &citiesBook);

            bufptr = &buf[i];
            int temp = parseTemp(&bufptr);

            stats->min = stats->min > temp ? temp : stats->min;
            stats->n += 1;
            stats->max = stats->max < temp ? temp : stats->max;
            stats->sum += (long)temp;

            // this is basically guaranteed so could get rid of this
            if(*bufptr=='\n'){
              ++entryCtr;
              ++bufptr;
            }

            // increment i by the shift in buf
            i += bufptr - &buf[i];
            start = i;

          }
          i++;
        }
      }
      break;
    }

    size_t start = 0;
    size_t i=0;

    // iterate through the read bytes until we're at end of the last line within last 128 bytes
    while (i<total-LEFTOVER_BUF){
      char *bufptr = buf;
      // iterate through buf with as few string copies as possible.
      if(buf[i]==';'){
        int len = i - start;
        Stats *stats = getOrCreateCity(citiesMap, &buf[start], len, &citiesBook);
        i++; // skip ';'
        bufptr = &buf[i];
        int temp = parseTemp(&bufptr); // moves bufptr up till end of temperature str.

        stats->min = stats->min > temp ? temp : stats->min;
        stats->n += 1;
        stats->max = stats->max < temp ? temp : stats->max;
        stats->sum += (long)temp;

        if(*bufptr=='\n'){
          ++entryCtr;
          ++bufptr;
        }
        i += bufptr - &buf[i]; // increment i by the shift in buf
        start = i;
      }
      i++;
    }
    
    leftover = total - start;
    memmove(buf, buf+start, leftover);
    
  }
  fclose(fp);
  free(buf);

  printResults(&citiesBook, citiesMap);
}
