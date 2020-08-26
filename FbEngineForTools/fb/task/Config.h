#ifndef FB_TASK_CONFIG_H
#define FB_TASK_CONFIG_H

#if (FB_BUILD == FB_DEBUG)

#define FB_TASK_TASKSCHEDULER_COLLECT_DEBUG_DATA FB_TRUE

#elif (FB_BUILD == FB_RELEASE)

#define FB_TASK_TASKSCHEDULER_COLLECT_DEBUG_DATA FB_FALSE

#elif (FB_BUILD == FB_FINAL_RELEASE)

#define FB_TASK_TASKSCHEDULER_COLLECT_DEBUG_DATA FB_FALSE

#else
#error "Unknown build."
#endif

#endif
