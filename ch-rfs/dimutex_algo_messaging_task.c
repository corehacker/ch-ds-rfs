/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo_messaging_task.c
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
static DIMUTEX_RET_E dimutex_algo_messaging_task_handle_msg (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_ALGO_MSG_DATA_X *px_msg_data);

static DIMUTEX_RET_E dimutex_algo_msg_handle_request (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx);

static DIMUTEX_RET_E dimutex_algo_msg_handle_reply (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx);

static DIMUTEX_RET_E dimutex_handle_server_serialize_data_rsp (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx);

/****************************** LOCAL FUNCTIONS *******************************/
void *dimutex_algo_messaging_task(
   void *p_thread_args)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   MSGQ_DATA_X x_data = {NULL};
   DIMUTEX_ALGO_MSG_DATA_X *px_msg_data = NULL;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;

   if (NULL == p_thread_args)
   {
      DMUT_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   px_dimutex_ctxt = (DIMUTEX_CTXT_X *) p_thread_args;
   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   /*
    * Roucairol and Carvalho Algorithm:
    *
    * Shared Database:
    *    CONSTANT
    *       ME, N;
    *
    *    INTEGER
    *       Our_Sequence_Number; Initial = 0;
    *       Hgst_Sequence_Number; Initial = 0;
    *
    *    BOOLEAN
    *       A[1:N]; Initial = false;
    *       Using; Initial = false;
    *       Waiting; Initial = false;
    *       Reply_Deferred[1:N]; Initial = false;
    *
    *
    * procedure TREAT-REPLY-MESSAGE(j)
    * {
    *    A[j] := true;
    * }
    *
    * procedure TREAT-REQUEST-MESSAGE(Their_Sequence_Number, j)
    * {
    *    BOOLEAN Our_Priority;
    *
    *    Hgst_Sequence_Number :=
    *       max (Hgst_Sequence_Number, Their_Sequence_Number);
    *
    *    Our_Priority := ((Their_Sequence_Number > Our_Sequence_Number) ||
    *       ((Their_Sequence_Number = Our_Sequence_Number) && (j > ME)));
    *
    *    if ((true == Using) || ((true == Waiting) && (true == Our_Priority))
    *    {
    *       Reply_Deferred[j] := true;
    *    }
    *
    *    if ((!((true == Using) || (true == Waiting)) ||
    *       ((true == Waiting) && ((false == A[j]) && (false == Our_Priority))))
    *    {
    *       A[j] := false;
    *       send (REPLY(ME), j);
    *    }
    *
    *    if ((true = Waiting) && (true == A[j]) && (false == Our_Priority))
    *    {
    *       A[j] := false;
    *       send (REPLY(ME), j);
    *       send (REQUEST (Our_Sequence_Number, ME), j)
    *    }
    * }
    */

   while (task_is_in_loop (px_resources->hl_messaging_task_hdl))
   {
      e_task_ret = task_get_msg_from_q (
         px_resources->hl_messaging_task_hdl, &x_data,
         DIMUTEX_ALGO_MESSAGING_TASK_Q_WAIT_TIMEOUT);
      if ((eTASK_RET_SUCCESS == e_task_ret) && (NULL != x_data.p_data)
         && (0 != x_data.ui_data_size))
      {
         px_msg_data = (DIMUTEX_ALGO_MSG_DATA_X *) x_data.p_data;
         e_error = dimutex_algo_messaging_task_handle_msg (px_dimutex_ctxt,
            px_msg_data);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_MED("dimutex_algo_messaging_task_handle_msg failed: %d",
               e_error);
         }
      }

      DMUT_LOG_MED("Messaging Task Loop");
   }

   DMUT_LOG_MED("Out of task loop");
CLEAN_RETURN:
   DMUT_LOG_MED("Notifying task exit");
   e_task_ret = task_notify_exit (
      px_resources->hl_messaging_task_hdl);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      DMUT_LOG_LOW("task_notify_exit failed: %d", e_task_ret);
   }
   else
   {
      DMUT_LOG_MED("task_notify_exit success");
   }
   return p_thread_args;
}

static DIMUTEX_RET_E dimutex_algo_messaging_task_handle_msg (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_ALGO_MSG_DATA_X *px_msg_data)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_MSG_HDR_X *px_msg_hdr = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_data))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (px_msg_data->ui_data_len < sizeof(DIMUTEX_MSG_HDR_X))
   {
      DMUT_LOG_LOW("Invalid Args. Invalid px_msg_data->ui_data_len: %d",
         px_msg_data->ui_data_len);
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_msg_hdr = (DIMUTEX_MSG_HDR_X *) px_msg_data->puc_data;
   switch (px_msg_hdr->ui_msg_id)
   {
      case eNODE_MSG_ID_ALGO_REQUEST:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_REQUEST");
         e_error = dimutex_algo_msg_handle_request (px_dimutex_ctxt, px_msg_hdr,
            px_msg_data->ui_src_node_index);
         break;
      }
      case eNODE_MSG_ID_ALGO_REPLY:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_REPLY");
         e_error = dimutex_algo_msg_handle_reply (px_dimutex_ctxt, px_msg_hdr,
            px_msg_data->ui_src_node_index);
         break;
      }
      case eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP");
         e_error = dimutex_handle_server_serialize_data_rsp (px_dimutex_ctxt,
            px_msg_hdr, px_msg_data->ui_src_node_index);
         break;
      }
      default:
      {
         DMUT_LOG_LOW("Invalid message: %d received.", px_msg_hdr->ui_msg_id);
         e_error = eDIMUTEX_RET_INVALID_ARGS;
         break;
      }
   }

CLEAN_RETURN:
   if (NULL != px_msg_data)
   {
      if (NULL != px_msg_data->puc_data)
      {
         pal_free(px_msg_data->puc_data);
         px_msg_data->puc_data = NULL;
      }
      pal_free(px_msg_data);
      px_msg_data = NULL;
   }
   return e_error;
}

static DIMUTEX_RET_E dimutex_algo_msg_handle_request (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   bool b_our_priority = false;
   NODE_MSG_ALGO_REQUEST_X *px_algo_req = NULL;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_DB_X *px_db = NULL;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_db = &(px_dimutex_ctxt->x_algo.x_db);
   px_algo_req = (NODE_MSG_ALGO_REQUEST_X *) px_msg_hdr;

   DMUT_LOG_FULL("[MSG]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[MSG]After pal_mutex_lock");

   DMUT_LOG_LOW(
      "[MSG]Got request from node: %d, Their Seq No: %d, Our Seq No: %d, "
      "Highest Seq No: %d",
      ui_node_idx, px_algo_req->ui_seq_number, px_db->ui_our_seq_no, px_db->ui_highest_seq_no);

   px_db->ui_highest_seq_no =
      PAL_MAX(px_db->ui_highest_seq_no, px_algo_req->ui_seq_number);

   DMUT_LOG_LOW("[MSG]Updated highest Seq No: %d", px_db->ui_highest_seq_no);

   b_our_priority = (
         ((px_algo_req->ui_seq_number > px_db->ui_our_seq_no)
            || ((px_algo_req->ui_seq_number == px_db->ui_our_seq_no)
               && (ui_node_idx > px_dimutex_ctxt->x_nodes.ui_this_node_idx))) ?
               (true) : (false));

   DMUT_LOG_LOW("[MSG]Does the other node %d request has higher priority? %s",
      ui_node_idx, (false == b_our_priority) ? ("TRUE") : ("FALSE"));
   DMUT_LOG_LOW("[MSG]Does my node request has higher priority? %d",
      b_our_priority);

   if ((px_db->b_using) || (px_db->b_waiting && b_our_priority))
   {
      if (px_db->b_using)
      {
         DMUT_LOG_LOW("[MSG]Reply to %d deferred because I am using it!",
            ui_node_idx);
      }
      else
      {
         DMUT_LOG_LOW("[MSG]Reply to %d deferred because I am waiting for the "
         "resource and I have a higher priority request.", ui_node_idx);
      }
      px_db->ba_reply_referred [ui_node_idx] = true;
   }

   if (!(px_db->b_using || px_db->b_waiting)
      || (px_db->b_waiting && !px_db->ba_a [ui_node_idx] && !b_our_priority))
   {
      px_db->ba_a [ui_node_idx] = false;

      DMUT_LOG_LOW(
         "[MSG]A request was pending from node: %d. Sending a reply to it.",
         ui_node_idx);
      e_error = dimutex_node_send_algo_reply (px_dimutex_ctxt, ui_node_idx);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_algo_reply failed: %d", e_error);
         e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
         }
         goto CLEAN_RETURN;
      }
   }

   if (px_db->b_waiting && px_db->ba_a [ui_node_idx] && !b_our_priority)
   {
      px_db->ba_a [ui_node_idx] = false;

      DMUT_LOG_LOW(
         "[MSG]A request was pending from node: %d. Sending a reply to it.",
         ui_node_idx);
      DMUT_LOG_LOW(
         "[MSG]A request was also made by me. Sending a request for the same.");

      e_error = dimutex_node_send_algo_reply (px_dimutex_ctxt, ui_node_idx);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_algo_reply failed: %d", e_error);
         e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
         }
         goto CLEAN_RETURN;
      }

      e_error = dimutex_node_send_algo_request (px_dimutex_ctxt, ui_node_idx);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_algo_request failed: %d", e_error);
         e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
         }
         goto CLEAN_RETURN;
      }
      else
      {
         px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);
         px_stat_entry = &(px_stats->xa_cs_entry_stat[px_stats->ui_count]);
         px_stat_entry->ui_no_request_msgs_sent++;
      }
   }
   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_algo_msg_handle_reply (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_DB_X *px_db = NULL;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr)
      || (ui_node_idx >= px_dimutex_ctxt->x_init_params.ui_no_nodes))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_db = &(px_dimutex_ctxt->x_algo.x_db);
   px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);
   DMUT_LOG_FULL("[MSG]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[MSG]After pal_mutex_lock");

   px_stat_entry = &(px_stats->xa_cs_entry_stat[px_stats->ui_count]);

   DMUT_LOG_LOW("Got reply from node %d", ui_node_idx);

   px_db->ba_a [ui_node_idx] = true;
   px_stat_entry->ui_no_reply_msgs_received++;

   if (true == px_db->b_waiting)
   {
      for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
      {
         if (ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
         {
            continue;
         }
         if (eDIMUTEX_NODE_TYPE_SERVER
            == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
         {
            continue;
         }
         if (false == px_db->ba_a [ui_index])
         {
            break;
         }
      }

      if (ui_index == px_dimutex_ctxt->x_init_params.ui_no_nodes)
      {
         DMUT_LOG_LOW(
            "[MSG]All nodes have replied. Notifying the resource task.");

         DMUT_LOG_FULL("[MSG]Before pal_sem_put");
         /*
          * All nodes have sent a reply. Put the semaphore.
          */
         e_pal_ret = pal_sem_put (px_resources->hl_algo_sem_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_sem_put failed: %d", e_pal_ret);
            e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
            if (ePAL_RET_SUCCESS != e_pal_ret)
            {
               DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
            }
            e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
            goto CLEAN_RETURN;
         }
         DMUT_LOG_FULL("[MSG]After pal_sem_put");
      }
   }

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_handle_server_serialize_data_rsp (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr,
   uint32_t ui_node_idx)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_DB_X *px_db = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr)
      || (ui_node_idx >= px_dimutex_ctxt->x_init_params.ui_no_nodes))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_db = &(px_dimutex_ctxt->x_algo.x_db);

   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");

   px_db->ba_awaiting_ser_rsp[ui_node_idx] = false;

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (true == px_db->ba_awaiting_ser_rsp[ui_index])
      {
         break;
      }
   }

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   if (ui_index == px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW(
         "[MSG]All servers have replied for serialize. "
         "Notifying the resource task.");

      DMUT_LOG_FULL("[MSG]Before pal_sem_put");
      /*
       * All nodes have sent a reply. Put the semaphore.
       */
      e_pal_ret = pal_sem_put (px_resources->hl_algo_ser_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
         goto CLEAN_RETURN;
      }
      DMUT_LOG_FULL("[MSG]After pal_sem_put");
   }
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}
