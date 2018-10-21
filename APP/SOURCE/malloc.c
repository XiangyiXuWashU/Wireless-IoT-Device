#include "Includes.h"
  
//////////////////////////////////////////////////////////////////////////////////	 

//MMU
								  
//////////////////////////////////////////////////////////////////////////////////

//Memory pool (4 byte alignment)
__align(4) u8 membase[MEM_MAX_SIZE];			//SRAM memory pool
//Memory management table
u16 memmapbase[MEM_ALLOC_TABLE_SIZE];			//SRAM memory pool MAP
//Memory management parameters
const u32 memtblsize=MEM_ALLOC_TABLE_SIZE;		//Memory table size
const u32 memblksize=MEM_BLOCK_SIZE;			//Memory block size
const u32 memsize=MEM_MAX_SIZE;				//Total memory size


//MMU
struct _m_mallco_dev mallco_dev=
{
    mem_init,		//init
    mem_perused,		//useage
    membase,		//memory pool
    memmapbase,		//Memory management is not ready
};

//copy memory c
//*des:Destination address
//*src:Source address
//n:Memory length to be copied (in bytes)
void mymemcpy(void *des,void *src,u32 n)  
{  
    u8 *xdes=des;
    u8 *xsrc=src;
    while(n--)*xdes++=*xsrc++;  
}  
//Set memory
//*s:Memory first address
//c: The value to set
//Count: the size of the memory to be set (in bytes)
void mymemset(void *s,u8 c,u32 count)  
{  
    u8 *xs = s;  
    while(count--)*xs++=c;  
}	   
//Memory management initialization
void mem_init(void)  
{  
    mymemset(mallco_dev.memmap, 0,memtblsize*2);        //Memory status table data is cleared
    mymemset(mallco_dev.membase, 0,memsize);	        //Memory pool all data is cleared
    mallco_dev.memrdy=1;				//Memory management initialization OK
}
//Get memory usage
//Return value: usage rate (0~100)
u8 mem_perused(void)  
{  
    u32 used=0;
    u32 i;
    for(i=0;i<memtblsize;i++)
        {
           if(mallco_dev.memmap[i])used++;
        }
    return (used*100)/(memtblsize);
}  
//Memory allocation (internal call)
//memx:Memory block
//size:The amount of memory to allocate (bytes)
//Return value: 0XFFFFFFFF, which represents an error; other, memory offset address
u32 mem_malloc(u32 size)  
{  
    signed long offset=0;  
    u16 nmemb;	                                        //Number of memory blocks required
    u16 cmemb=0;                                        //Number of consecutive empty memory blocks
    u32 i;  
    if(!mallco_dev.memrdy)mallco_dev.init();	        //Not initialized, perform initialization first
    if(size==0)return 0XFFFFFFFF;			//No need to assign
    nmemb=size/memblksize;  				//Get the number of contiguous memory blocks that need to be allocated
    if(size%memblksize)nmemb++;  
    for(offset=memtblsize-1;offset>=0;offset--)	        //Search the entire memory control area
    {     
       if(!mallco_dev.memmap[offset])cmemb++;	        //Increased number of contiguous empty memory blocks
       else cmemb=0;				        //Continuous memory block clear
       if(cmemb==nmemb)			                //Found continuous nmemb empty memory blocks
        {
            for(i=0;i<nmemb;i++)  			//Label memory block is not empty
            {
                mallco_dev.memmap[offset+i]=nmemb;
            }
            return (offset*memblksize);		//Return offset address
        }
    }  
    return 0XFFFFFFFF;//No memory block matching the allocation condition was found
}  
//Free memory (internal call)
//offset:Memory address offset
//Return value: 0, the release is successful; 1, the release fails;
u8 mem_free(u32 offset)  
{  
    int i;  
    if(!mallco_dev.memrdy)//Not initialized, perform initialization first
	{
          mallco_dev.init();
          return 1;//Uninitialized
        }
    if(offset<memsize)//The offset is in the memory pool.
    {  
        int index=offset/memblksize;		//The memory block number where the offset is located
        int nmemb=mallco_dev.memmap[index];	//Number of memory blocks
        for(i=0;i<nmemb;i++)  			//Memory block clear
        {  
            mallco_dev.memmap[index+i]=0;  
        }
        return 0;  
    }else return 2;//Offset over zone.
}  
//Free memory (external call)
//ptr:Memory first address
void myfree(void *ptr)  
{  
	u32 offset;  
    if(ptr==NULL)return;//The address is 0.
 	offset=(u32)ptr-(u32)mallco_dev.membase;  
    mem_free(offset);	//Free memory
}  
//Allocate memory (external call)
//size:Memory size (bytes)
//Return value: The first address of the memory allocated.
void *mymalloc(u32 size)  
{  
    u32 offset;  									      
	offset=mem_malloc(size);  	   				   
    if(offset==0XFFFFFFFF)return NULL;  
    else return (void*)((u32)mallco_dev.membase+offset);  
}  
//Reassign memory (external call)
//*ptr: Old memory first address
//size: The amount of memory to allocate (bytes)
//Return value: The newly allocated memory first address.
void *myrealloc(void *ptr,u32 size)  
{  
    u32 offset;  
    offset=mem_malloc(size);  
    if(offset==0XFFFFFFFF)return NULL;     
    else  
    {  									   
        mymemcpy((void*)((u32)mallco_dev.membase+offset),ptr,size);	//Copy old memory contents to new memory
        myfree(ptr);  							//Free old memory
        return (void*)((u32)mallco_dev.membase+offset);  		//Return the new memory first address
    }  
}












