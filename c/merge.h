#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    for(size_t i=0; i<len;i ++){
        s[i] = tmp[i];
    }

}

// sorts in place
int mergeSort(char **s, size_t len){
    // only one alloc, too big for stack
    char **tmp = malloc(len * sizeof(char *));
    if(!tmp){
        return 1;
    }
    _mergeSort(s, tmp, len);
    free(tmp);
    return 0;
}
