/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo_messaging_task.h
 * \author sandeepprakash
 *
 * \date   06-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_ALGO_MESSAGING_TASK_H__
#define __DIMUTEX_ALGO_MESSAGING_TASK_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_ALGO_MESSAGING_TASK_Q_WAIT_TIMEOUT          (5000)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _DIMUTEX_ALGO_MSG_DATA_X
{
   uint32_t ui_src_node_index;

   uint32_t ui_data_len;

   uint8_t *puc_data;
} DIMUTEX_ALGO_MSG_DATA_X;

/***************************** FUNCTION PROTOTYPES ****************************/
void *dimutex_algo_messaging_task(
   void *p_thread_args);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_ALGO_MESSAGING_TASK_H__ */
