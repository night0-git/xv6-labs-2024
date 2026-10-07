#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

struct arguments {
  const char *path;
  int all;
  int summary;
};

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
      // Skip empty slots.
      if (de.inum == 0) {
        continue;
      }
      // Skip to avoid infinite recursion.
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
        continue;
      }

      // Construct the full path of the file/directory.
      char file_path[512];
      int path_len = strlen(path) + 1 + strlen(de.name);
      if (path_len >= sizeof(file_path)) {
        fprintf(2, "du: path too long: %s/%s\n", path, de.name);
        exit(1);
      }
      memcpy(file_path, path, strlen(path));
      file_path[strlen(path)] = '/';
      memcpy(file_path + strlen(path) + 1, de.name, strlen(de.name));
      file_path[path_len] = '\0';

      total_size += print_size(file_path, all, summary, 0);
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

void
parse_args(int argc, char *argv[], struct arguments *args)
{
  args->all = 0;
  args->summary = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-a") == 0) {
        args->all = 1;
      } else if (strcmp(argv[i], "-s") == 0) {
        args->summary = 1;
      } else {
        fprintf(2, "du: unknown option %s\n", argv[i]);
        fprintf(2, "Usage: du [PATH] [-a] [-s]\n");
        exit(1);
      }
    } else {
      if (args->path) {
        fprintf(2, "du: multiple paths specified\n");
        fprintf(2, "Usage: du [PATH] [-a] [-s]\n");
        exit(1);
      }
      args->path = argv[i];
    }
  }

  // Default to current directory.
  if (!args->path) {
    args->path = ".";
  }
}

int
main(int argc, char *argv[])
{
  struct arguments args;
  parse_args(argc, argv, &args);

  print_size(args.path, args.all, args.summary, 1);

  exit(0);
}
