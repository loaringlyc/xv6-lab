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
struct run* steal(int cpuid);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  char lockname[10];

  for(int i = 0; i < NCPU; i++){
    snprintf(lockname, sizeof(lockname), "kmem%d", i); // give name to each lock
    initlock(&kmem[i].lock, lockname);  
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  int id;
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  id = cpuid();

  acquire(&kmem[id].lock);
  r->next = kmem[id].freelist;
  kmem[id].freelist = r;
  release(&kmem[id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  int id;
  struct run *r;

  push_off(); // in case CPUID changes
  id = cpuid();

  acquire(&kmem[id].lock);
  r = kmem[id].freelist;
  if(!r){ // if no free page, try stealing first
    release(&kmem[id].lock);
    r = steal(id);
    acquire(&kmem[id].lock);
    kmem[id].freelist = r;
  }
  if(r)
    kmem[id].freelist = r->next;
  release(&kmem[id].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// steal free pages from other CPU for CPU of cpuid
struct run*
steal(int cpuid)
{
  struct run *slow, *fast, *head;

  for(int i = 0; i<NCPU; i++){
    if(i == cpuid){
      continue;
    }
    acquire(&kmem[i].lock); // 访问也需要锁，防止在访问的时候别人改了
    if(!kmem[i].freelist){
      release(&kmem[i].lock);
      continue;
    }
    slow = kmem[i].freelist;
    fast = kmem[i].freelist;
    
    // 如果只有一页/两页，两个都在表头
    // 如果有三/四页，slow在第二页，fast在第三页
    while(fast && fast->next){ 
      slow = slow->next;
      fast = fast->next->next;
    } 
    if (slow == fast) { // 链表中只有一个节点
      head = kmem[i].freelist;
      kmem[i].freelist = 0;
      release(&kmem[i].lock);
      return head;
    }
    head = slow->next;
    slow->next = 0;
    release(&kmem[i].lock);
    return head;
  }
  return 0;
}
