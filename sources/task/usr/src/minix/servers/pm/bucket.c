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
    message m;

    if (mp->mp_scheduler == NONE || mp->mp_scheduler == KERNEL) {
        return EPERM;
    }

    int bucket_nr = m_in.m1_i1;

    if (bucket_nr < 0 || bucket_nr >= NR_BUCKETS) {
        return EINVAL;
    }

    m.m_pm_sched_scheduling_set_bucket.bucket_nr = bucket_nr;
    m.m_pm_sched_scheduling_set_bucket.endpoint = mp->mp_endpoint;

    return _taskcall(mp->mp_scheduler, SCHEDULING_SET_BUCKET, &m);
}