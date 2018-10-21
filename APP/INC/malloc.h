#ifndef __MALLOC_H
#define __MALLOC_H
#include "stm32f10x.h"
//////////////////////////////////////////////////////////////////////////////////	 

//MMU
								  
//////////////////////////////////////////////////////////////////////////////////
 
#ifndef NULL
#define NULL 0
#endif

//Memory parameter setting.
#define MEM_BLOCK_SIZE			32  	  		//Size of memory block
#define MEM_MAX_SIZE			40*1024  		//Max managable memory size
#define MEM_ALLOC_TABLE_SIZE	MEM_MAX_SIZE/MEM_BLOCK_SIZE 	//memory table size
		 
//Memory management controller
struct _m_mallco_dev
{
    void (*init)(void);			//init
    u8 (*perused)(void);		  	//memory usage
    u8 	*membase;			//memory pool
    u16 *memmap; 				//memory management status table
    u8  memrdy; 				//Is memory management ready
};
extern struct _m_mallco_dev mallco_dev;	//defined in mallco.c

void mymemset(void *s,u8 c,u32 count);	   //set memory
void mymemcpy(void *des,void *src,u32 n);  //copy memory
void mem_init(void);			   //memory management initialization function
u32 mem_malloc(u32 size);		   //memory allocation
u8 mem_free(u32 offset);		   //memory free
u8 mem_perused(void);			   //get memory usage
////////////////////////////////////////////////////////////////////////////////
//User calling function
void myfree(void *ptr);  		   //memory free(external call)
void *mymalloc(u32 size);		   //memory allocation(external call)
void *myrealloc(void *ptr,u32 size);	   //momory re-allocation(external call)
#endif













