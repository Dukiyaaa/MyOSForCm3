#ifndef __TCONFIG_H
#define __TCONFIG_H

#define TINYOS_PRIO_COUNT 32
#define TINYOS_SLICE_MAX  10
#define TINYOS_IDLETASK_STACK_SIZE 1024

#define TINYOS_TIMERTASK_STACK_SIZE 1024
#define TINYOS_TIMERTASK_PRIO       1

#define TINYOS_SYSTICK_MS           10

//裁剪部分，利用条件编译 1为开启功能
#define TINYOS_ENABLE_SEM            0
#define TINYOS_ENABLE_MUTEX          0
#define TINYOS_ENABLE_FLAGGROUP      0
#define TINYOS_ENABLE_MBOX           0
#define TINYOS_ENABLE_MEMBLOCK       0
#define TINYOS_ENABLE_TIMER          0
#define TINYOS_ENABLE_CPUUSAGE_STATE 0
#define TINYOS_ENABLE_HOOKS          0
#endif

