// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NHASH 13 
#define HASH(dev, blockno) (((dev) ^ (blockno)) % NHASH)

struct {
  struct spinlock lock;
  int size;
  struct buf buf[NBUF];

  struct spinlock buckets_lock[NHASH]; 
  struct buf *buckets[NHASH];
} bcache;

void
binit(void)
{
  struct buf *b;

  bcache.size = 0;
  initlock(&bcache.lock, "bcache");
  for (int i = 0; i < NHASH; i++){
    initlock(&bcache.buckets_lock[i], "bcache.bucket");
  }
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *availb, *prevb;
  int bid, temp_size, i;

  bid = HASH(dev, blockno);
  acquire(&bcache.buckets_lock[bid]);

  // Is the block already cached?
  for(b = bcache.buckets[bid]->next; b; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buckets_lock[bid]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.buckets_lock[bid]);

  // Not cached.
  // First see if all available buffers are in the hashtable
  acquire(&bcache.lock);
  temp_size = bcache.size;
  if(temp_size < NBUF){
    availb = &bcache.buf[temp_size];
    availb->dev = dev;
    availb->blockno = blockno;
    availb->valid = 0;
    availb->refcnt = 1;
    bcache.size++;
    release(&bcache.lock);

    acquire(&bcache.buckets_lock[bid]); 
    for(b = bcache.buckets[bid]; b->next; b = b->next){ // stop when b is the prev buf
      b->next = availb;
    }
    release(&bcache.buckets_lock[bid]);
    acquiresleep(&availb->lock);
    return availb;
  }
  release(&bcache.lock);

  // Second search in the hash table whether there is an available buf
  acquire(&bcache.buckets_lock[bid]);
  for(b = bcache.buckets[bid]->next; b; b = b->next){
    if(b->refcnt == 0){
      availb = b;
      availb->dev = dev;
      availb->blockno = blockno;
      availb->valid = 0;
      availb->refcnt = 1;
      release(&bcache.buckets_lock[bid]);
      acquiresleep(&availb->lock);
      return availb;
    }
  }
  release(&bcache.buckets_lock[bid]);

  for(i = (bid+1)%NHASH; i != bid; i = (i+1)%NHASH){
    acquire(&bcache.buckets_lock[i]);
    for(b = bcache.buckets[i]->next, prevb=bcache.buckets[i]; b; 
        b = b->next, prevb = prevb->next){
      if(b->refcnt == 0){
        availb = b;
        availb->dev = dev;
        availb->blockno = blockno;
        availb->valid = 0;
        availb->refcnt = 1;
        prevb->next = b->next;
        release(&bcache.buckets_lock[i]);

        acquire(&bcache.buckets_lock[bid]);
        for(b = bcache.buckets[bid]; b->next; b = b->next)
          ;
        b->next = availb;
        release(&bcache.buckets_lock[bid]);

        acquiresleep(&availb->lock);
        return availb;
      }
    }
    release(&bcache.buckets_lock[i]);
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
  
  release(&bcache.lock);
}

void
bpin(struct buf *b) {
  int bid = HASH(b->dev, b->blockno);
  acquire(&bcache.buckets_lock[bid]);
  b->refcnt++;
  release(&bcache.buckets_lock[bid]);
}

void
bunpin(struct buf *b) {
  int bid = HASH(b->dev, b->blockno);
  acquire(&bcache.buckets_lock[bid]);
  b->refcnt--;
  release(&bcache.buckets_lock[bid]);
}


