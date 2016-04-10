#include <libc.h>
#include <os.h>

#include "log.h"


#define SYSCALL_ARG_MAX (4)


s32 do_task_create(u32 *args);
s32 do_task_sleep (u32 *args);
s32 do_sem_create (u32 *args);
s32 do_sem_get    (u32 *args);
s32 do_sem_put    (u32 *args);
s32 do_sem_delete (u32 *args);

struct __syscall__ syscall_table[] = {
    {SYS_TASK_CREATE,  do_task_create},
    {SYS_TASK_SLEEP,   do_task_sleep }, /* not available now */
    {SYS_SEM_CREATE,   do_sem_create }, /* not available now */
    {SYS_SEM_GET,      do_sem_get    }, /* not available now */
    {SYS_SEM_PUT,      do_sem_put    }, /* not available now */
    {SYS_SEM_DELETE,   do_sem_delete }, /* not available now */
};

PUBLIC s32 system_call(u32 nr, u32 *args)
{
    s32 ret;
    PRINT_EMG("syscall %d \n", nr);
    PRINT_STAMP();
    ret = syscall_table[nr].handler(args);   /* syscall handler may invoke task_dispatch */
    PRINT_STAMP();
    return ret;
}

PUBLIC s32 os_task_create(func_1 entry, u32 arg, u32 prio)
{
    register int __r0 __asm("r0");
    register int __r1 __asm("r1");
    register int __r2 __asm("r2");
    register int __r3 __asm("r3");

    __r0 = (u32)entry;
    __r1 = arg;
    __r2 = prio;
    /* invoke the swi */
    asm ( 
            "swi " SYS_TASK_CREATE "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
}

PUBLIC s32 os_task_sleep(u32 ticks)
{
#if 1
    register s32 __r0 __asm("r0");
    register s32 __r1 __asm("r1");
    register s32 __r2 __asm("r2");
    register s32 __r3 __asm("r3");

    __r0 = (u32)ticks;
    /* invoke the swi */
    asm (
            "swi " SYS_TASK_CREATE "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
#endif
}

PUBLIC s32 os_semaphore_create(u32 tokens)
{
    register s32 __r0 __asm("r0");
    register s32 __r1 __asm("r1");
    register s32 __r2 __asm("r2");
    register s32 __r3 __asm("r3");

    __r0 = (u32)tokens;
    /* invoke the swi */
    asm (
            "swi " SYS_SEM_CREATE "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
}

PUBLIC s32 os_semaphore_delete(u32 sem_id)
{
    register s32 __r0 __asm("r0");
    register s32 __r1 __asm("r1");
    register s32 __r2 __asm("r2");
    register s32 __r3 __asm("r3");

    __r0 = (u32)sem_id;
    /* invoke the swi */
    asm (
            "swi " SYS_SEM_DELETE "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
}

PUBLIC s32 os_semaphore_get(u32 sem_id)
{
    register s32 __r0 __asm("r0");
    register s32 __r1 __asm("r1");
    register s32 __r2 __asm("r2");
    register s32 __r3 __asm("r3");

    __r0 = (u32)sem_id;
    /* invoke the swi */
    asm (
            "swi " SYS_SEM_GET "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
}

PUBLIC s32 os_semaphore_put(u32 sem_id)
{
    register s32 __r0 __asm("r0");
    register s32 __r1 __asm("r1");
    register s32 __r2 __asm("r2");
    register s32 __r3 __asm("r3");

    __r0 = (u32)sem_id;
    /* invoke the swi */
    asm (
            "swi " SYS_SEM_PUT "\n\t"
        :"=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)
        : "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)
        :"cc"
            );
    return __r0;
}

PUBLIC s32 do_task_create(u32 *args)
{
    func_1 entry; 
    u32 arg; 
    u32 prio;
    entry = (func_1)(args[0]);
    arg   = args[1];
    prio  = args[2];
    return task_create(entry, arg, prio);
}

PRIVATE s32 do_task_sleep(u32 *args)
{
    task_sleep(args[0]);
    return 0;
}

PRIVATE s32 do_sem_create(u32 *args)
{
    u32 tokens = args[0];
    return semaphore_create(tokens);
}

PRIVATE s32 do_sem_get(u32 *args)
{
    u32 sem_id = args[0];
    return semaphore_get(sem_id);
}

PRIVATE s32 do_sem_put(u32 *args)
{
    u32 sem_id = args[0];
    return semaphore_put(sem_id);
}

PRIVATE s32 do_sem_delete(u32 *args)
{
    u32 sem_id = args[0];
    return semaphore_delete(sem_id);
}
