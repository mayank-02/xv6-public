#include "types.h"
#include "multiboot.hh"
#include "kernel.hh"
#include "spinlock.h"
#include "kalloc.hh"
#include "cpu.hh"
#include "amd64.h"
#include "hwvm.hh"
#include "condvar.h"
#include "proc.hh"

void initpic(void);
void initioapic(void);
void inituart(void);
void initcga(void);
void initconsole(void);
void initpg(void);
void initmp(void);
void initlapic(void);
void inittls(void);
void initnmi(void);
void inittrap(void);
void initseg(void);
void initkalloc(u64 mbaddr);
void initrcu(void);
void initproc(void);
void initbio(void);
void initinode(void);
void initdisk(void);
void inituser(void);
void initsamp(void);
void initpci(void);
void initnet(void);
void initsched(void);
void initlockstat(void);
void initwq(void);
void initsperf(void);
void initidle(void);
void idleloop(void);

static volatile int bstate;

void
mpboot(void)
{
  initseg();
  inittls();
  initlapic();
  initsamp();
  initidle();
  initnmi();
  bstate = 1;
  idleloop();
}

static void
bootothers(void)
{
  extern u8 _bootother_start[];
  extern u64 _bootother_size;
  extern void (*apstart)(void);
  struct cpu *c;
  char *stack;
  u8 *code;

  // Write bootstrap code to unused memory at 0x7000.
  // The linker has placed the image of bootother.S in
  // _binary_bootother_start.
  code = (u8*) p2v(0x7000);
  memmove(code, _bootother_start, _bootother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == cpus+cpunum())  // We've started already.
      continue;

    // Tell bootother.S what stack to use and the address of apstart;
    // it expects to find these two addresses stored just before
    // its first instruction.
    stack = (char*) ksalloc(slab_stack);
    *(u32*)(code-4) = (u32)v2p(&apstart);
    *(u64*)(code-12) = (u64)stack + KSTACKSIZE;

    bstate = 0;
    lapicstartap(c->id, v2p(code));
    // Wait for cpu to finish mpmain()
    while(bstate == 0)
      ;
  }
}

void
cmain(u64 mbmagic, u64 mbaddr)
{
  extern u64 cpuhz;

  initpg();
  initseg();
  inittls();       // thread local storage
  inithz();        // CPU Hz, microdelay
  initpic();       // interrupt controller
  initioapic();
  inituart();
  initcga();
  initconsole();
  initsperf();
  inittrap();
  initmp();
  initlapic();
  initkalloc(mbaddr);
  initproc();      // process table
  initsched();     // scheduler run queues
  initidle();
  initgc();        // gc epochs and threads
  initbio();       // buffer cache
  initinode();     // inode cache
  initdisk();      // disk
  initwq();
  initsamp();
  initlockstat();
  initpci();
  initnet();

  if (VERBOSE)
    cprintf("ncpu %d %lu MHz\n", ncpu, cpuhz / 1000000);

  inituser();      // first user process
  initnmi();
  bootothers();    // start other processors
  kpml4.e[0] = 0;  // don't need 1 GB identity mapping anymore
  lcr3(rcr3());

  idleloop();

  panic("Unreachable");
}