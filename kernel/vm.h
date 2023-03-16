struct vma{
  struct file *f;
  uint64 length;
  uint64 base;
  int pid;
  int prot;
  int flags;
};