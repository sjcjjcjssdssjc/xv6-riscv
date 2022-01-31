// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[8];

void
kinit()
{
  for(int i = 0; i < 8; i++){
    initlock(&kmem[i].lock, "kmem");
  }
  //printf("ok\n");
  freerange(end, (void*)PHYSTOP);
  //printf("ok2\n");
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  //int i=0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    //printf("%d",i);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int cid = cpuid();
  pop_off();

  acquire(&kmem[cid].lock);
  r->next = kmem[cid].freelist;
  kmem[cid].freelist = r;
  release(&kmem[cid].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cid = cpuid();
  acquire(&kmem[cid].lock);
  pop_off();
  r = kmem[cid].freelist;
  
  if(r){
    push_off();
    kmem[cid].freelist = r->next;
    pop_off();
    release(&kmem[cid].lock);
  }
  else {
    release(&kmem[cid].lock);
    for(int i = 0; i < 8; i++){
      if(i != cid){
        acquire(&kmem[i].lock);
        r = kmem[i].freelist;
        if(r){
          kmem[i].freelist = r->next;
          release(&kmem[i].lock);
          break;
        }
        release(&kmem[i].lock);
      }
    }
  }
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
