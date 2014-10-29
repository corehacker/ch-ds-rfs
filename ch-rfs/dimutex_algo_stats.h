/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo_stats.h
 * \author sandeepprakash
 *
 * \date   13-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_ALGO_STATS_H__
#define __DIMUTEX_ALGO_STATS_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_ALGO_STATS_MAX_CS_ENTRIES             (50)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _DIMUTEX_ALGO_CS_ENTRY_STAT_X
{
   uint32_t ui_cs_req_time_ms;

   uint32_t ui_cs_grant_time_ms;

   uint32_t ui_cs_exec_end_time_ms;

   uint32_t ui_cs_allowed_time_ms;

   uint32_t ui_no_request_msgs_sent;

   uint32_t ui_no_reply_msgs_received;
} DIMUTEX_ALGO_CS_ENTRY_STAT_X;

typedef struct _DIMUTEX_ALGO_STATS_X
{
   uint32_t ui_unit_time_ms;

   uint32_t ui_count;

   DIMUTEX_ALGO_CS_ENTRY_STAT_X  xa_cs_entry_stat[DIMUTEX_ALGO_STATS_MAX_CS_ENTRIES];

   uint32_t ui_first_20_avg;

   uint32_t ui_last_20_avg;

   uint32_t ui_full_avg;

   uint32_t ui_min_msgs;

   uint32_t ui_max_msgs;
} DIMUTEX_ALGO_STATS_X;

/***************************** FUNCTION PROTOTYPES ****************************/

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_ALGO_STATS_H__ */
