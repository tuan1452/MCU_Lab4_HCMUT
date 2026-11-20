/**
  ******************************************************************************
  * @file           : scheduler.c
  * @brief          : Cooperative Scheduler Implementation (Lab 4 - HCMUT)
  *                   Features O(1) Delta-Queue in Timer ISR (SCH_Update)
  *                   with pending_ticks tracking to prevent tick loss.
  ******************************************************************************
  */

#include "scheduler.h"
#include <stdio.h>

/* Global task array and status */
static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint32_t current_task_count = 0;
static uint32_t next_task_id = 1;
static volatile uint32_t system_time_ticks = 0;
static volatile uint32_t pending_ticks = 0;

static uint8_t Error_code_G = ERROR_NORMAL;

/* Nested-safe critical section helpers */
static inline uint32_t SCH_Lock(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void SCH_Unlock(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* Apply any accumulated pending ticks to the delta queue (must be called locked) */
static void SCH_Apply_Pending_Ticks_Locked(void)
{
    uint32_t i = 0;
    while (pending_ticks > 0 && i < current_task_count) {
        if (SCH_tasks_G[i].Delay == 0) {
            SCH_tasks_G[i].RunMe = 1;
            i++;
        } else if (SCH_tasks_G[i].Delay > pending_ticks) {
            SCH_tasks_G[i].Delay -= pending_ticks;
            pending_ticks = 0;
        } else {
            pending_ticks -= SCH_tasks_G[i].Delay;
            SCH_tasks_G[i].Delay = 0;
            SCH_tasks_G[i].RunMe = 1;
            i++;
        }
    }
    if (i >= current_task_count) {
        pending_ticks = 0;
    }
}

/* Private helper for adding a task with a specific taskID (must be called locked) */
static uint32_t SCH_Add_Task_Locked(void (* pFunction)(void), uint32_t DELAY, uint32_t PERIOD, uint32_t taskID)
{
    if (pFunction == 0) {
        return NO_TASK_ID;
    }

    if (current_task_count >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return NO_TASK_ID;
    }

    uint32_t idx = 0;
    uint32_t rem_delay = DELAY;

    /* Find insertion point in the delta-delay queue */
    while (idx < current_task_count && rem_delay >= SCH_tasks_G[idx].Delay) {
        rem_delay -= SCH_tasks_G[idx].Delay;
        idx++;
    }

    /* If inserting before existing tasks, adjust the delta delay of the following task */
    if (idx < current_task_count) {
        SCH_tasks_G[idx].Delay -= rem_delay;
    }

    /* Shift subsequent tasks to make room for new task */
    for (int32_t j = (int32_t)current_task_count; j > (int32_t)idx; j--) {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1];
    }

    /* Place new task in queue */
    SCH_tasks_G[idx].pTask   = pFunction;
    SCH_tasks_G[idx].Delay   = rem_delay;
    SCH_tasks_G[idx].Period  = PERIOD;
    SCH_tasks_G[idx].RunMe   = (idx == 0 && rem_delay == 0) ? 1 : 0;
    SCH_tasks_G[idx].TaskID  = taskID;

    current_task_count++;

    return taskID;
}

/**
  * @brief  Initialize scheduler data structures.
  */
void SCH_Init(void)
{
    uint32_t primask = SCH_Lock();
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask   = 0;
        SCH_tasks_G[i].Delay   = 0;
        SCH_tasks_G[i].Period  = 0;
        SCH_tasks_G[i].RunMe   = 0;
        SCH_tasks_G[i].TaskID  = NO_TASK_ID;
    }
    current_task_count = 0;
    next_task_id = 1;
    system_time_ticks = 0;
    pending_ticks = 0;
    Error_code_G = ERROR_NORMAL;
    SCH_Unlock(primask);
}

/**
  * @brief  Update function called in TIM2 interrupt service routine (10ms tick).
  *         Runs in O(1) time - only checks the head of the sorted delta queue
  *         and counts pending_ticks if head is waiting for dispatch.
  */
void SCH_Update(void)
{
    system_time_ticks++;

    if (current_task_count > 0 && SCH_tasks_G[0].pTask != 0) {
        if (SCH_tasks_G[0].Delay > 0 && pending_ticks == 0) {
            SCH_tasks_G[0].Delay--;
            if (SCH_tasks_G[0].Delay == 0) {
                SCH_tasks_G[0].RunMe = 1;
            }
        } else {
            pending_ticks++;
        }
    }
}

/**
  * @brief  Add a task to the scheduler queue.
  * @param  pFunction: pointer to the function to execute
  * @param  DELAY: initial delay before first execution (in ticks)
  * @param  PERIOD: repetition interval (in ticks, 0 for one-shot)
  * @retval Unique taskID assigned to the task, or NO_TASK_ID if queue full.
  */
uint32_t SCH_Add_Task(void (* pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
    uint32_t primask = SCH_Lock();
    SCH_Apply_Pending_Ticks_Locked();

    uint32_t id = next_task_id++;
    if (next_task_id == 0) {
        next_task_id = 1; /* Wrap around avoiding NO_TASK_ID */
    }
    uint32_t res = SCH_Add_Task_Locked(pFunction, DELAY, PERIOD, id);
    SCH_Unlock(primask);
    return res;
}

/**
  * @brief  Delete a task from the scheduler queue by its taskID.
  * @param  taskID: unique identifier of the task
  * @retval RETURN_NORMAL (1) if success, RETURN_ERROR (0) if not found.
  */
uint8_t SCH_Delete_Task(uint32_t taskID)
{
    uint32_t primask = SCH_Lock();
    SCH_Apply_Pending_Ticks_Locked();

    if (taskID == NO_TASK_ID || current_task_count == 0) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        SCH_Unlock(primask);
        return RETURN_ERROR;
    }

    uint32_t target_idx = SCH_MAX_TASKS;
    for (uint32_t i = 0; i < current_task_count; i++) {
        if (SCH_tasks_G[i].TaskID == taskID) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        SCH_Unlock(primask);
        return RETURN_ERROR;
    }

    /* Add deleted task's remaining delay to subsequent task if present */
    if (target_idx + 1 < current_task_count) {
        SCH_tasks_G[target_idx + 1].Delay += SCH_tasks_G[target_idx].Delay;
        if (target_idx == 0 && SCH_tasks_G[1].Delay == 0) {
            SCH_tasks_G[1].RunMe = 1;
        }
    }

    /* Shift remaining tasks left */
    for (uint32_t i = target_idx; i < current_task_count - 1; i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1];
    }

    SCH_tasks_G[current_task_count - 1].pTask   = 0;
    SCH_tasks_G[current_task_count - 1].Delay   = 0;
    SCH_tasks_G[current_task_count - 1].Period  = 0;
    SCH_tasks_G[current_task_count - 1].RunMe   = 0;
    SCH_tasks_G[current_task_count - 1].TaskID  = NO_TASK_ID;

    current_task_count--;

    SCH_Unlock(primask);

    return RETURN_NORMAL;
}

/**
  * @brief  Dispatcher function running in the super loop (main while(1)).
  *         Executes due tasks, handles periodic re-queuing, and deletes one-shot tasks.
  */
void SCH_Dispatch_Tasks(void)
{
    while (1) {
        uint32_t primask = SCH_Lock();
        SCH_Apply_Pending_Ticks_Locked();

        if (current_task_count == 0 ||
            (SCH_tasks_G[0].RunMe == 0 && SCH_tasks_G[0].Delay > 0)) {
            SCH_Unlock(primask);
            break;
        }

        void (*pTask)(void) = SCH_tasks_G[0].pTask;
        uint32_t period     = SCH_tasks_G[0].Period;
        uint32_t taskID     = SCH_tasks_G[0].TaskID;

        for (uint32_t i = 0; i < current_task_count - 1; i++) {
            SCH_tasks_G[i] = SCH_tasks_G[i + 1];
        }
        SCH_tasks_G[current_task_count - 1].pTask   = 0;
        SCH_tasks_G[current_task_count - 1].Delay   = 0;
        SCH_tasks_G[current_task_count - 1].Period  = 0;
        SCH_tasks_G[current_task_count - 1].RunMe   = 0;
        SCH_tasks_G[current_task_count - 1].TaskID  = NO_TASK_ID;
        current_task_count--;

        if (period > 0 && pTask != 0) {
            SCH_Add_Task_Locked(pTask, period, period, taskID);
        }
        SCH_Unlock(primask);

        if (pTask != 0) {
            (*pTask)();
        }
    }

    SCH_Report_Status();
    SCH_Go_To_Sleep();
}

/**
  * @brief  Return the current system time in milliseconds.
  *         Accurate to the nearest 10ms tick.
  */
uint32_t get_time(void)
{
    return system_time_ticks * TIMER_TICK_MS;
}

/**
  * @brief  Report system status / errors.
  */
void SCH_Report_Status(void)
{
    if (Error_code_G != ERROR_NORMAL) {
        /* Error occurred */
        // printf("[SCH ERROR] Error code: %u\r\n", Error_code_G);
    }
}

/**
  * @brief  Enter low-power sleep mode until next interrupt (WFI).
  */
void SCH_Go_To_Sleep(void)
{
    if (pending_ticks == 0 &&
        (current_task_count == 0 ||
         (SCH_tasks_G[0].RunMe == 0 && SCH_tasks_G[0].Delay > 0))) {
        __WFI();
    }
}

/**
  * @brief  TIM2 Period Elapsed Callback (10ms tick).
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        SCH_Update();
    }
}
