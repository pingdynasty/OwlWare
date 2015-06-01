#ifndef __SRAMALLOC_H
#define __SRAMALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

void sram_init(char *ptr, int size_in_bytes);
void* sram_alloc(int elem_size);
void sram_free(void *p);
int sram_used();

#ifdef __cplusplus
}
#endif

#endif /* __SRAMALLOC_H */
