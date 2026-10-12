#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/util.h"

int
is_dir(const char *path)
{
  struct stat st;

  if (stat(path, &st) < 0) {
    return 0;
  }

  return st.type == T_DIR;
}

char *
join_path(const char *dir, const char *name)
{
  int dl = strlen(dir), nl = strlen(name);

  char *p = malloc(dl + nl + 2);
  if (p == 0)
    return 0;

  strcpy(p, dir);

  if (dl > 0 && p[dl - 1] != '/') {
    p[dl++] = '/';
    p[dl] = '\0';
  }

  strcpy(p + dl, name);

  return p;
}
