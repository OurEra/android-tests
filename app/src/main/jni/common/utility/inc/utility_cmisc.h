#ifndef _UTILITY_CMISC_H_
#define _UTILITY_CMISC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum cmisc_func_id {
    CMISC_ID_SPLIT = 0,
  } cmisc_func_id;

  int32_t cmisc_split(char* str, const char *delim, char*** out);
  void MISC_RET(cmisc_func_id id, void* hdl, int32_t size);


#ifdef __cplusplus
} //extern "C"
#endif

#endif //_UTILITY_CMISC_H_
