#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"
#include "user/util.h"

void
print_formatted(const char *path, int size)
{
  printf("%d\t%s\n", size, path);
}

int
print_size(const char *path, int all, int summary, int is_root)
{
  struct stat st;
  struct dirent de;
  int fd = open(path, O_RDONLY);

  if (fd < 0) {
    fprintf(2, "du: cannot open %s\n", path);
    exit(1);
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "du: cannot stat %s\n", path);
    exit(1);
  }

  int total_size = st.size;

  if (st.type == T_DIR) {
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) {
        continue;
      }
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
        continue;
      }

      char *file_path = join_path(path, de.name);
      if (file_path == 0) {
        fprintf(2, "du: out of memory\n");
        exit(1);
      }
      total_size += print_size(file_path, all, summary, 0);
      free(file_path);
    }

    if (is_root || !summary) {
      print_formatted(path, total_size);
    }
  } else if (is_root || all) {
    print_formatted(path, total_size);
  }

  close(fd);
  return total_size;
}

int
main(int argc, char *argv[])
{
  int all = 0, summary = 0;
  char *path = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-a") == 0) {
        all = 1;
      } else if (strcmp(argv[i], "-s") == 0) {
        summary = 1;
      } else {
        fprintf(2, "du: unknown option %s\n", argv[i]);
        fprintf(2, "Usage: du [PATH] [-a] [-s]\n");
        exit(1);
      }
    } else {
      if (path) {
        fprintf(2, "du: multiple paths specified\n");
        fprintf(2, "Usage: du [PATH] [-a] [-s]\n");
        exit(1);
      }
      path = argv[i];
    }
  }
  if (!path) {
    path = ".";
  }

  print_size(path, all, summary, 1);

  exit(0);
}
