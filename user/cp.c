#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h" // Chứa các flag O_RDONLY, O_WRONLY, O_CREATE...

int main(int argc, char *argv[]) {
    int fd_src, fd_dst, n;
    char buf[1024];

    // 1. Kiểm tra đối số
    if (argc != 3) {
        fprintf(2, "usage: cp src dst\n"); // fd 2 là stderr trong xv6
        exit(1);
    }

    // 2. Mở file nguồn (chỉ đọc)
    if ((fd_src = open(argv[1], O_RDONLY)) < 0) {
        fprintf(2, "cp: cannot open %s\n", argv[1]);
        exit(1);
    }

    // 3. Mở/Tạo file đích
    // Lưu ý: xv6 sử dụng O_CREATE (thay vì O_CREAT) và hàm open() chỉ nhận 2 tham số.
    if ((fd_dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
        fprintf(2, "cp: cannot open %s\n", argv[2]);
        close(fd_src);
        exit(1);
    }

    // 4. Vòng lặp copy dữ liệu
    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n) != n) {
            fprintf(2, "cp: write error\n");
            break; // Thoát vòng lặp để xuống đóng file và exit
        }
    }

    if (n < 0) {
        fprintf(2, "cp: read error\n");
    }

    // 5. Dọn dẹp
    close(fd_src);
    close(fd_dst);
    
    // Nếu có lỗi lúc đọc/ghi, n hoặc kết quả write sẽ làm vòng lặp dừng sớm
    if (n < 0) exit(1);
    
    exit(0);
}