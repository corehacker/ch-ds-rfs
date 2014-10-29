/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex.c
 * \author sandeepprakash
 *
 * \date   24-Sep-2012
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
DIMUTEX_RET_E dimutex_init (
   DIMUTEX_HDL *phl_dimutex_hdl,
   DIMUTEX_INIT_PARAMS_X *px_init_params)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   TASK_CREATE_PARAM_X x_task_param = {{0}};

   if ((NULL == phl_dimutex_hdl) || (NULL == px_init_params))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if ((NULL == px_init_params->hl_sockmon_hdl)
      || (0 == px_init_params->us_listen_port_start_ho)
      || (0 == px_init_params->ui_no_nodes)
      || (px_init_params->ui_no_nodes > DIMUTEX_MAX_NODES)
      || (px_init_params->ui_node_index > (DIMUTEX_MAX_NODES - 1)))
   {
      DMUT_LOG_LOW("Invalid Args. Sockmon Handle: %p, Listen Port: %d, "
         "No of Nodes: %d, Index: %d", px_init_params->hl_sockmon_hdl,
         px_init_params->us_listen_port_start_ho, px_init_params->ui_no_nodes,
         px_init_params->ui_node_index);
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   *phl_dimutex_hdl = NULL;

   px_dimutex_ctxt = pal_malloc(sizeof(DIMUTEX_CTXT_X), NULL);
   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("pal_malloc failed: %p", px_dimutex_ctxt);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   (void) pal_memmove(&(px_dimutex_ctxt->x_init_params), px_init_params,
      sizeof(px_dimutex_ctxt->x_init_params));

   x_task_param.b_msgq_needed = true;
   x_task_param.ui_msgq_size = DIMUTEX_TASK_MSGQ_SIZE;
   x_task_param.fn_task_routine = dimutex_node_task;
   x_task_param.p_app_data = px_dimutex_ctxt;
   e_task_ret = task_create(&(px_dimutex_ctxt->hl_listner_task_hdl),
      &x_task_param);
   if ((eTASK_RET_SUCCESS != e_task_ret)
      || (NULL == px_dimutex_ctxt->hl_listner_task_hdl))
   {
      DMUT_LOG_LOW("task_create failed: %d, %p", e_task_ret,
         px_dimutex_ctxt, px_dimutex_ctxt->hl_listner_task_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_task_ret = task_start (px_dimutex_ctxt->hl_listner_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         DMUT_LOG_LOW("task_start failed: %d", e_task_ret);
         e_task_ret = task_delete (px_dimutex_ctxt->hl_listner_task_hdl);
         if (eTASK_RET_SUCCESS != e_task_ret)
         {
            DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
         }
         px_dimutex_ctxt->hl_listner_task_hdl = NULL;
         e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      }
      else
      {
         DMUT_LOG_MED("task_create success for hl_listner_task_hdl");
         DMUT_LOG_MED("Dimutex Init Success");
         *phl_dimutex_hdl = (DIMUTEX_HDL) px_dimutex_ctxt;
         e_error = eDIMUTEX_RET_SUCCESS;
      }
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_deinit (
   DIMUTEX_HDL hl_dimutex_hdl)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;

   if (NULL == hl_dimutex_hdl)
   {
      DMUT_LOG_LOW("Invalid Handle");
      e_error = eDIMUTEX_RET_INVALID_HANDLE;
      goto CLEAN_RETURN;
   }

   px_dimutex_ctxt = (DIMUTEX_CTXT_X *) hl_dimutex_hdl;

   if (NULL != px_dimutex_ctxt->hl_listner_task_hdl)
   {
      e_task_ret = task_delete (px_dimutex_ctxt->hl_listner_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         DMUT_LOG_LOW("task_delete failed: %d", e_task_ret);
      }
      px_dimutex_ctxt->hl_listner_task_hdl = NULL;
   }

   pal_free(px_dimutex_ctxt);
   px_dimutex_ctxt = NULL;
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}
