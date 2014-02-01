#include "sramalloc.h"
#include <string.h>

typedef
struct
{
  int is_available;
  int size;
} MCB, *MCB_P;


char *mem_start_p;
int max_mem;
int allocated_mem; /* this is the memory in use. */
int mcb_count;

char *heap_end;

enum {NEW_MCB=0,NO_MCB,REUSE_MCB};
enum {FREE,IN_USE};
void InitMem(char *ptr, int size_in_bytes)
{
  /* store the ptr and size_in_bytes in global variable */

  max_mem = size_in_bytes;
  mem_start_p = ptr;
  mcb_count = 0;
  allocated_mem = 0;
  heap_end = mem_start_p + size_in_bytes;
  memset(mem_start_p,0x00,max_mem);
  /* This function is complete :-) */
}

void * myalloc(int elem_size)
{
  /* check whether any chunk (allocated before) is free first */

  MCB_P p_mcb;
  int flag = NO_MCB;
  int sz;

  p_mcb = (MCB_P)mem_start_p;

  sz = sizeof(MCB);

  if( (elem_size + sz) > (max_mem - (allocated_mem + mcb_count * sz ) ) )
    {
      return NULL;
    }
  while( heap_end > ( (char *)p_mcb + elem_size + sz) )
    {

      if ( p_mcb->is_available == 0)
	{

	  if( p_mcb->size == 0)
	    {
	      flag = NEW_MCB;
	      break;
	    }
	  if( p_mcb->size >= (elem_size + sz) )
	    {
	      flag = REUSE_MCB;
	      break;
	    }
	}
      p_mcb = (MCB_P) ( (char *)p_mcb + p_mcb->size);


    }

  if( flag != NO_MCB)
    {
      p_mcb->is_available = 1;

      if( flag == NEW_MCB)
	{
	  p_mcb->size = elem_size + sizeof(MCB); 
	}
      else if( flag == REUSE_MCB)
	{
	  elem_size = p_mcb->size - sizeof(MCB);
	}
      mcb_count++;
      allocated_mem += elem_size;
      return ( (char *) p_mcb + sz);
    }

  return NULL;


  /* if size of the available chunk is equal to greater than required size, use that chunk */


}

void myfree(void *p)
{
  /* Mark in MCB that this chunk is free */
  MCB_P ptr = (MCB_P)p;
  ptr--;

  if(ptr->is_available != FREE)
    {
      mcb_count--;
      ptr->is_available = FREE;
      allocated_mem -= (ptr->size - sizeof(MCB));
    }
}
