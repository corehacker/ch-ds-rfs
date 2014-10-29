/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo_resource_task.c
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
#define DIMUTEX_SERIALIZE_DATA_MAX_LEN       (512)

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static DIMUTEX_RET_E dimutex_algo_check_complete_status(
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   bool *pb_is_complete);

static DIMUTEX_RET_E dimutex_algo_post_complete_to_main_task (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_algo_request_resource (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_algo_release_resource (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_algo_enter_critical_section (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static uint32_t dimutex_algo_get_rand_bw_10_50();

static DIMUTEX_RET_E dimutex_algo_compute_times (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

/****************************** LOCAL FUNCTIONS *******************************/
void *dimutex_algo_resource_task(
   void *p_thread_args)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   bool b_is_complete = false;

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
    * procedure REQUEST-RESOURCE ()
    * {
    *    Integer j;
    *    Waiting := true;
    *    Our_Sequence_Number := Hgst_Sequence_Number + 1;
    *
    *    for (j := 1; j <= N; j := j + 1)
    *    {
    *       if ((j != ME) && (false == A[j]))
    *       {
    *          send (REQUEST (Our_Sequence_Number, ME), j);
    *       }
    *    }
    *
    *    waitfor (A[j] = true for all j != ME);
    *    Waiting := false;
    *    Using := true;
    * }
    *
    * procedure RELEASE-RESOURCE ()
    * {
    *    Integer i;
    *    Using := false;
    *    for (j := 1; j <= N; j := j + 1)
    *    {
    *       if (true == Reply_Deferred[j])
    *       {
    *          A[j] := false;
    *          Reply_Deferred[j] := false;
    *
    *          Send (REPLY (ME), j);
    *       }
    *    }
    * }
    */
   while (task_is_in_loop (px_resources->hl_resource_task_hdl))
   {
      e_error = dimutex_algo_request_resource (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_algo_request_resource failed: %d", e_error);
      }
      e_error = dimutex_algo_enter_critical_section (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_algo_enter_critical_section failed: %d",
            e_error);
      }
      e_error = dimutex_algo_release_resource (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_algo_release_resource failed: %d", e_error);
      }
      e_error = dimutex_algo_compute_times (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_algo_compute_times failed: %d", e_error);
      }
      e_error = dimutex_algo_check_complete_status (px_dimutex_ctxt,
         &b_is_complete);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_algo_check_complete_status failed: %d", e_error);
      }
      if (true == b_is_complete)
      {
         DMUT_LOG_LOW("Algorithm execution is complete. Waiting for teardown "
         "from leader");
         break;
      }
      DMUT_LOG_MED("[RES]Resource Task Loop");
   }

   DMUT_LOG_MED("Out of task loop");
CLEAN_RETURN:
   DMUT_LOG_MED("Notifying task exit");
   e_task_ret = task_notify_exit (px_resources->hl_resource_task_hdl);
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

static DIMUTEX_RET_E dimutex_algo_check_complete_status(
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   bool *pb_is_complete)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == pb_is_complete))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   *pb_is_complete = false;
   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);
   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");

   DMUT_LOG_LOW("[RES]This node has entered critical section %d times.",
      px_stats->ui_count);
   if (px_stats->ui_count >= DIMUTEX_ALGO_CS_MAX_REQUEST_COUNT)
   {
      if (false == px_resources->b_send_complete)
      {
         px_resources->b_send_complete = true;

         /*
          * Send completion message to leader.
          */
         if (0 != px_dimutex_ctxt->x_nodes.ui_this_node_idx)
         {
            e_error = dimutex_node_send_algo_complete (px_dimutex_ctxt, 0);
            if (eDIMUTEX_RET_SUCCESS != e_error)
            {
               DMUT_LOG_LOW("dimutex_node_send_algo_complete failed: %d",
                  e_error);
            }
         }
         else
         {
            e_error = dimutex_algo_post_complete_to_main_task (px_dimutex_ctxt);
            if (eDIMUTEX_RET_SUCCESS != e_error)
            {
               DMUT_LOG_LOW(
                  "dimutex_algo_post_complete_to_main_task failed: %d",
                  e_error);
            }
         }
      }
      else
      {
         e_error = eDIMUTEX_RET_SUCCESS;
      }
   }

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }

   if (px_stats->ui_count >= DIMUTEX_ALGO_CS_MAX_REQUEST_COUNT)
   {
      *pb_is_complete = true;
   }
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_algo_post_complete_to_main_task (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_ALGO_COMPLETE_X *px_algo_complete = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_algo_complete = pal_malloc (sizeof(DIMUTEX_ALGO_COMPLETE_X), NULL);
   if (NULL == px_algo_complete)
   {
      DMUT_LOG_LOW("pal_malloc failed");
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_LOW("Posting eDIMUTEX_MSG_ID_ALGO_COMPLETE to myself");
   px_algo_complete->x_hdr.ui_msg_id = eDIMUTEX_MSG_ID_ALGO_COMPLETE;
   px_algo_complete->x_hdr.ui_msg_pay_len = sizeof(*px_algo_complete) -
         sizeof(px_algo_complete->x_hdr);

   (void) pal_memmove (&(px_algo_complete->x_stats),
      &(px_dimutex_ctxt->x_algo.x_algo_stats),
      sizeof(px_algo_complete->x_stats));

   e_error = dimutex_post_msg_to_q (px_dimutex_ctxt,
      (uint8_t *) px_algo_complete, sizeof(*px_algo_complete));
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_post_msg_to_q failed: %d", e_error);
      pal_free (px_algo_complete);
      px_algo_complete = NULL;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_algo_request_resource (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_DB_X *px_db = NULL;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_db = &(px_dimutex_ctxt->x_algo.x_db);
   px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);

   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");

   px_stat_entry = &(px_stats->xa_cs_entry_stat[px_stats->ui_count]);

   DMUT_LOG_LOW("[RES]Initiated resource request for %d @ %d ms.",
      px_stats->ui_count, pal_get_system_time());

   px_db->b_waiting = true;

   px_stat_entry->ui_cs_req_time_ms = pal_get_system_time();

   px_db->ui_our_seq_no = px_db->ui_highest_seq_no + 1;

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
      {
         DMUT_LOG_LOW("[RES]Not Sending dimutex_node_send_algo_request to %d "
            "(myself)",
            ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (true == px_db->ba_a[ui_index])
      {
         DMUT_LOG_LOW("[RES]Not Sending dimutex_node_send_algo_request to %d",
            ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         DMUT_LOG_LOW("[RES]Not Sending dimutex_node_send_algo_request to %d "
            "because it is a server",
            ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }


      DMUT_LOG_LOW(
         "[RES]Sending dimutex_node_send_algo_request to %d (%s). Our "
         "Seq No.: %d",
         ui_index,
         ((eDIMUTEX_NODE_TYPE_SERVER ==
            px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type) ?
               ("Server") :
                  ((eDIMUTEX_NODE_TYPE_CLIENT ==
                     px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type) ?
                        ("Client") :
                        ("Unknown"))),
         px_db->ui_our_seq_no);
      e_error = dimutex_node_send_algo_request (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_algo_request failed: %d", e_error);
         break;
      }
      else
      {
         px_stat_entry->ui_no_request_msgs_sent++;
      }
   }

   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      px_db->b_waiting = false;
      e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
      if (e_pal_ret != ePAL_RET_SUCCESS)
      {
         DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      }
      goto CLEAN_RETURN;
   }

   if (0 != px_stat_entry->ui_no_request_msgs_sent)
   {
      e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
      if (e_pal_ret != ePAL_RET_SUCCESS)
      {
         px_db->b_waiting = false;
         DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
         e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
         goto CLEAN_RETURN;
      }

      DMUT_LOG_LOW( "[RES]Sent request to all nodes (%d) to all @ %d ms. "
      "Waiting for response from all.",
         px_dimutex_ctxt->x_init_params.ui_no_nodes, px_stat_entry->ui_cs_req_time_ms);
      /*
       * Wait for reply from all nodes.
       */
      DMUT_LOG_FULL("[RES]Before pal_sem_get");
      e_pal_ret = pal_sem_get (px_resources->hl_algo_sem_hdl,
         PAL_TIME_WAIT_INFINITE);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         px_db->b_waiting = false;
         DMUT_LOG_LOW("pal_sem_get failed: %d", e_pal_ret);
         e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
         goto CLEAN_RETURN;
      }
      DMUT_LOG_FULL("[RES]After pal_sem_get");

      DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
      e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
      if (e_pal_ret != ePAL_RET_SUCCESS)
      {
         px_db->b_waiting = false;
         DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
         e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
         goto CLEAN_RETURN;
      }
      DMUT_LOG_FULL("[RES]After pal_mutex_lock");
   }

   px_db->b_waiting = false;
   px_db->b_using = true;
   px_stat_entry->ui_cs_grant_time_ms = pal_get_system_time();
   DMUT_LOG_LOW("[RES]Got notification from messaging task @ %d ms. "
      "Got response from all.", px_stat_entry->ui_cs_grant_time_ms);
   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
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

static DIMUTEX_RET_E dimutex_algo_release_resource (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_DB_X *px_db = NULL;

   if (NULL == px_dimutex_ctxt)
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

   DMUT_LOG_LOW("[RES]Releasing resource @ %d ms.", pal_get_system_time());

   px_db->b_using = false;

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
   {
      if (true == px_db->ba_reply_referred[ui_index])
      {
         px_db->ba_a[ui_index] = false;
         px_db->ba_reply_referred[ui_index] = false;

         DMUT_LOG_LOW("[RES]Sending reply to node %d which was deferred.",
            ui_index);
         e_error = dimutex_node_send_algo_reply (px_dimutex_ctxt, ui_index);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_send_algo_reply failed: %d", e_error);
            break;
         }
      }
   }

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
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

static DIMUTEX_RET_E dimutex_algo_enter_critical_section (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;
   DIMUTEX_NODE_SERIALIZE_DATA_X x_serialization_data = {{0}};
   uint8_t uca_serialize_data[DIMUTEX_SERIALIZE_DATA_MAX_LEN] = {0};
   uint32_t ui_serialize_data_len = 0;
   DIMUTEX_NODES_X *px_nodes = NULL;
   DIMUTEX_NODE_CTXT_X *px_node_ctxt = NULL;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);
   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");

   px_stat_entry = &(px_stats->xa_cs_entry_stat[px_stats->ui_count]);

   printf("[RES]****************************************************************************************************\n");
   printf ("[RES]*CS Entry: Current Count: %d, Req @: %d ms, Granted @: %d ms, "
      "Req Sent: %d, Reply Recvd: %d, Will Hold For %d ms*\n",
      (px_stats->ui_count + 1),
      px_stat_entry->ui_cs_req_time_ms,
      px_stat_entry->ui_cs_grant_time_ms,
      px_stat_entry->ui_no_request_msgs_sent,
      px_stat_entry->ui_no_reply_msgs_received,
      (DIMUTEX_ALGO_CS_EXECUTION_MAX_UNITS * px_stats->ui_unit_time_ms));
   printf("[RES]****************************************************************************************************\n\n");

   DMUT_LOG_LOW("[RES]****************************************************************************************************");
   DMUT_LOG_LOW ("[RES]*CS Entry: Current Count: %d, Req @: %d ms, Granted @: %d ms, "
      "Req Sent: %d, Reply Recvd: %d, Will Hold For %d ms*",
      (px_stats->ui_count + 1),
      px_stat_entry->ui_cs_req_time_ms,
      px_stat_entry->ui_cs_grant_time_ms,
      px_stat_entry->ui_no_request_msgs_sent,
      px_stat_entry->ui_no_reply_msgs_received,
      (DIMUTEX_ALGO_CS_EXECUTION_MAX_UNITS * px_stats->ui_unit_time_ms));
   DMUT_LOG_LOW("[RES]****************************************************************************************************");

   px_nodes = &(px_dimutex_ctxt->x_nodes);
   px_node_ctxt = &(px_nodes->xa_nodes[px_nodes->ui_this_node_idx]);
   (void) snprintf((char *) uca_serialize_data, sizeof(uca_serialize_data),
      "%2d : %2d : %s\n", px_nodes->ui_this_node_idx, (px_stats->ui_count + 1),
      px_node_ctxt->uca_node_dns_name_str);
   ui_serialize_data_len = pal_strnlen (uca_serialize_data,
      sizeof(uca_serialize_data));
   ui_serialize_data_len += 1; // For the '\0' character.

   x_serialization_data.puc_serialization_data = uca_serialize_data;
   x_serialization_data.ui_serialization_data_len = ui_serialize_data_len;
   (void) pal_strncpy (x_serialization_data.uca_serialize_res_id_str,
      (const uint8_t *) DIMUTEX_SERIALIZATION_RES_ID_STR,
      sizeof(x_serialization_data.uca_serialize_res_id_str));
   e_error = dimutex_node_send_serialize_data_to_all_servers (px_dimutex_ctxt,
      &x_serialization_data);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_send_serialize_data_to_all_servers failed: %d",
         e_error);
   }

   DMUT_LOG_LOW("Sent serialize request to all servers. Waiting for their "
      "responses");

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_FULL("[RES]Before pal_sem_get");
   e_pal_ret = pal_sem_get (px_resources->hl_algo_ser_hdl,
      PAL_TIME_WAIT_INFINITE);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sem_get failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_sem_get");

   pal_sleep((DIMUTEX_ALGO_CS_EXECUTION_MAX_UNITS * px_stats->ui_unit_time_ms));
   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");
   DMUT_LOG_LOW("[RES]Exiting critical section.");

   px_stats->ui_count++;

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
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

static uint32_t dimutex_algo_get_rand_bw_10_50()
{
   uint32_t ui_rand = 0;
   uint32_t ui_retries = 10;

   while (1)
   {
      ui_rand = pal_rand32 ();
      ui_rand = ui_rand % 50;
      if (ui_rand < 10)
      {
         if (0 == ui_retries)
         {
            ui_rand = 10 + (pal_rand32 () % 10);
            break;
         }
         ui_retries--;
         continue;
      }
      else
      {
         break;
      }
   }
   return ui_rand;
}

static DIMUTEX_RET_E dimutex_algo_compute_times (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_ALGO_RESOURCES_X *px_resources = NULL;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   DIMUTEX_ALGO_STATS_X *px_stats = NULL;
   uint32_t ui_temp = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_resources = &(px_dimutex_ctxt->x_algo.x_resources);
   px_stats = &(px_dimutex_ctxt->x_algo.x_algo_stats);
   DMUT_LOG_FULL("[RES]Before pal_mutex_lock");
   e_pal_ret = pal_mutex_lock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_lock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   DMUT_LOG_FULL("[RES]After pal_mutex_lock");


   ui_temp = dimutex_algo_get_rand_bw_10_50 ();
   px_resources->ui_next_entry_in_ms = (ui_temp * px_stats->ui_unit_time_ms);
   DMUT_LOG_LOW( "[RES]Generated Random No: %d, Next entry in %d ms", ui_temp,
      px_resources->ui_next_entry_in_ms);

   e_pal_ret = pal_mutex_unlock (px_resources->hl_algo_mutex_hdl);
   if (e_pal_ret != ePAL_RET_SUCCESS)
   {
      DMUT_LOG_LOW("pal_mutex_unlock failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("[RES]Waiting %d ms till next CS request",
         px_resources->ui_next_entry_in_ms);
      pal_sleep(px_resources->ui_next_entry_in_ms);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}
