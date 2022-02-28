struct vma{
  struct file *f;
  uint64 va;
  uint64 length;
  uint64 offset;
  int prot;
  int flags;
  //int fd;
};