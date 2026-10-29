#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

// Run from an otherwise idle xv6 shell: memory/process totals are system-wide.
static void
check(int ok, char *message)
{
  if (!ok) {
    fprintf(2, "sysinfoedge: FAIL: %s\n", message);
    exit(1);
  }
}

static struct sysinfo
snapshot(void)
{
  struct sysinfo info;
  check(sysinfo(&info) == 0, "valid output buffer rejected");
  check(info.freemem % PGSIZE == 0, "freemem is not a whole number of pages");
  check(info.nproc > 0 && info.nproc <= NPROC, "nproc out of range");
  return info;
}

static void
test_repeat(void)
{
  struct sysinfo before = snapshot();
  for (int i = 0; i < 1000; i++) {
    struct sysinfo now = snapshot();
    check(now.freemem == before.freemem, "repeated calls changed free memory");
    check(now.nproc == before.nproc, "repeated calls changed process count");
  }
  printf("repeat: OK (1000 calls)\n");
}

static void
test_memory(void)
{
  // Warm up page-table allocation before comparing exact data-page usage.
  char *base = sbrk(4 * PGSIZE);
  check(base != (char *)-1, "memory warm-up allocation failed");
  check(sbrk(-4 * PGSIZE) != (char *)-1, "memory warm-up release failed");
  struct sysinfo before = snapshot();
  for (int i = 0; i < 20; i++) {
    check(sbrk(4 * PGSIZE) == base, "heap allocation failed");
    struct sysinfo allocated = snapshot();
    check(before.freemem >= allocated.freemem &&
          before.freemem - allocated.freemem == 4 * PGSIZE,
          "allocating four pages did not consume exactly four pages");
    check(sbrk(-4 * PGSIZE) != (char *)-1, "heap release failed");
    struct sysinfo after = snapshot();
    check(after.freemem == before.freemem, "released pages were not counted");
    check(after.nproc == before.nproc, "sbrk changed process count");
  }
  printf("memory: OK (20 allocation/free cycles)\n");
}

static void
test_buffers(void)
{
  char *old = sbrk(0);
  int size = PGROUNDUP((uint64)old) - (uint64)old + 2 * PGSIZE;
  check(sbrk(size) == old, "buffer allocation failed");
  char *page = (char *)PGROUNDUP((uint64)old);
  char *limit = old + size;
  struct sysinfo expected = snapshot(), received;
  char *places[] = {
    page, page + 1, page + PGSIZE - 8, limit - sizeof(struct sysinfo)
  };
  for (int i = 0; i < 4; i++) {
    char *dst = places[i];
    if (dst > page)
      dst[-1] = 0x35;
    if (dst + sizeof(received) < limit)
      dst[sizeof(received)] = 0x56;
    check(sysinfo((struct sysinfo *)dst) == 0, "valid buffer layout rejected");
    memmove(&received, dst, sizeof(received));
    check(received.freemem == expected.freemem &&
          received.nproc == expected.nproc, "copied struct has wrong contents");
    if (dst > page)
      check(dst[-1] == 0x35, "write before output buffer");
    if (dst + sizeof(received) < limit)
      check(dst[sizeof(received)] == 0x56, "write after output buffer");
  }

  // A failed copyout may have written a prefix; atomic output is not required.
  uint64 invalid[] = {
    0, (uint64)test_buffers, (uint64)limit,
    (uint64)limit - 8, MAXVA, (uint64)-1
  };
  for (int i = 0; i < 6; i++) {
    check(sysinfo((struct sysinfo *)invalid[i]) == -1,
          "invalid/read-only/partly unmapped buffer accepted");
  }
  check(sbrk(-size) != (char *)-1, "buffer release failed");
  check(sysinfo((struct sysinfo *)page) == -1, "freed page accepted");
  snapshot();
  printf("buffers: OK (aligned, unaligned, cross-page, bounds, invalid addresses)\n");
}

static void
test_processes(void)
{
  int gate[2];
  check(pipe(gate) == 0, "pipe failed");
  struct sysinfo before = snapshot();
  int children = 4;
  for (int i = 0; i < children; i++) {
    int pid = fork();
    if (pid < 0) {
      close(gate[0]);
      close(gate[1]);
      while (wait(0) > 0) {}
      check(0, "fork failed");
    }
    if (pid == 0) {
      close(gate[1]);
      char token;
      int n = read(gate[0], &token, 1);
      close(gate[0]);
      if (n != 1)
        exit(1);
      for (int j = 0; j < 100; j++)
        snapshot();
      exit(0);
    }
  }
  // Each child remains alive until the parent sends release tokens.
  struct sysinfo during = snapshot();
  check(during.nproc == before.nproc + children, "fork count is wrong");
  check(write(gate[1], "go!!", children) == children, "release children failed");
  for (int i = 0; i < 100; i++)
    snapshot();
  for (int i = 0; i < children; i++) {
    int status = -1;
    check(wait(&status) > 0 && status == 0, "child test failed");
  }
  struct sysinfo after = snapshot();
  check(after.nproc == before.nproc, "wait did not restore process count");
  check(after.freemem == before.freemem, "fork/wait leaked memory");
  close(gate[0]);
  close(gate[1]);
  printf("processes: OK (4 children, concurrent calls, wait cleanup)\n");
}

int
main(void)
{
  printf("sysinfoedge: start\n");
  snapshot();
  test_repeat();
  test_memory();
  test_buffers();
  test_processes();
  printf("sysinfoedge: OK\n");
  exit(0);
}
