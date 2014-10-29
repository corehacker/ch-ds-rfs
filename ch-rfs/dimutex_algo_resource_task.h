/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo_resource_task.h
 * \author sandeepprakash
 *
 * \date   06-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_ALGO_RESOURCE_TASK_H__
#define __DIMUTEX_ALGO_RESOURCE_TASK_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_SERIALIZATION_RES_ID_STR        "dimutex_server_data"

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
void *dimutex_algo_resource_task(
   void *p_thread_args);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_ALGO_RESOURCE_TASK_H__ */
