#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CAPACITY 10000 // assuming 10000 max unique cities.
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
  while (*s){
    hash ^= (uint64_t)(unsigned char)*s++;
    hash *= FNV_PRIME;
  }
  return hash;
}

typedef struct Stats
{
  long sum;  
  int min;
  int max;
  int n;   // overflow? max_val is 2^31 ~2e9 ig, so should be good...
} Stats;

// Simple hash map with linked list collision resolution (not the best for our usage..., we try linear or quadratic probing next)
typedef struct cityEntry
{
  char *city;
  Stats stats;
  struct cityEntry *next;
} cityEntry;

cityEntry *citiesMap[CAPACITY];

Stats *getOrCreateCity(char *city)
{
  size_t idx = hash_str(city) % CAPACITY;
  cityEntry *e =  citiesMap[idx];

  // follow linked list until we hit same city (can we avoid this somehow? probing?)
  while (e != NULL){
    if (strcmp(e->city, city)==0)
      return &e->stats;
    e = e->next;
  }

  // new if entry doesn't exist
  e = malloc(sizeof *e);
  entryCtr++;
  e->city = strdup(city);
  // dirty magic placeholders for now..
  e->stats = (Stats){0, 1000000, -1000000, 0};
  e->next = citiesMap[idx];
  citiesMap[idx] = e;
  // printf("created entry for city %s\n", city);
  return &e->stats;
  
}
//--------------------- MergeSort ---------------------
void mergeSorted(char **l1, char **l2, int len1, int len2, char **tmp)
{
    int i=0,j=0;
    while(i<len1 && j<len2){
        if(strcmp(l1[i], l2[j])>0){
            tmp[i+j] = l2[j];
        j++;
            continue;
        }
        tmp[i+j]= l1[i];
        i++;
    }
    while(i<len1){
        tmp[i+j]= l1[i];
        i++;
    }
    while(j<len2){
        tmp[i+j] = l2[j];
        j++;
    }
}

void _mergeSort(char **s, char **tmp, size_t len){
    if(len<=1){
        return;
    }
    int mid = len / 2;
    _mergeSort(s, tmp, mid);
    _mergeSort(s+mid, tmp+mid, len-mid);
    mergeSorted(s, s+mid, mid, len-mid, tmp);
    // we copy a lot, can we skip?
    for(int i=0; i<len;i ++){
        s[i] = tmp[i];
    }

}

// sorts in place
int mergeSort(char **s, size_t len){
    // only one alloc
    char **tmp = malloc(len* sizeof(char*));
    if(!tmp){
        return 1;
    }
    _mergeSort(s, tmp, len);
    free(tmp);
    return 0;
}

//--------------------- Helpers -----------------------

// splits line by delim and mallocs 2 strings
char **parseLine(const char *line, char delimiter)
{
  char **results = malloc(2 * sizeof(char *));
  if (results == NULL)
  {
    return NULL;
  }
  int len = 0, i = 0;
  int tok_idx = 0;
  int start_idx = 0;

  while (1)
  {
    if (line[i] == delimiter || line[i] == '\0')
    {
      len = i - start_idx;

      results[tok_idx] = malloc((len + 1) * sizeof(char));
      if (results[tok_idx] == NULL)
        return NULL;

      memcpy(results[tok_idx], &line[start_idx], len);
      results[tok_idx][len] = '\0';

      tok_idx++;
      start_idx = i + 1;
    }
    if (line[i] == '\0')
      break;
    i++;
  }

  return results;
}

//Adapted atoi, we just ignore the '.', effectively 10x and converting to int.
int parseTemp(char *s)
{
  int n=0;
  int neg = *s=='-'? 1:0;
  s += neg;

  while( (*s>='0' && *s<='9') || *s=='.'){
    if(*s=='.'){s++;}
    n = n*10 - (int)(*s++ - '0');
  }
  return neg ? n : -n;
}

void printResults(cityEntry *cityEntries[], FILE* fp){
  fprintf(fp, "{");
  for(int i=0;i<CAPACITY;i++){
    if (cityEntries[i]!=NULL){
      cityEntry *currEntry = cityEntries[i];
      // go through linked list to make sure we exhaust collided entries
      while(currEntry!=NULL){
        fprintf(fp, "%s:%.1f/%.1f/%.1f, ", 
          currEntry->city, 
          (float)currEntry->stats.min/MULT_FACTOR, 
          (float)currEntry->stats.sum/currEntry->stats.n/MULT_FACTOR, 
          (float)currEntry->stats.max/MULT_FACTOR
        );
        printCtr++;
        currEntry = currEntry->next;
      }
      }
    //   // printf("%d: %s, %d, %d, %ld, %d \n", i, citiesMap[i]->city, citiesMap[i]->entry.min, citiesMap[i]->entry.max, citiesMap[i]->entry.sum, citiesMap[i]->entry.n);
  }
  fprintf(fp, "}");
}  

int main()
{
  FILE *fp;
  char line[128]; // longest name's 100 chars, temp longest: 4 chars
  fp = fopen("data/measurements_1B.txt", "r");
  
  if (fp == NULL){
    printf("Error opening file!\n");
    return 1;
  }
  
  while (fgets(line, sizeof(line), fp) != NULL)
  {
    char **results = parseLine(line, ';');

    Stats *stats = getOrCreateCity(results[0]);
    int temp = parseTemp(results[1]);
    free(results);

    stats->min = stats->min > temp ? temp : stats->min;
    stats->n += 1;
    stats->max = stats->max < temp ? temp : stats->max;
    stats->sum += (long) temp;
  }
  fclose(fp);
  
  fp = fopen("output/c_sol.txt","w");
  if (fp == NULL) {
       printf("Error opening file!");
       return 1;
   }
  printResults(citiesMap, fp);
  fclose(fp);
  printf("%d %d", entryCtr, printCtr);
  
}
    
