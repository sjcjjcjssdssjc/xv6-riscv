struct vma{
  struct file *f;
  uint64 va;
  uint64 length;
  int prot;
  int flags;
  //int fd;
};