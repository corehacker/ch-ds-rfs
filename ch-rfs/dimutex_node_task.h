/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_task.h
 * \author sandeepprakash
 *
 * \date   26-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_NODE_TASK_H__
#define __DIMUTEX_NODE_TASK_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_TASK_Q_WAIT_TIMEOUT          (1000)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _DIMUTEX_LISTENER_SOCK_ACT_DATA_X
{
   DIMUTEX_MSG_HDR_X x_hdr;

   PAL_SOCK_HDL hl_sock_hdl;
} DIMUTEX_NODE_SOCK_ACT_DATA_X;

typedef struct _DIMUTEX_ALGO_COMPLETE_X
{
   DIMUTEX_MSG_HDR_X x_hdr;

   DIMUTEX_ALGO_STATS_X x_stats;
} DIMUTEX_ALGO_COMPLETE_X;

/***************************** FUNCTION PROTOTYPES ****************************/
void *dimutex_node_task(
   void *p_thread_args);

SOCKMON_RET_E dimutex_node_sockmon_active_sock_cbk (
   SOCKMON_SOCK_ACTIVITY_STATUS_E e_status,
   PAL_SOCK_HDL hl_sock_hdl,
   void *p_app_data);

DIMUTEX_RET_E dimutex_post_msg_to_q (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint8_t *puc_data,
   uint32_t ui_data_len);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_NODE_TASK_H__ */
