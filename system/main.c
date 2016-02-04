#include <libc.h>

#include "memory_map.h"

#include "mmio.h"
#include "uart.h"
#include "timer.h"
#include "uart.h"
#include "log.h"
#include "cpu.h"

char sys_banner[] = {"SOS system buildtime [" __TIME__ " " __DATE__ "]"};
extern void __switch_to(func_1);

extern u32 __get_sp();
extern u32 __get_cpsr();
/* task*/
#define TASK_UNUSED -1
#define TASK_RUNNING 0
#define TASK_SUSPEND 1
#define TASK_READY   2

#if 0
u32 init_task()
{
    int i;
    for(i=0;i<5;i++) {
        g_task[i].task_id = -1;
    }

    PRINT_EMG("in init_task, g_task: 0x%x; g_stack: 0x%x\n", g_task, g_stack);
}

/* 得到一个空闲的TCB, 偏移 == prio == task_id */
u32 get_free_task(u32 prio)
{
    PRINT_EMG("prio: %d\n", prio);
    if (g_task[prio].task_id == -1)
    {
        PRINT_EMG("in get_free_task: %d\n", prio);
        return prio;
    }
    return -1;
}

void dump_task(u32 tid)
{
    struct task *tsk = &g_task[tid];
    int i;
    PRINT_EMG("task_id: %d\n", tid);
    PRINT_EMG("func_0: 0x%x\n", tsk->func);
    PRINT_EMG("stack: 0x%x\n", tsk->stack);
    PRINT_EMG("stack_size: %d\n", tsk->stack_size);
    PRINT_EMG("sp: 0x%x\n", tsk->sp);
    for(i=0;i<15;i++) {
        PRINT_EMG("sp[%d]: 0x%x\n", i, ((int *)(tsk->sp))[i]);
    }
    PRINT_EMG("cpsr: 0x%x\n", ((int *)(tsk->sp))[15]);
}

s32 launch(u32 para)
{
    void (*func)(void);
    func = (void (*)(void))para;
    PRINT_EMG("launch start,  func: 0x%x \n", func);
    func();
    PRINT_EMG("launch end\n");
    return 0;
}

u32 init_context(struct task *tsk, u32 task_id, func_0 func)
{
    u32 i = 0;
    u32 *context = NULL;
    tsk->task_id = task_id;
    tsk->stack = &g_stack[task_id*1024];
    tsk->stack_size = 1024; /* 1024 * 4bytes */
    tsk->func = launch;
    context = tsk->stack + tsk->stack_size - 16;
    tsk->sp = (int)context;   /* R0-R12 SP, LR, CPSR */
    context[15] = 0x1d3; /* CPSR <ARM1176JZF-S TRM P97> */
    context[14] = (u32)launch; /* LR */
    context[13] = ((u32)tsk->stack + tsk->stack_size); /* SP */
    for(i=0;i<13;i++) { /* R0 - R12 */
        context[i] = 0;
    }

    context[0] = (int)func;

    tsk->state = TASK_READY;

    return task_id;
}



u32 task_create(u32 task_prio, func_1 func)
{
    u32 tid = get_free_task(task_prio);
    if (tid == -1) {
        return -1;
    }
    PRINT_EMG("task_create 0\n");
    tid = init_context(&g_task[tid], tid, func);
    dump_task(tid);
    PRINT_EMG("task_create 1\n");
    return tid;
}

void start_first_task(u32 tid)
{
    struct task *tsk = &g_task[tid];
    PRINT_EMG("in start_first_task, tsk: 0x%x\n", tsk);
    g_running_task = tsk;
    __start_first_task(tsk);
}


/* switch to new task */
void task_switch(u32 tid)
{
    struct task *new = &g_task[tid];
    struct task *tmp = g_running_task;
    g_running_task->state = TASK_READY;
    new->state = TASK_RUNNING;
/*    PRINT_EMG("task_switch 0\n");*/
    g_running_task = new;
    __switch_context(tmp, new);
/*    PRINT_EMG("task_switch 1\n");*/
}

/* tid 1 */
void testA();
void testB()
{
    while(1) {
        uart_puts("in testB\n");           
        delay_gpio();
        task_switch(0); /* switch to testA */
        /*__switch_context(g_running_task, &g_task[0]);*/
    }
}

/* tid 0 */
void testA()
{
    while(1) {
        uart_puts("in testA\n");           
        delay_gpio();
        task_switch(1); /* switch to testA */
    }
}


void enable_int()
{
    __asm volatile (
            "cpsie i"
            );
}

void disable_int()
{
    __asm volatile (
            "cpsid i"
            );
}


void panic(const char *str)
{
    PRINT_EMG("%s\n", str);
    while(1);
}

void undefined_execption()
{
    panic("not inplemented!");
}

typedef (*exception_entry)(void);
/* 16 (internal) + 32 (external) */
exception_entry ee[48] = {
    [0 ... 47] = (exception_entry)undefined_execption,
};
const u32 vector_table_base = 0x0;
void init_vector_table()
{
    int i;
    u32 *base = (u32 *)vector_table_base;
    for(i=0;i<48;i++) {
        base[i] = (u32)ee[i];
        PRINT_EMG("[%d]: 0x%x\n", i, base[i]);
    }
    return;
}

u8 uart_recv1() {
    /* wait for UART to become ready to transmit*/
    while (1) {
        if (!(readl(UART0_FR) & (1 << 4))) {
            break;
        }
    }
    return readl(UART0_DR);
}
#endif

char* get_cpu_mode()
{
    u32 cpsr, mode;
    cpsr = __get_cpsr();
    mode = cpsr & 0x1f;
    switch (mode) {
        case (16):
            return "user mode";
        case (17):
            return "fiq mode";
        case (18):
            return "irq mode";
        case (19):
            return "supervisor mode";
        case (22):
            return "secmonitor mode";
        case (23):
            return "abort mode";
        case (27):
            return "undefined mode";
        case (31):
            return "system mode";
        default:
            return "unknown mode";
    }
}

u32 test_gpio = 1;

int main(u32 sp)
{
    u32 cpsr;
    u32 pc;
    u32 tid;
    u8  ch;

    int_init();
    uart_init();

    PRINT_INFO("%s\n", sys_banner);
    PRINT_INFO("cpu_mode: %s; lr: 0x%x; sp: 0x%x; cpsr: 0x%x\n", 
            get_cpu_mode(), __get_lr(), sp, __get_cpsr());
    set_gpio_function(16, 1);
    set_gpio_output(16, 0);
    unlock_irq();

    while(test_gpio) {
        set_gpio_output(16, 1);     /* led off */
        mdelay(1000);
        set_gpio_output(16, 0);     /* led on */
        mdelay(1000);
        PRINT_EMG("%d\n", __LINE__);
    }
    while(1) {
    };
#if 0
    assert(1==2);
    set_gpio_function(16, 1);
    init_task();

    //tid = task_create(0, testA);
    //PRINT_EMG("get tid %d\n", tid);
    //dump_task(0);

    //tid = task_create(1, testB);
    //PRINT_EMG("get tid %d\n", tid);
    //dump_task(1);

    //start_first_task(0);

    init_vector_table();
    enable_int();
    PRINT_EMG("call __die...");
    __die();
    PRINT_EMG("after call __die...");
    while(1) {
        set_gpio_value(16, 0);  /* LED ON */
        delay_gpio();
        set_gpio_value(16, 1);
        delay_gpio();
        uart_puts("in main\n");
        cpsr = __get_cpsr();
        sp = __get_sp();
        pc = __get_pc();
        PRINT_EMG("cpsr: 0x%x; sp: 0x%x; pc: 0x%x\n", cpsr, sp, pc);

    }
#endif
    return 0;
}
