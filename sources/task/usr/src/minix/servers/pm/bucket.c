#include "pm.h"
#include <sys/stat.h>
#include <minix/callnr.h>
#include <minix/endpoint.h>
#include <minix/com.h>
#include <minix/vm.h>
#include <signal.h>
#include <libexec.h>
#include <sys/ptrace.h>
#include "mproc.h"

int do_setbucket(void) {
    int bucket = m_in.m1_i1;
    return bucket;
}