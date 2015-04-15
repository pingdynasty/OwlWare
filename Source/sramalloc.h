#ifndef __SRAMALLOC_H
#define __SRAMALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

void InitMem(char *ptr, int size_in_bytes);
void * myalloc(int elem_size);
void myfree(void *p);

#ifdef __cplusplus
}
#endif

#endif /* __SRAMALLOC_H */
