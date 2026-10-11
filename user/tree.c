#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/util.h"

#define NOLIMIT 0x7fffffff

void
tree(char *path, char *prefix, int level, int max_depth, int only_dir)
{
  int fd, count = 0, i;
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
    if (de.inum == 0) {
      continue;
    }
    if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
      continue;
    }

    memcpy(names[count], de.name, DIRSIZ);
    names[count][DIRSIZ] = 0;
    if (only_dir) {
      char *child = join_path(path, names[count]);
      int dir = child && is_dir(child);
      free(child);
      if (!dir) {
        continue;
      }
    }
    count++;
  }
  close(fd);

  for (i = 0; i < count; i++) {
    int last = (i == count - 1);
    printf("%s%s%s\n", prefix, last ? "└── " : "├── ", names[i]);

    if (level >= max_depth) {
      continue;
    }
    char *child = join_path(path, names[i]);
    if (child == 0) {
      continue;
    }
    if (is_dir(child)) {
      char *np = malloc(strlen(prefix) + 8);
      if (np) {
        strcpy(np, prefix);
        strcpy(np + strlen(prefix), last ? "    " : "│   ");
        tree(child, np, level + 1, max_depth, only_dir);
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
  char *path = 0;
  int max_depth = NOLIMIT, only_dir = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-d") == 0) {
        only_dir = 1;
      } else if (strcmp(argv[i], "-L") == 0) {
        if (i + 1 >= argc || argv[i + 1][0] < '0' || argv[i + 1][0] > '9') {
          fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
          exit(1);
        }
        max_depth = atoi(argv[++i]);
      } else {
        fprintf(2, "tree: unknown option %s\n", argv[i]);
        fprintf(2, "Usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
    } else {
      if (strcmp(argv[i], "0") == 0) {
        continue;
      }
      if (path) {
        fprintf(2, "tree: multiple paths specified\n");
        fprintf(2, "Usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
      path = argv[i];
    }
  }
  if (!path) {
    path = ".";
  }

  printf("%s\n", path);
  tree(path, "", 1, max_depth, only_dir);
  exit(0);
}
