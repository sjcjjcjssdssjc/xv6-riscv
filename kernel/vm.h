struct vma{
  struct file *f;
  uint64 va;//min(R + PGSIZE,L + length)
  uint64 length;
  uint64 base;
  uint64 L;//does not change
  int pid;
  int prot;
  int flags;
};