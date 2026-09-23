#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "helpers.h"
#define CHUNK 1024 * 1024 * 64
#define LEFTOVER_BUF 1024
#define BUF_SIZE CHUNK + LEFTOVER_BUF
#ifndef TABLE_SIZE
  #define TABLE_SIZE 10000
#endif
#define N_THREADS 10

int entryCtr = 0;
int printCtr = 0;

static uint16_t htGlobal[TABLE_SIZE];
static cityEntry citiesMapGlobal[CAPACITY]; // only unique cities in here so CAPACITY

Book citiesBook = {
  .count = 0};

typedef struct WorkerArgs{
  int id;
  int fd;
  size_t ctr;
  off_t start;
  off_t end;
  cityEntry citiesMap[CAPACITY];
}WorkerArgs;

// each thread for now just preads and counts rows.
void *readChunk(void *args){
  WorkerArgs *w = args;
  int entryCtr = 0;
  uint16_t ht[TABLE_SIZE];

  off_t offset = w->start;
  size_t leftover = 0;
  char *buf = malloc(BUF_SIZE * sizeof(char)); // ~ 10000 lines, so safer to heap alloc
  if (!buf){
    return NULL;
  }
  
  while (offset < w->end)
  {
    size_t wanted = CHUNK;
    off_t remaining = w->end - offset;
    if((off_t)wanted > remaining){
      wanted = (size_t)remaining;
    }

    size_t n = pread(w->fd, buf+leftover, wanted, offset);
    // printf("id: %d n: %ld linectr: %d \n",w->id, n, entryCtr);
    fflush(stdout);
    if (n<0){
      perror("pread");
      return NULL;
    }
    if (n==0){
      break;
    }

    size_t total = n + leftover;
    size_t i = 0;
    char *p = buf;

    // we've reached EOF basically
    if (n < CHUNK)
    {
      // this is guaranteed to end at EOF by the input rules
      while (i < total)
      {
        Stats *stats = getCityFromLine(&p, ht, w->citiesMap, &w->ctr);
        int temp = parseTemp(&p); // moves bufptr up till '\n'
        
        stats->min = stats->min > temp ? temp : stats->min;
        stats->n += 1;
        stats->max = stats->max < temp ? temp : stats->max;
        stats->sum += (long)temp;

        entryCtr++;
        
        i = p - buf;
      }
      
      break;
    }

    // iterate through the read bytes until we're at end of the last line within LEFTOVER_BUF
    while (i < total - LEFTOVER_BUF)
    {
      Stats *stats = getCityFromLine(&p, ht, w->citiesMap, &w->ctr);
      int temp = parseTemp(&p); // moves bufptr up till '\n'
      
      stats->min = stats->min > temp ? temp : stats->min;
      stats->n += 1;
      stats->max = stats->max < temp ? temp : stats->max;
      stats->sum += (long)temp;
      entryCtr++;
      
      i = p - buf;
    }
    leftover = total - i;
    memmove(buf, buf + i, leftover);
    offset += n;
  }

  free(buf);
  // if id = 0, read only until the first \n
  // else read until first eof, or go past end until first \n
  // printf("total lines in thread %d: %d\n", w->id, entryCtr);
  w->ctr = entryCtr;
  return NULL;
}

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

  // cut into chunks and find boundaries
  for(int i=1;i<N_THREADS;++i){

    off_t pos = (file_size*i) / N_THREADS;

    char c;

    while (pread(fd, &c, 1, pos)==1){
      pos++;

      if(c == '\n') break;
    }
    boundaries[i] = pos;
  }
 
  pthread_t threads[N_THREADS];
  WorkerArgs args[N_THREADS];

  for(int i=0; i<N_THREADS; ++i){    
    args[i].id = i;
    args[i].fd = fd;
    args[i].start = boundaries[i];
    args[i].end = boundaries[i+1];
    args[i].ctr = 0;
    pthread_create(&threads[i], NULL, readChunk, &args[i]);
  }
  
  for(int i=0; i<N_THREADS; ++i){    
    pthread_join(threads[i], NULL);
    total_lines += args[i].ctr;
    // printResults(ht, citiesMap, &citiesBook);
  }
  close(fd);
  printf("total collisions: %d", collisionCtr);
  printf("total lines: %d\n", total_lines);
}