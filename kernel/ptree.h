#ifndef _PTREE_H_H
#define _PTREE_H_H

#include "types.h"

struct ptreeinfo {
  int pid;
  int ppid;
  int state;
  uint64 memsize;
  char name[16];
};

#endif