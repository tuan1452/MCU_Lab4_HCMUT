/**
  ******************************************************************************
  * @file           : scheduler.h
  * @brief          : Header for Cooperative Scheduler (Lab 4 - HCMUT)
  ******************************************************************************
  */

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* Number of tasks scheduler can handle */
#define SCH_MAX_TASKS                   40
#define NO_TASK_ID                      0

/* Error codes */
#define ERROR_NORMAL                    0
#define ERROR_SCH_TOO_MANY_TASKS        1
#define ERROR_SCH_CANNOT_DELETE_TASK    2

#define RETURN_NORMAL                   1
#define RETURN_ERROR                    0

#define TIMER_TICK_MS                   10  /* 1 tick = 10 ms */

/* Task data structure */
typedef struct {
    void (* pTask)(void);   /* Pointer to the task function (void void) */
    uint32_t Delay;         /* Delta delay (in ticks) relative to previous task in queue */
    uint32_t Period;        /* Interval (ticks) between periodic executions */
    uint8_t RunMe;          /* Run flag set by scheduler when task is due */
    uint32_t TaskID;        /* Unique identifier for task */
} sTask;

/* Scheduler API prototypes */
void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (* pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);

/* Helper / Demonstration APIs */
uint32_t get_time(void);
void SCH_Report_Status(void);
void SCH_Go_To_Sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCHEDULER_H */
