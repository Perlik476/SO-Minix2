#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <minix/rs.h>

#include <string.h>
#include <unistd.h>

int set_bucket(int bucket_nr) {
    message m;
    memset(&m, 0, sizeof(m));
    m.m1_i1 = bucket_nr;
    return(_syscall(PM_PROC_NR, PM_SETBUCKET, &m));
}