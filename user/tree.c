#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define NOLIMIT 0x7fffffff

static int
isdir(char *path)
{
  struct stat st;

  if (stat(path, &st) < 0)
    return 0;
  return st.type == T_DIR;
}

static char *
joinpath(char *dir, char *name)
{
  int dl = strlen(dir), nl = strlen(name);
  char *p = malloc(dl + nl + 2);

  if (p == 0)
    return 0;
  memmove(p, dir, dl);
  if (dl > 0 && p[dl - 1] != '/')
    p[dl++] = '/';
  memmove(p + dl, name, nl);
  p[dl + nl] = 0;
  return p;
}

void
tree(char *path, char *prefix, int level, int maxDepth, int onlyDir)
{
  int fd, cnt = 0, i;
  struct stat st;
  struct dirent de;
  char (*names)[DIRSIZ + 1];

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    close(fd);
    return;
  }

  int max = st.size / sizeof(de);
  if (max == 0) {
    close(fd);
    return;
  }
  names = malloc(max * (DIRSIZ + 1));
  if (names == 0) {
    fprintf(2, "tree: out of memory\n");
    close(fd);
    return;
  }

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    memmove(names[cnt], de.name, DIRSIZ);
    names[cnt][DIRSIZ] = 0;
    if (onlyDir) {
      char *child = joinpath(path, names[cnt]);
      int d = child && isdir(child);
      free(child);
      if (!d)
        continue;
    }
    cnt++;
  }
  close(fd);

  for (i = 0; i < cnt; i++) {
    int last = (i == cnt - 1);
    printf("%s%s%s\n", prefix, last ? "└── " : "├── ", names[i]);

    if (level >= maxDepth)
      continue;
    char *child = joinpath(path, names[i]);
    if (child == 0)
      continue;
    if (isdir(child)) {
      char *np = malloc(strlen(prefix) + 8);
      if (np) {
        strcpy(np, prefix);
        strcpy(np + strlen(prefix), last ? "    " : "│   ");
        tree(child, np, level + 1, maxDepth, onlyDir);
        free(np);
      }
    }
    free(child);
  }
  free(names);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int maxDepth = NOLIMIT, onlyDir = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      onlyDir = 1;
    } else if (strcmp(argv[i], "-L") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] < '0' || argv[i + 1][0] > '9') {
        fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
      maxDepth = atoi(argv[++i]);
    } else {
      path = argv[i];
    }
  }

  printf("%s\n", path);
  tree(path, "", 1, maxDepth, onlyDir);
  exit(0);
}
