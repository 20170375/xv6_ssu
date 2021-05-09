#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"
#include "stdbool.h"

struct {
	struct spinlock lock;
	struct slab slab[NSLAB];
} stable;

bool get_bit(char *b, int i){
	int index = (i / 8);
	int offset = (i % 8);
	b += index;
	return (*b & (1 << offset)) != 0;
}

void set_bit(char *b, int i){
	int index = (i / 8);
	int offset = (i % 8);
	b += (index * 8);
	*b = *b | (1 << offset);
}

void clear_bit(char *b, int i){
	int index = (i / 8);
	int offset = (i % 8);
	b += (index * 8);
	*b = *b & ~(1 << offset);
}

void slabinit(){
	initlock(&stable.lock, "stable");
	acquire(&stable.lock);

	for(int i=0; i<NSLAB; i++){
		stable.slab[i].size = 8;
		for(int j=0; j<i; j++)
			stable.slab[i].size *= 2;
		
		stable.slab[i].num_pages = 0;
		stable.slab[i].num_objects_per_page = 4096 / stable.slab[i].size;
		stable.slab[i].num_free_objects = stable.slab[i].num_objects_per_page;
		stable.slab[i].num_used_objects = 0;
		stable.slab[i].bitmap = kalloc();
		memset(stable.slab[i].bitmap, 0, 4096);
		stable.slab[i].page[stable.slab[i].num_pages++] = kalloc();
	}
	release(&stable.lock);
}

char *kmalloc(int size){
	if(size<0 && size>2048)
		return 0;
	
	char *addr = 0;
	struct slab *s = 0;

	acquire(&stable.lock);

	// find fit object
	for(s = stable.slab; s < &stable.slab[NSLAB]; s++){
		if(size <= s->size)
			break;
	}
		
	// new page needed
	if(s->num_free_objects == 0){
		// maximum pages limit
		if(s->num_pages >= MAX_PAGES_PER_SLAB){
			release(&stable.lock);
			return 0;
		}

		s->page[s->num_pages] = kalloc();

		// kalloc fail
		if(s->page[s->num_pages] == 0){
			release(&stable.lock);
			return 0;
		}
		
		s->num_pages++;
		s->num_free_objects += s->num_objects_per_page;
	}
		
	// alloc object
	int len = s->num_pages * s->num_objects_per_page;
	for(int i=0; i<len; i++){
		if(!get_bit(s->bitmap, i)){
			int page_index = i / s->num_objects_per_page;
			int page_offset = i % s->num_objects_per_page;

			addr = s->page[page_index] + (page_offset * s->size);
			set_bit(s->bitmap, i);
			s->num_free_objects--;
			s->num_used_objects++;

			break;
		}
	}
	
	release(&stable.lock);
	return addr;
}

void kmfree(char *addr, int size){
	struct slab *s;

	acquire(&stable.lock);
	
	for(s = stable.slab; s < &stable.slab[NSLAB]; s++)
		if(size <= s->size)
			break;

	int len = s->num_pages * s->num_objects_per_page;
	for(int i=0; i<len; i++){
		int page_index = i / s->num_objects_per_page;
		int page_offset = i % s->num_objects_per_page;

		if(addr == (s->page[page_index] + (page_offset * s->size))){
			// object free
			memset(addr, 1, s->size);
			s->num_free_objects++;
			s->num_used_objects--;

			// bitmap frees
			clear_bit(s->bitmap, i);

			// page free
			int nPages = s->num_pages;
			int nObjects = s->num_objects_per_page;
			if(nPages > 1){
				for(int j=0, page_index=0; j<len; j+=nObjects, page_index++){
					// check page
					bool clean = true;
					for(int k=0; k<nObjects; k++){
						if(get_bit(s->bitmap, j))
							clean = false;
					}
					
					if(clean){
						// bitmap pull
						for(int loop=0; loop<nObjects; loop++){
							for(int k=j; k<len-1; k++){
								if(get_bit(s->bitmap, i+1))		// if next bit is 1, set 1
									set_bit(s->bitmap, i);
								else                  			// if next bit is 0, set 0
									clear_bit(s->bitmap, i);
							}
							clear_bit(s->bitmap, len-1);
						}

						// page pull
						kfree(s->page[page_index]);
						for(int k=j; k<(nPages - 1); k++)
							s->page[k] = s->page[k+1];
						s->num_pages--;
						s->num_free_objects -= nObjects;

						break;
					}
				}
				
				release(&stable.lock);
				return;
			}
		}
	}
	
	release(&stable.lock);
	return;
}

void slabdump(){
	cprintf("__slabdump__\n");

	struct slab *s;

	cprintf("size\tnum_pages\tused_objects\tfree_objects\n");

	for(s = stable.slab; s < &stable.slab[NSLAB]; s++){
		cprintf("%d\t%d\t\t%d\t\t%d\n", 
			s->size, s->num_pages, s->num_used_objects, s->num_free_objects);
	}
}
