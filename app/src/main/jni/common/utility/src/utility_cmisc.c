#include <utility_cmisc.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * RETURN:
 *  ERROR:  -1
 *  OK:     OUT SIZE
 **/
int32_t cmisc_split(char* str, const char *delim, char*** out) {

  int32_t size = 0;
  char lstr[strlen(str) + 1];
  strcpy(lstr, str);
  char *tokenPtr = strtok(lstr, delim);
  while(tokenPtr != NULL) {
    size++;
    tokenPtr = strtok(NULL, delim);
  }
  if (size == 0)
    return -1;

  strcpy(lstr, str);
  tokenPtr = strtok(lstr, delim);
  char** arr = (char **)malloc(size * sizeof(char**));
  int index = 0;
  while(tokenPtr != NULL) {
    arr[index] = (char*) malloc(strlen(tokenPtr) + 1);
    strcpy(arr[index], tokenPtr);
    index++;
    tokenPtr = strtok(NULL, delim);
  }
  *out = arr;

  return size;
}

void MISC_RET(cmisc_func_id id, void* hdl, int32_t size) {

  switch(id) {

    case CMISC_ID_SPLIT: {
      if (hdl && size > 0) {
        char** arr = (char **) *(char ***)hdl;
        int32_t i;
        for (i = 0; i < size; i++) {
          free(arr[i]);
        }
        free(arr);
      }
    }
    break;
  }
}

#ifdef __cplusplus
} //extern "C"
#endif
