#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "helpers.h"

#define CHUNK 1024*1024
#define BUF_SIZE CHUNK+1024
#ifndef CAPACITY
  #define CAPCITY 10000
#endif

int entryCtr = 0;
int printCtr = 0;

cityEntry *citiesMap[CAPACITY];

Book citiesBook = {
  .count=0
};

//--------------------- Helpers -----------------------

int main(int argc, char *argv[])
{
  // expect the first arg to be the filepath, default ot measurements_1B
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
  
  // read_buffer
  char buf[BUF_SIZE]; // ~ 10000 lines
  size_t leftover = 0;

  while (1){
    size_t n = fread(buf+leftover, 1, CHUNK*sizeof(char), fp);
    printf("read buf %ld\n", n);
    char *bufptr = buf;
    size_t total = leftover + n;
    size_t start = 0;
    Stats *stats = malloc(sizeof(Stats));
    for (size_t i=0; i<total; i++){
      // iterate through bufs with as few string copies as possible.
      if(buf[i]==';'){
        int len = i - start;
        stats = getOrCreateCity(citiesMap, &buf[start], len, &citiesBook);
        i++; // skip ';'
        bufptr = &buf[i];
        int temp = parseTemp(bufptr); // moves bufptr up till end of temperature str.
        printf("%d,\t", temp);
        stats->min = stats->min > temp ? temp : stats->min;
        stats->n += 1;
        stats->max = stats->max < temp ? temp : stats->max;
        stats->sum += (long)temp;
        if(*bufptr=='\n'){
          printf("EOL");
          bufptr++;}
        printf("%ld, %ld\n", i, bufptr - &buf[i]);
        // i += bufptr - &buf[i];
      }
      // increment i by the shift in buf
      bufptr = buf;
      start = i + 1;
    }
    free(stats);

    leftover = total - start;
    memmove(buf, buf+start, leftover);
    if(n<CHUNK){
      if (ferror(fp)){
        perror("fread");
        break;
      }
      if (feof(fp)){
        printf("EOF!");
        if(leftover > 0){

        }
      }
      break;
    }
  }
    
  fclose(fp);

  fp = fopen("output/c_sol.txt", "w");
  if (fp == NULL)
  {
    printf("Error opening file!");
    return 1;
  }
  printResults(fp, &citiesBook, citiesMap);
  fclose(fp);
  // printf("%d %d", entryCtr, printCtr);
}
