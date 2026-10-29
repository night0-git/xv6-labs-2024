# Sysinfo checks

Run in an otherwise idle xv6 instance:

    make qemu

At the xv6 shell:

    sysinfotest
    sysinfoedge
    trace 8388608 sysinfotest

The upstream test ends with "sysinfotest: OK"; the extended test ends with "sysinfoedge: OK". A failed extended assertion prints
"sysinfoedge: FAIL: ..." and exits with status 1.

Base: origin/syscall (393d47b); conf/lab.mk selects LAB=syscall.
SYS_trace=22 and SYS_sysinfo=23. The trace name table includes sysinfo.
The upstream sysinfotest.c is preserved and checks memory exhaustion,
allocation/free accounting, invalid pointers, and process counts.

Extended sysinfoedge coverage:
- Valid stack output; free memory is page-aligned; process count is in range.
- 1000 calls without changes to free memory or process count.
- 20 allocation/free cycles: exactly four pages consumed and returned.
  Page tables are warmed up before comparing data-page counts.
- Aligned, unaligned, cross-page, and end-of-mapping output buffers.
- Sentinel bytes before/after the output detect writes outside the struct.
- NULL/text (read-only), unmapped, partially unmapped, MAXVA, all-ones,
  and freed-page addresses must return -1.
- Four children kept alive with a pipe: process count rises by four.
- Concurrent calls in parent/children, exit status checks, and wait cleanup:
  process count and free memory return to the baseline.

Global totals may change if unrelated background programs run.
A failed copyout may write a prefix before returning -1; the test does not
require atomic output. Free memory and process count are collected separately,
so the test does not require a single atomic system snapshot.
These checks do not exhaust every possible interleaving, allocation failure,
or process-table exhaustion case.

Validation: compiled and passed on Ubuntu 24.04 / QEMU, xv6 with 3 CPUs.
