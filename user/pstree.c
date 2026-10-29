#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/ptree.h"

#define MAX_PROCS 64

// In thông tin 1 tiến trình kèm thụt lề theo độ sâu depth (mỗi cấp 2 khoảng trắng)
static void
print_process(struct ptreeinfo *p, int depth)
{
  for (int d = 0; d < depth; d++) {
    printf("  ");
  }
  printf("%d %s state=%d mem=%ld\n", p->pid, p->name, p->state, p->memsize);
}

// Đệ quy tìm và in các con có ppid khớp với parent_pid
static void
print_tree(struct ptreeinfo *buf, int n, int parent_pid, int depth)
{
  for (int i = 0; i < n; i++) {
    if (buf[i].ppid == parent_pid && buf[i].pid != parent_pid) {
      print_process(&buf[i], depth);
      print_tree(buf, n, buf[i].pid, depth + 1);
    }
  }
}

int
main(int argc, char *argv[])
{
  struct ptreeinfo buf[MAX_PROCS];
  int n = ptree(buf, MAX_PROCS);

  if (n < 0) {
    fprintf(2, "pstree: ptree failed\n");
    exit(1);
  }

  // 1. Sắp xếp mảng theo pid tăng dần để các tiến trình cùng cấp in theo thứ tự pid
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (buf[j].pid > buf[j + 1].pid) {
        struct ptreeinfo tmp = buf[j];
        buf[j] = buf[j + 1];
        buf[j + 1] = tmp;
      }
    }
  }

  // 2. Xác định các tiến trình gốc (root process: pid == 1, ppid == 0 hoặc không có cha trong danh sách)
  for (int i = 0; i < n; i++) {
    int is_root = 0;
    if (buf[i].pid == 1 || buf[i].ppid == 0) {
      is_root = 1;
    } else {
      int parent_exists = 0;
      for (int j = 0; j < n; j++) {
        if (buf[i].ppid == buf[j].pid) {
          parent_exists = 1;
          break;
        }
      }
      if (!parent_exists) {
        is_root = 1;
      }
    }

    if (is_root) {
      print_process(&buf[i], 0);
      print_tree(buf, n, buf[i].pid, 1);
    }
  }

  exit(0);
}