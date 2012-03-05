#include "gc.hh"
#include "atomic.hh"
#include "crange_arch.hh"
#include "crange.hh"
#include "radix.hh"
#include "cpputil.hh"
#include "hwvm.hh"

#define VM_CRANGE 1
#define VM_RADIX  0

using std::atomic;

// A memory object (physical pages or inode).
enum vmntype { EAGER, ONDEMAND };

struct vmnode {
  const u64 npages;
  atomic<char*> page[128];
  atomic<u64> ref;
  const enum vmntype type;
  struct inode *const ip;
  const u64 offset;
  const u64 sz;

  vmnode(u64 npg, vmntype type = EAGER,
         inode *i = 0, u64 off = 0, u64 s = 0);
  ~vmnode();
  void decref();
  int allocpg();
  vmnode* copy();

  int demand_load();
  NEW_DELETE_OPS(vmnode)
};

// A mapping of a chunk of an address space to
// a specific memory object.
enum vmatype { PRIVATE, COW };

struct vma
#if VM_CRANGE
  : public range
#endif
#if VM_RADIX
  : public radix_elem
#endif
{
  const uptr vma_start;        // start of mapping
  const uptr vma_end;          // one past the last byte
  const enum vmatype va_type;
  struct vmnode * const n;

  vma(vmap *vmap, uptr start, uptr end, enum vmatype vtype, vmnode *vmn);
  ~vma();

  virtual void do_gc() { delete this; }
  NEW_DELETE_OPS(vma)
};

// An address space: a set of vmas plus h/w page table.
// The elements of e[] are not ordered by address.
struct vmap {
#if VM_CRANGE
  struct crange cr;
#endif

#if VM_RADIX
  struct radix rx;
#endif

  atomic<u64> ref;
  pgmap *const pml4;           // Page table
  char *const kshared;

  vmap();
  ~vmap();

  bool replace_vma(vma *a, vma *b);

  void decref();
  vmap* copy(int share);
  vma* lookup(uptr start, uptr len);
  int insert(vmnode *n, uptr va_start, int dotlb);
  int remove(uptr start, uptr len);

  int pagefault(uptr va, u32 err);
  int copyout(uptr va, void *p, u64 len);
  NEW_DELETE_OPS(vmap)

 private:
  int pagefault_wcow(vma *m);
};