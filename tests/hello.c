#include <lib.h>
#include <minix/rs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
//    message m;
//    endpoint_t pm_ep;
//    minix_rs_lookup("pm", &pm_ep);
//
//    errno = 0;
//    m.m1_i1 = 2;
//    int res = _syscall(pm_ep, PM_SETBUCKET, &m);
//
//    printf("do_setbucket: %d (errno=%d)\n", res, errno);

    printf("%d\n", set_bucket(-1));
    printf("%d\n", errno);
    printf("%d\n", set_bucket(NR_BUCKETS));
    printf("%d\n", errno);
    errno = 0;
    printf("%d\n", set_bucket(0));
    printf("%d\n", errno);

//    errno = 0;
//    printf("set_bucket(5): %d (errno=%d)\n", set_bucket(5), errno);
//
//    errno = 0;
//    printf("set_bucket(7): %d (errno=%d)\n", set_bucket(7), errno);
//
//    if (!fork()) {
//        errno = 0;
//        printf("child: set_bucket(10): %d (errno=%d)\n", set_bucket(3), errno);
//    }
//    else {
//        errno = 0;
//        printf("parent: set_bucket(10): %d (errno=%d)\n", set_bucket(8), errno);
//
//        errno = 0;
//        printf("parent: set_bucket(-1): %d (errno=%d)\n", set_bucket(-1), errno);
//    }

    return 0;
}
