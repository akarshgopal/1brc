#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <bits/mman-linux.h>

#include "helpers.h"
#define CHUNK 1024 * 1024 * 64
#define LEFTOVER_BUF 1024
#define BUF_SIZE CHUNK + LEFTOVER_BUF
#ifndef TABLE_SIZE
  #define TABLE_SIZE 10000
#endif
#define N_THREADS 16

int entryCtr = 0;
int printCtr = 0;

static uint16_t htGlobal[TABLE_SIZE];
static cityEntry citiesMapGlobal[CAPACITY]; // only unique cities in here so CAPACITY

static Book citiesBookGlobal = {
  .count = 0};

typedef struct WorkerArgs{
  int id;
  char* buf;
  size_t ctr;
  off_t start;
  off_t end;
  cityEntry citiesMap[CAPACITY];
  uint16_t ht[TABLE_SIZE];
}WorkerArgs;

void *readChunk(void *args){
  WorkerArgs *w = args;

  char *cursor = w->buf + w->start;
  char *end = w->buf + w->end;

  // !HOT PATH: every single possible speed-up here affects bottom line
  while (cursor < end)
  {
    Stats *stats = getCityFromLine(&cursor, w->ht, w->citiesMap, &w->ctr);
    int temp = parseTemp(&cursor); // moves bufptr up till '\n'
    
    stats->min = stats->min > temp ? temp : stats->min;
    ++(stats->n);
    stats->max = stats->max < temp ? temp : stats->max;
    stats->sum += (long)temp;
  }

  return NULL;
}

void mergeResults(uint16_t ht[], uint16_t cityCount, cityEntry citiesMap[CAPACITY], uint16_t htGlobal[], cityEntry citiesMapGlobal[CAPACITY]){
  // we need to iterate through cities -> get their stats and update the global citiesMap
  for(size_t i=0; i< cityCount; ++i){
    Stats *stats = &citiesMap[i].stats;
    Stats *globalStats = getOrCreateCity(citiesMap[i].city, htGlobal, citiesMapGlobal, &citiesBookGlobal.count);
    globalStats->min = globalStats->min > stats->min ? stats->min : globalStats->min;
    globalStats->n += stats->n;
    globalStats->max = globalStats->max < stats->max ? stats->max : globalStats->max;
    globalStats->sum += stats->sum;
  }
}

// Input expectations:
// File longer than at least 10k lines. At small lengths ~1-10 records,
// the threading model, the bounds calculation etc will break!
int main(int argc, char *argv[])
{
  int total_lines = 0;
  char *path = "data/measurements_1B.txt";
  if (argc == 2)
  {
    path = argv[1];
  }
  int fd = open(path, O_RDONLY);
  struct stat st;
  if (fstat(fd, &st)==-1){
    perror("fstat");
    return 1;
  }
  off_t file_size = st.st_size;

  off_t boundaries[N_THREADS+1];
  boundaries[0] = 0;
  boundaries[N_THREADS] = file_size;
  
  char *buf = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  
  if (buf == MAP_FAILED){
    perror("mmap");
    return 1;
  }
  if (madvise(buf, file_size, MADV_SEQUENTIAL)!=0){
    perror("madvise");
  }

  // cut into chunks and find boundaries by advancing until next \n
  for(int i=1;i<N_THREADS;++i){

    off_t pos = (file_size*i) / N_THREADS;

    while (pos<file_size && buf[pos] != '\n'){
      ++pos;
    }

    if(pos<file_size){
      ++pos;
    }

    boundaries[i] = pos;
  }
 
  //shift final end back to previous \n and run single byte parsing for last line
  off_t pos = file_size-2; // -2 to avoid any trailing \n. Hacky...
  while(pos>0 && buf[pos] != '\n'){
    --pos;
  }
  off_t last_line = pos+1;
  boundaries[N_THREADS] = last_line;
  pthread_t threads[N_THREADS];

  static WorkerArgs args[N_THREADS];

  for(int i=0; i<N_THREADS; ++i){    
    args[i].id = i;
    args[i].buf = buf;
    args[i].start = boundaries[i];
    args[i].end = boundaries[i+1];
    args[i].ctr = 0;
    int created = pthread_create(&threads[i], NULL, readChunk, &args[i]);
    if(created!=0){
      perror("Failed to create thread");
    }
  }
  
  for(int i=0; i<N_THREADS; ++i){    
    pthread_join(threads[i], NULL);
    mergeResults(args[i].ht, args[i].ctr, args[i].citiesMap, htGlobal, citiesMapGlobal);
  }
  
  // parse last line
  {
    char *cursor = buf+last_line+1;
    printf("cursor char: %c\t", *cursor);
    while(*cursor!=';'){
      cursor++;
    }
    size_t len = cursor - &buf[last_line];
    char *city = malloc(len * sizeof(char) + 1);
    city = memcpy(city, &buf[last_line], len);
    city[len] = '\0';
    printf("last line: %s\n", city);
    Stats *stats = getOrCreateCity(city, htGlobal, citiesMapGlobal, &citiesBookGlobal.count);
    cursor++;
    int temp = parseTemp(&cursor); // moves bufptr up till '\n'
    
    stats->min = stats->min > temp ? temp : stats->min;
    ++(stats->n);
    stats->max = stats->max < temp ? temp : stats->max;
    stats->sum += (long)temp;
  }

  munmap(buf, file_size);
  close(fd);
  printResults(htGlobal, citiesMapGlobal, citiesBookGlobal.count);
}
