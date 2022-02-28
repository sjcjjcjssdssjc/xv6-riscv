struct vma{
  struct file *f;
  uint64 va;//min(R + PGSIZE,L + length)
  uint64 length;
  uint64 offset;//offset is always compared with L !!
  uint64 base;
  uint64 L;
  int prot;
  int flags;
  //int fd;
};