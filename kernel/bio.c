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
extern uint ticks;
struct buffer_cache{
  struct spinlock lock;
  struct buf buf[NBUF];

} bcache[BUCKETS];
//struct spinlock global_lock;
void
binit(void)
{
  struct buf *b;
  char s[15]={"bcache"};

  //initlock(&global_lock.lock, "bcache");
  for(int i = 0; i < BUCKETS; i++) {
    s[6] = i / 10;
    s[7] = i % 10;
    initlock(&bcache[i].lock, s);
    
    for(b = bcache[i].buf; b < bcache[i].buf+NBUF; b++){
      initsleeplock(&b->lock, "buffer");
      b->timestamp = 0;
      b->owner = bcache + i;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int ind = blockno % BUCKETS;
  acquire(&bcache[ind].lock);

  // Is the block already cached?
  for(b = bcache[ind].buf+NBUF-1; b >= bcache[ind].buf; b--){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[ind].lock);//calls brelase?
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache[ind].buf + (ticks % NBUF); b < bcache[ind].buf+NBUF; b++){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache[ind].lock);
      acquiresleep(&b->lock);
      return b;
    }
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

  b->refcnt--;
  releasesleep(&b->lock);

}

void
bpin(struct buf *b) {
  int ind = b->blockno % BUCKETS;
  acquire(&bcache[ind].lock);
  b->refcnt++;
  release(&bcache[ind].lock);
}

void
bunpin(struct buf *b) {
  int ind = b->blockno % BUCKETS;
  acquire(&bcache[ind].lock);
  b->refcnt--;
  release(&bcache[ind].lock);
}


