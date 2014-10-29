/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo.c
 * \author sandeepprakash
 *
 * \date   06-Oct-2012
 *
 * \brief
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include "dimutex_env.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
DIMUTEX_RET_E dimutex_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   TASK_CREATE_PARAM_X x_task_param = {{0}};
   PAL_MUTEX_CREATE_PARAM_X x_mutex_param = {{0}};
   PAL_SEM_CREATE_PARAM_X  x_sem_param = {{0}};
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   (void) pal_memset (&(px_dimutex_ctxt->x_algo), 0x00,
      sizeof(px_dimutex_ctxt->x_algo));

   px_dimutex_ctxt->x_algo.x_algo_stats.ui_unit_time_ms =
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms;

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);

   x_sem_param.ui_initial_count = 0;
   x_sem_param.ui_max_count = 1;
   e_pal_ret = pal_sem_create(&(px_resources->hl_algo_sem_hdl),
      &x_sem_param);
   if ((ePAL_RET_SUCCESS != e_pal_ret)
      || (NULL == px_resources->hl_algo_sem_hdl))
   {
      DMUT_LOG_LOW("pal_sem_create failed: %d, %p",
         e_pal_ret, px_dimutex_ctxt, px_resources->hl_algo_sem_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   x_sem_param.ui_initial_count = 0;
   x_sem_param.ui_max_count = 1;
   e_pal_ret = pal_sem_create(&(px_resources->hl_algo_ser_hdl),
      &x_sem_param);
   if ((ePAL_RET_SUCCESS != e_pal_ret)
      || (NULL == px_resources->hl_algo_ser_hdl))
   {
      DMUT_LOG_LOW("pal_sem_create failed: %d, %p",
         e_pal_ret, px_dimutex_ctxt, px_resources->hl_algo_ser_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_mutex_create(&(px_resources->hl_algo_mutex_hdl),
      &x_mutex_param);
   if ((ePAL_RET_SUCCESS != e_pal_ret)
      || (NULL == px_resources->hl_algo_mutex_hdl))
   {
      DMUT_LOG_LOW("pal_mutex_create failed: %d, %p", e_pal_ret,
         px_dimutex_ctxt, px_resources->hl_algo_mutex_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   x_task_param.b_msgq_needed = true;
   x_task_param.ui_msgq_size = DIMUTEX_TASK_MSGQ_SIZE;
   x_task_param.fn_task_routine = dimutex_algo_messaging_task;
   x_task_param.p_app_data = px_dimutex_ctxt;
   e_task_ret = task_create(&(px_resources->hl_messaging_task_hdl),
      &x_task_param);
   if ((eTASK_RET_SUCCESS != e_task_ret)
      || (NULL == px_resources->hl_messaging_task_hdl))
   {
      DMUT_LOG_LOW("task_create failed: %d, %p", e_task_ret,
         px_resources->hl_messaging_task_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_MED("task_create success for hl_messaging_task_hdl");

   x_task_param.fn_task_routine = dimutex_algo_resource_task;
   x_task_param.p_app_data = px_dimutex_ctxt;
   e_task_ret = task_create(&(px_resources->hl_resource_task_hdl),
      &x_task_param);
   if ((eTASK_RET_SUCCESS != e_task_ret)
      || (NULL == px_resources->hl_resource_task_hdl))
   {
      DMUT_LOG_LOW("task_create failed: %d, %p", e_task_ret,
         px_resources->hl_resource_task_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_MED("task_create success for hl_resource_task_hdl");
      DMUT_LOG_MED("Dimutex Algo Setup Success");
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      if (NULL != px_dimutex_ctxt)
      {
         if (NULL != px_resources->hl_resource_task_hdl)
         {
            e_task_ret = task_delete (
               px_resources->hl_resource_task_hdl);
            if (eTASK_RET_SUCCESS != e_task_ret)
            {
               DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
            }
            px_resources->hl_resource_task_hdl = NULL;
         }
         if (NULL != px_resources->hl_messaging_task_hdl)
         {
            e_task_ret = task_delete (
               px_resources->hl_messaging_task_hdl);
            if (eTASK_RET_SUCCESS != e_task_ret)
            {
               DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
            }
            px_resources->hl_messaging_task_hdl = NULL;
         }
         if (NULL != px_resources->hl_algo_mutex_hdl)
         {
            e_pal_ret = pal_mutex_destroy (
               px_resources->hl_algo_mutex_hdl);
            if (ePAL_RET_SUCCESS != e_pal_ret)
            {
               DMUT_LOG_LOW("pal_mutex_destroy failed: %d", e_task_ret);
            }
            px_resources->hl_algo_mutex_hdl = NULL;
         }
         if (NULL != px_resources->hl_algo_sem_hdl)
         {
            e_pal_ret = pal_sem_destroy (
               px_resources->hl_algo_sem_hdl);
            if (ePAL_RET_SUCCESS != e_pal_ret)
            {
               DMUT_LOG_LOW("pal_sem_destroy failed: %d", e_task_ret);
            }
            px_resources->hl_algo_sem_hdl = NULL;
         }
      }
   }
   return e_error;
}

DIMUTEX_RET_E dimutex_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);

   if (NULL != px_resources->hl_resource_task_hdl)
   {
      e_task_ret = task_delete (px_resources->hl_resource_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
      }
      px_resources->hl_resource_task_hdl = NULL;
   }
   if (NULL != px_resources->hl_messaging_task_hdl)
   {
      e_task_ret = task_delete (px_resources->hl_messaging_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
      }
      px_resources->hl_messaging_task_hdl = NULL;
   }
   if (NULL != px_resources->hl_algo_mutex_hdl)
   {
      e_pal_ret = pal_mutex_destroy (px_resources->hl_algo_mutex_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("pal_mutex_destroy failed: %d", e_task_ret);
      }
      px_resources->hl_algo_mutex_hdl = NULL;
   }
   if (NULL != px_resources->hl_algo_ser_hdl)
   {
      e_pal_ret = pal_sem_destroy (px_resources->hl_algo_ser_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sem_destroy failed: %d", e_task_ret);
      }
      px_resources->hl_algo_ser_hdl = NULL;
   }
   if (NULL != px_resources->hl_algo_sem_hdl)
   {
      e_pal_ret = pal_sem_destroy (px_resources->hl_algo_sem_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sem_destroy failed: %d", e_task_ret);
      }
      px_resources->hl_algo_sem_hdl = NULL;
   }
   (void) pal_memset (&(px_dimutex_ctxt->x_algo), 0x00,
      sizeof(px_dimutex_ctxt->x_algo));
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);

   e_task_ret = task_start (px_resources->hl_messaging_task_hdl);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      DMUT_LOG_LOW("task_start failed: %d", e_task_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   e_task_ret = task_start (px_resources->hl_resource_task_hdl);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      DMUT_LOG_LOW("task_start failed: %d", e_task_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_algo_stop (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

CLEAN_RETURN:
   return e_error;
}
