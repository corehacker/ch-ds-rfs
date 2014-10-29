/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node.c
 * \author sandeepprakash
 *
 * \date   26-Sep-2012
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
static DIMUTEX_RET_E dimutex_node_task_init_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_node_task_init (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_node_task_deinit (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_node_generate_ping (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_node_handle_msgs (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr);

static DIMUTEX_RET_E dimutex_node_handle_listen_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

static DIMUTEX_RET_E dimutex_node_handle_msgs_from_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint8_t *puc_msg_data,
   uint32_t ui_msg_size);

static DIMUTEX_RET_E dimutex_node_handle_data_from_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint8_t *puc_msg_data,
   uint32_t ui_msg_size);

static DIMUTEX_RET_E dimutex_node_handle_conn_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl);

static DIMUTEX_RET_E dimutex_node_task_handle_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr);

static DIMUTEX_RET_E dimutex_node_handle_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr);

/****************************** LOCAL FUNCTIONS *******************************/
void *dimutex_node_task(
   void *p_thread_args)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   MSGQ_DATA_X x_data = {NULL};
   DIMUTEX_MSG_HDR_X *px_msg_hdr = NULL;

   if (NULL == p_thread_args)
   {
      DMUT_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   px_dimutex_ctxt = (DIMUTEX_CTXT_X *) p_thread_args;

   e_error = dimutex_node_task_init (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_task_init failed: %d", e_error);
      goto CLEAN_RETURN;
   }
   DMUT_LOG_MED("dimutex_node_task_init success");

   while (task_is_in_loop (px_dimutex_ctxt->hl_listner_task_hdl))
   {
      e_task_ret = task_get_msg_from_q(px_dimutex_ctxt->hl_listner_task_hdl,
         &x_data, DIMUTEX_TASK_Q_WAIT_TIMEOUT);
      if ((eTASK_RET_SUCCESS == e_task_ret) && (NULL != x_data.p_data)
         && (0 != x_data.ui_data_size))
      {
         px_msg_hdr = (DIMUTEX_MSG_HDR_X *) x_data.p_data;

         e_error = dimutex_node_handle_msgs (px_dimutex_ctxt, px_msg_hdr);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_handle_msgs failed: %d", e_error);
         }
         else
         {
            dimutex_node_log_status (px_dimutex_ctxt);
         }
      }

      e_error = dimutex_node_generate_ping (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_FULL("dimutex_node_generate_ping failed: %d", e_error);
      }

      DMUT_LOG_FULL("Dimutex Listener Task");
   }

   DMUT_LOG_MED("Out of task loop");
   e_error = dimutex_node_task_deinit (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_task_deinit failed: %d", e_error);
   }
   DMUT_LOG_MED("dimutex_node_task_deinit success");
CLEAN_RETURN:
   DMUT_LOG_MED("Notifying task exit");
   e_task_ret = task_notify_exit (px_dimutex_ctxt->hl_listner_task_hdl);
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

static DIMUTEX_RET_E dimutex_node_handle_listen_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_ADDR_IN_X x_in_addr = {0};
   PAL_SOCK_HDL *phl_sock_hdl = NULL;
   uint32_t ui_index = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < DIMUTEX_MAX_NODES; ui_index++)
   {
      if (NULL == px_dimutex_ctxt->hla_temp_node_sock [ui_index])
      {
         DMUT_LOG_LOW("Found empty sock context @ %d index", ui_index);
         break;
      }
   }

   if (ui_index >= DIMUTEX_MAX_NODES)
   {
      DMUT_LOG_LOW("No empty temp socket present.");
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   phl_sock_hdl = &(px_dimutex_ctxt->hla_temp_node_sock [ui_index]);

   DMUT_LOG_MED("New connection. Accepting it.");
   e_pal_ret = pal_sock_accept (px_dimutex_ctxt->hl_listner_sock_hdl,
      &x_in_addr, phl_sock_hdl);
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (NULL == *phl_sock_hdl))
   {
      DMUT_LOG_MED ("pal_sock_accept failed: %d", e_pal_ret);
   }
   else
   {
      DMUT_LOG_MED("New connection on listen socket.");

      e_error = dimutex_node_handle_conn_sock_act (px_dimutex_ctxt,
         phl_sock_hdl);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_handle_conn_sock_act failed: %d", e_error);
         e_pal_ret = pal_sock_close (*phl_sock_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_MED("pal_sock_close failed: %d", e_pal_ret);
         }
         *phl_sock_hdl = NULL;
      }
   }

   DMUT_LOG_MED("Re-registering the listen socket.");
   e_error = dimutex_node_register_sock (px_dimutex_ctxt,
      px_dimutex_ctxt->hl_listner_sock_hdl);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_register_sock failed: %d",
         e_error);
   }

CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_handle_msgs_from_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint8_t *puc_msg_data,
   uint32_t ui_msg_size)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   NODE_MSG_HDR_X *px_msg_header = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == puc_msg_data) || (0 == ui_msg_size))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_msg_header = (NODE_MSG_HDR_X *) px_dimutex_ctxt->uca_temp_sock_buf;
   switch (px_msg_header->ui_msg_id)
   {
      /*
       * Node Setup Messages.
       */
      case eNODE_MSG_ID_SETUP_JOIN:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SETUP_JOIN");
         e_error = dimutex_node_handle_setup_join (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_SETUP_JOIN_DONE:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SETUP_JOIN_DONE");
         e_error = dimutex_node_handle_setup_join_done (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_SETUP_ESTABLISH_PEERS:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SETUP_ESTABLISH_PEERS");
         e_error = dimutex_node_handle_setup_establish_peers (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_SETUP_ESTABLISH_DONE:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SETUP_ESTABLISH_DONE");
         e_error = dimutex_node_handle_setup_establish_done (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_SETUP_PING:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SETUP_PING");
         e_error = dimutex_node_handle_setup_ping (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }

      /*
       * Algorithm Messages.
       */
      case eNODE_MSG_ID_ALGO_SETUP:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_SETUP");
         e_error = dimutex_node_handle_algo_setup (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_SETUP_COMPLETE:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_SETUP_COMPLETE");
         e_error = dimutex_node_handle_algo_setup_complete (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_TEARDOWN:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_TEARDOWN");
         e_error = dimutex_node_handle_algo_teardown (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_TEARDOWN_COMPLETE:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_TEARDOWN_COMPLETE");
         e_error = dimutex_node_handle_algo_teardown_complete (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_START:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_START");
         e_error = dimutex_node_handle_algo_start (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_STOP:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_STOP");
         e_error = dimutex_node_handle_algo_stop (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_COMPLETE:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_COMPLETE");
         e_error = dimutex_node_handle_algo_complete (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_REQUEST:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_REQUEST");
         e_error = dimutex_node_handle_forward_msg (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_ALGO_REPLY:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_ALGO_REPLY");
         e_error = dimutex_node_handle_forward_msg (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }

      /*
       * Server messages.
       */
      case eNODE_MSG_ID_SERVER_SERIALIZE_DATA_REQ:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SERVER_SERIALIZE_DATA_REQ");
         e_error = dimutex_node_handle_server_serialize_data_req (
            px_dimutex_ctxt, phl_act_sock_hdl, px_msg_header);
         break;
      }
      case eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP:
      {
         DMUT_LOG_MED("Got eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP");
         e_error = dimutex_node_handle_forward_msg (px_dimutex_ctxt,
            phl_act_sock_hdl, px_msg_header);
         break;
      }
      default:
      {
         DMUT_LOG_LOW("Invalid message: %d received.",
            px_msg_header->ui_msg_id);
         e_error = eDIMUTEX_RET_INVALID_ARGS;
         break;
      }
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_handle_data_from_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint8_t *puc_msg_data,
   uint32_t ui_msg_size)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == puc_msg_data) || (0 == ui_msg_size))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_error = dimutex_node_register_sock (px_dimutex_ctxt, *phl_act_sock_hdl);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_register_sock failed: %d", e_error);

   }
   else
   {
      e_error = dimutex_node_handle_msgs_from_nodes (px_dimutex_ctxt,
         phl_act_sock_hdl, puc_msg_data, ui_msg_size);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_handle_msgs_from_clients failed: %d",
            e_error);
      }
   }
CLEAN_RETURN:
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      if ((NULL != phl_act_sock_hdl) && (NULL != *phl_act_sock_hdl))
      {
         e_pal_ret = pal_sock_close (*phl_act_sock_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_MED("pal_sock_close failed: %d", e_pal_ret);
         }
         *phl_act_sock_hdl = NULL;
      }
   }
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_handle_conn_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_recv_size = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   ui_recv_size = sizeof(px_dimutex_ctxt->uca_temp_sock_buf);

   e_error = dimutex_node_read_msg (*phl_act_sock_hdl,
      px_dimutex_ctxt->uca_temp_sock_buf,
      sizeof(px_dimutex_ctxt->uca_temp_sock_buf), &ui_recv_size);
   if ((eDIMUTEX_RET_SUCCESS == e_error)
      && (ui_recv_size >= sizeof(NODE_MSG_HDR_X)))
   {
      DMUT_LOG_LOW("Received %d bytes.", ui_recv_size);
      e_error = dimutex_node_handle_data_from_nodes (px_dimutex_ctxt,
         phl_act_sock_hdl, px_dimutex_ctxt->uca_temp_sock_buf, ui_recv_size);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_handle_data_from_clients failed: %d",
            e_error);
      }
   }
   else
   {
      DMUT_LOG_LOW("dimutex_node_read_msg failed: %d. Hence closing all sockets.",
         e_pal_ret);
      e_pal_ret = pal_sock_close(*phl_act_sock_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sock_close failed: %d", e_pal_ret);
      }
      *phl_act_sock_hdl = NULL;

      (void) dimutex_node_cleanup_socks (px_dimutex_ctxt);

      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_task_handle_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_ALGO_COMPLETE_X *px_algo_complete = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_algo_complete = (DIMUTEX_ALGO_COMPLETE_X *) px_msg_hdr;

   e_error = dimutex_node_algo_complete (px_dimutex_ctxt,
      &(px_algo_complete->x_stats), 0);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_algo_complete failed: %d", e_error);
   }

   pal_free(px_algo_complete);
   px_algo_complete = NULL;
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_handle_sock_act (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_NODE_SOCK_ACT_DATA_X *px_sock_act_data = NULL;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_sock_act_data = (DIMUTEX_NODE_SOCK_ACT_DATA_X *) px_msg_hdr;

   if (px_sock_act_data->hl_sock_hdl
      == px_dimutex_ctxt->hl_listner_sock_hdl)
   {
      e_error = dimutex_node_handle_listen_sock_act (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_handle_listen_sock_act failed: %d",
            e_error);
      }
   }
   else
   {
      for (ui_index = 0; ui_index < DIMUTEX_MAX_NODES; ui_index++)
      {
         if (px_sock_act_data->hl_sock_hdl
            == px_dimutex_ctxt->hla_temp_node_sock [ui_index])
         {
            DMUT_LOG_LOW("Activity on socket @ %d temp index", ui_index);
            break;
         }
      }
      if (ui_index < DIMUTEX_MAX_NODES)
      {
         e_error = dimutex_node_handle_conn_sock_act (px_dimutex_ctxt,
            &(px_dimutex_ctxt->hla_temp_node_sock [ui_index]));
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_handle_conn_sock_act failed: %d",
               e_error);
         }
      }
      else
      {
         for (ui_index = 0;
               ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
               ui_index++)
         {
            if (px_sock_act_data->hl_sock_hdl
               == px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index])
            {
               DMUT_LOG_LOW("Activity on socket from %d node", ui_index);
               break;
            }
         }
         if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
         {
            e_error = dimutex_node_handle_conn_sock_act (px_dimutex_ctxt,
               &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]));
            if (eDIMUTEX_RET_SUCCESS != e_error)
            {
               DMUT_LOG_LOW("dimutex_node_handle_conn_sock_act failed: %d",
                  e_error);
            }
         }
         else
         {
            e_error = eDIMUTEX_RET_SUCCESS;
         }
      }
   }
   pal_free(px_sock_act_data);
   px_sock_act_data = NULL;

CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_generate_ping (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_curr_time_ms = 0;
   uint32_t ui_elapsed_time_ms = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   ui_curr_time_ms = pal_get_system_time ();
   ui_elapsed_time_ms = ui_curr_time_ms - px_dimutex_ctxt->ui_last_ping_time_ms;
   if (ui_elapsed_time_ms < DIMUTEX_TASK_PING_PEERS_INTERVAL_MS)
   {
      e_error = eDIMUTEX_RET_SUCCESS;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_LOW("Generating Ping");
   e_error = dimutex_node_send_ping_to_all (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_node_send_ping_to_all failed: %d", e_error);
   }
   px_dimutex_ctxt->ui_last_ping_time_ms = pal_get_system_time ();
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_handle_msgs (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_MSG_HDR_X *px_msg_hdr)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_msg_hdr))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   switch (px_msg_hdr->ui_msg_id)
   {
      case eDIMUTEX_MSG_ID_LISTENER_SOCK_ACT:
      {
         DMUT_LOG_MED("Got eDIMUTEX_MSG_ID_LISTENER_SOCK_ACT");
         e_error = dimutex_node_handle_sock_act (px_dimutex_ctxt,
            px_msg_hdr);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_handle_sock_act failed: %d", e_error);
         }
         break;
      }
      case eDIMUTEX_MSG_ID_ALGO_COMPLETE:
      {
         DMUT_LOG_MED("Got eDIMUTEX_MSG_ID_ALGO_COMPLETE");
         e_error = dimutex_node_task_handle_algo_complete (px_dimutex_ctxt,
            px_msg_hdr);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_handle_algo_complete failed: %d", e_error);
         }
         break;
      }
      default:
      {
         DMUT_LOG_LOW("Invalid Msg Id: %d", px_msg_hdr->ui_msg_id);
         e_error = eDIMUTEX_RET_INVALID_ARGS;
         break;
      }
   }

CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_task_init_nodes (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   DIMUTEX_INIT_PARAMS_X *px_init_params = NULL;
   DIMUTEX_NODES_X *px_nodes = NULL;
   DIMUTEX_NODE_CTXT_X *px_ldr_node = NULL;
   DIMUTEX_NODE_CTXT_X *px_cur_node = NULL;
   PAL_SOCK_IN_ADDR_X x_in_addr = {0};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_init_params = &(px_dimutex_ctxt->x_init_params);
   px_nodes = &(px_dimutex_ctxt->x_nodes);

   /*
    * Setup the leader node details.
    */
   px_ldr_node = &(px_nodes->xa_nodes[0]);

   (void) pal_strncpy (px_ldr_node->uca_node_dns_name_str,
      px_init_params->uca_leader_host_name_str,
      sizeof(px_ldr_node->uca_node_dns_name_str));
   px_ldr_node->us_node_listen_port_ho = px_init_params->us_listen_port_start_ho
      + 0;
   DMUT_LOG_LOW("Leader: %s:%d", px_ldr_node->uca_node_dns_name_str,
      px_ldr_node->us_node_listen_port_ho);

   /*
    * Setup current node details.
    */
   px_dimutex_ctxt->x_nodes.ui_no_nodes++;
   px_cur_node = &(px_nodes->xa_nodes[px_init_params->ui_node_index]);
   px_nodes->ui_this_node_idx = px_init_params->ui_node_index;
   px_cur_node->us_node_listen_port_ho = px_init_params->us_listen_port_start_ho
      + px_init_params->ui_node_index;
   e_pal_ret = pal_gethostname (px_cur_node->uca_node_dns_name_str,
      sizeof(px_cur_node->uca_node_dns_name_str));
   if (ePAL_RET_FAILURE == e_pal_ret)
   {
      DMUT_LOG_LOW("pal_gethostname failed: %d", e_error);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_gethostbyname (px_cur_node->uca_node_dns_name_str,
      &x_in_addr);
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (0 == x_in_addr.ui_ip_addr_no))
   {
      DMUT_LOG_LOW("pal_gethostbyname failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   px_cur_node->ui_node_ip_addr_ho = pal_ntohl(x_in_addr.ui_ip_addr_no);
   px_cur_node->ui_node_index = px_nodes->ui_this_node_idx;
   px_cur_node->e_node_type = ((true == px_init_params->b_act_as_server) ?
         (eDIMUTEX_NODE_TYPE_SERVER) : (eDIMUTEX_NODE_TYPE_CLIENT));
   DMUT_LOG_LOW("Me: %s (0x%x):%d. Server/Client? %s",
      px_cur_node->uca_node_dns_name_str, px_cur_node->ui_node_ip_addr_ho,
      px_cur_node->us_node_listen_port_ho,
      ((eDIMUTEX_NODE_TYPE_SERVER == px_cur_node->e_node_type) ? ("Server") :
            ("Client")));
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_task_init (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint16_t us_listen_port_ho = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_error = dimutex_node_task_init_nodes (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_task_init_nodes failed: %d", e_error);
      goto CLEAN_RETURN;
   }

   us_listen_port_ho = (px_dimutex_ctxt->x_init_params.us_listen_port_start_ho +
         px_dimutex_ctxt->x_init_params.ui_node_index);
   e_pal_ret = tcp_listen_sock_create (&(px_dimutex_ctxt->hl_listner_sock_hdl),
      us_listen_port_ho);
   if ((ePAL_RET_SUCCESS != e_pal_ret)
      || (NULL == px_dimutex_ctxt->hl_listner_sock_hdl))
   {
      DMUT_LOG_LOW("tcp_listen_sock_create failed: %d, %p",
         e_pal_ret, px_dimutex_ctxt->hl_listner_sock_hdl);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   e_error = dimutex_node_register_sock (px_dimutex_ctxt,
      px_dimutex_ctxt->hl_listner_sock_hdl);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_register_sock failed: %d", e_error);
      e_pal_ret = tcp_listen_sock_delete(px_dimutex_ctxt->hl_listner_sock_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("tcp_listen_sock_delete failed: %d", e_pal_ret);
      }
      px_dimutex_ctxt->hl_listner_sock_hdl = NULL;
      goto CLEAN_RETURN;
   }

   if (px_dimutex_ctxt->x_init_params.ui_node_index > 0)
   {
      e_error = dimutex_node_establish_conn_to_node (px_dimutex_ctxt, 0);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_connect_to_leader failed: %d", e_error);
      }
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_task_deinit (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   SOCKMON_REGISTER_DATA_X x_register_data = {NULL};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   (void) dimutex_node_cleanup_socks (px_dimutex_ctxt);

   x_register_data.hl_sock_hdl = px_dimutex_ctxt->hl_listner_sock_hdl;
   e_sockmon_ret = sockmon_deregister_sock (
      px_dimutex_ctxt->x_init_params.hl_sockmon_hdl, &x_register_data);
   if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
   {
      DMUT_LOG_LOW("sockmon_deregister_sock failed: %d", e_pal_ret);
   }

   if (NULL != px_dimutex_ctxt->hl_listner_sock_hdl)
   {
      e_pal_ret = tcp_listen_sock_delete (px_dimutex_ctxt->hl_listner_sock_hdl);
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         DMUT_LOG_LOW("tcp_listen_sock_delete failed: %d", e_pal_ret);
      }
      px_dimutex_ctxt->hl_listner_sock_hdl = NULL;
   }
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_post_msg_to_q (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint8_t *puc_data,
   uint32_t ui_data_len)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   MSGQ_DATA_X x_data = {NULL};

   if ((NULL == px_dimutex_ctxt) || (NULL == puc_data) || (0 == ui_data_len))
   {
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_data.p_data = puc_data;
   x_data.ui_data_size = ui_data_len;
   e_task_ret = task_add_msg_to_q(px_dimutex_ctxt->hl_listner_task_hdl,
      &x_data, DIMUTEX_TASK_Q_WAIT_TIMEOUT);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      DMUT_LOG_LOW("task_add_msg_to_q failed: %d", e_error);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }

CLEAN_RETURN:
   return e_error;
}

SOCKMON_RET_E dimutex_node_sockmon_active_sock_cbk (
   SOCKMON_SOCK_ACTIVITY_STATUS_E e_status,
   PAL_SOCK_HDL hl_sock_hdl,
   void *p_app_data)
{
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_CTXT_X *px_dimutex_ctxt = NULL;
   DIMUTEX_NODE_SOCK_ACT_DATA_X *px_sock_act_data = NULL;

   if ((NULL == hl_sock_hdl) || (NULL == p_app_data))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_sockmon_ret = eSOCKMON_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_dimutex_ctxt = (DIMUTEX_CTXT_X *) p_app_data;

   DMUT_LOG_LOW("dimutex_node_sockmon_active_sock_cbk called.");

   px_sock_act_data = pal_malloc (sizeof(DIMUTEX_NODE_SOCK_ACT_DATA_X),
      NULL );

   px_sock_act_data->x_hdr.ui_msg_id = eDIMUTEX_MSG_ID_LISTENER_SOCK_ACT;
   px_sock_act_data->x_hdr.ui_msg_pay_len = sizeof(*px_sock_act_data) -
         sizeof(px_sock_act_data->x_hdr);
   px_sock_act_data->hl_sock_hdl = hl_sock_hdl;
   e_error = dimutex_post_msg_to_q (
      px_dimutex_ctxt,
      (uint8_t *) px_sock_act_data,
      sizeof(*px_sock_act_data));
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_post_msg_to_q failed: %d", e_error);
      pal_free(px_sock_act_data);
      px_sock_act_data = NULL;
      e_sockmon_ret = eSOCKMON_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_sockmon_ret = eSOCKMON_RET_SUCCESS;
   }

CLEAN_RETURN:
   return e_sockmon_ret;
}
