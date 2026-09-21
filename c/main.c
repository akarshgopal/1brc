#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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
  int fd =open(path, O_RDONLY);

  if (fd == -1)
  {
    perror("Error opening file!\n");
    return 1;
  }

  struct stat st;
  if (fstat(fd, &st)== -1){
    perror("fstat");
    close(fd);
    return 1;
  }

  size_t size = (size_t)st.st_size;
  char *buf = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (buf == MAP_FAILED){
    perror("mmap");
    close(fd);
    return 1;
  }
  if (madvise(buf, size, MADV_SEQUENTIAL)!=0){
    perror("madvise");
  }

  char *p = buf;
  char *end = buf + size;

  while (p < end)
  {
    // read city bytes -> fetch stats, and advance p until after ';'
    Stats *stats = getCityFromLine(&p, ht, citiesMap, &citiesBook);
    int temp = parseTemp(&p); // moves p up till after '\n'
    
    stats->min = stats->min > temp ? temp : stats->min;
    stats->n += 1;
    stats->max = stats->max < temp ? temp : stats->max;
    stats->sum += (long)temp;
      
    }
    munmap(buf, size);
    close(fd);

    printResults(ht, citiesMap, &citiesBook);
}
