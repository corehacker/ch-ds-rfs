/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_setup.c
 * \author sandeepprakash
 *
 * \date   29-Sep-2012
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
DIMUTEX_RET_E dimutex_node_handle_setup_join (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   NODE_MSG_JOIN_X *px_join_cred = NULL;
   DIMUTEX_NODE_CTXT_X *px_join_node = NULL;
   PAL_SOCK_HDL *phl_join_sock_hdl = NULL;
   uint32_t ui_join_index = 0;
   bool b_joined = false;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_join_cred = (NODE_MSG_JOIN_X *) px_msg_header;

   ui_join_index = px_join_cred->x_node_ctxt.ui_node_index;

   px_join_node =
      &(px_dimutex_ctxt->x_nodes.xa_nodes [ui_join_index]);
   (void) pal_memmove(px_join_node, &(px_join_cred->x_node_ctxt),
      sizeof(*px_join_node));
   px_join_node->e_state = eDIMUTEX_NODE_STATE_JOINED;
   px_dimutex_ctxt->x_nodes.ui_no_nodes++;
   phl_join_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_join_index]);
   *phl_join_sock_hdl = *phl_act_sock_hdl;
   *phl_act_sock_hdl = NULL;

   DMUT_LOG_LOW("New Node Joined The System");
   DMUT_LOG_LOW("Name           : %s", px_join_cred->x_node_ctxt.uca_node_dns_name_str);
   DMUT_LOG_LOW("Index          : %d", px_join_cred->x_node_ctxt.ui_node_index);
   DMUT_LOG_LOW("Port           : %d", px_join_cred->x_node_ctxt.us_node_listen_port_ho);
   DMUT_LOG_LOW("Client/Server? : %s",
      ((eDIMUTEX_NODE_TYPE_SERVER == px_join_cred->x_node_ctxt.e_node_type) ?
         ("Server") : ("Client")));

   e_error = dimutex_node_send_join_done_to_node (px_dimutex_ctxt,
      ui_join_index);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_send_join_done_to_node to node %d failed: %d "
      "to node %d", ui_join_index, e_error);
      goto CLEAN_RETURN;
   }

   if (DIMUTEX_LEADER_NODE_INDEX != px_dimutex_ctxt->x_nodes.ui_this_node_idx)
   {
      /*
       * No need to wait for all the nodes to JOIN in the case of non-leaders.
       * All other nodes will establish their connections later by a separate
       * message eNODE_MSG_ID_SETUP_ESTABLISH_PEERS.
       */
      e_error = eDIMUTEX_RET_SUCCESS;
      goto CLEAN_RETURN;
   }

   b_joined = dimutex_check_all_nodes_have_joined (px_dimutex_ctxt);
   if (true == b_joined)
   {
      DMUT_LOG_LOW("Got required number of nodes: %d, Required: %d",
         px_dimutex_ctxt->x_nodes.ui_no_nodes,
         px_dimutex_ctxt->x_init_params.ui_no_nodes);
      /*
       * TODO: Send establish peer message to client.
       */
      DMUT_LOG_LOW("Sending eNODE_MSG_ID_SETUP_ESTABLISH_PEERS to %d node",
         (DIMUTEX_LEADER_NODE_INDEX + 1));
      e_error = dimutex_node_send_establish_peers_to_node (px_dimutex_ctxt,
         (DIMUTEX_LEADER_NODE_INDEX + 1));
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_establish_peers_to_node failed: %d "
         "to node %d", e_error, (DIMUTEX_LEADER_NODE_INDEX + 1));
      }
   }
   else
   {
      DMUT_LOG_LOW("Waiting for required number of nodes: %d, Required: %d",
         px_dimutex_ctxt->x_nodes.ui_no_nodes,
         px_dimutex_ctxt->x_init_params.ui_no_nodes);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_setup_join_done (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_NODE_CTXT_X *px_join_node = NULL;
   bool b_joined = false;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_error = dimutex_node_get_active_sock_index (px_dimutex_ctxt,
      phl_act_sock_hdl, &ui_index);
   if ((eDIMUTEX_RET_SUCCESS == e_error)
      && (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes))
   {
      px_join_node = &(px_dimutex_ctxt->x_nodes.xa_nodes [ui_index]);

      DMUT_LOG_LOW("Got eNODE_MSG_ID_SETUP_JOIN_DONE from node %d", ui_index);
      DMUT_LOG_LOW("Number of nodes successfully joined: %d, Required: %d",
         px_dimutex_ctxt->x_nodes.ui_no_nodes,
         px_dimutex_ctxt->x_init_params.ui_no_nodes);
      px_join_node->e_state = eDIMUTEX_NODE_STATE_JOINED;

      if (DIMUTEX_LEADER_NODE_INDEX == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
      {
         DMUT_LOG_LOW("Not waiting for all eNODE_MSG_ID_SETUP_JOIN_DONE because"
            " I am leader.");
         e_error = eDIMUTEX_RET_SUCCESS;
         goto CLEAN_RETURN;
      }

      b_joined = dimutex_check_all_nodes_have_joined (px_dimutex_ctxt);
      if (true == b_joined)
      {
         e_error = dimutex_node_send_establish_done_to_node (px_dimutex_ctxt,
            DIMUTEX_LEADER_NODE_INDEX);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_send_establish_done_to_node failed: %d",
               e_error);
         }
      }
      else
      {
         e_error = eDIMUTEX_RET_SUCCESS;
      }
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_setup_establish_peers (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   NODE_MSG_ESTABLISH_PEERS_X *px_est_peers = NULL;
   uint32_t ui_index = 0;
   DIMUTEX_NODE_CTXT_X *px_node = NULL;
   bool b_joined = false;
   uint32_t ui_exp_size = 0;
   NODE_MSG_PEER_DATA_X *px_peer_data = NULL;
   uint8_t *puc_peer_offset = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_est_peers = (NODE_MSG_ESTABLISH_PEERS_X *) px_msg_header;

   if (px_est_peers->x_peers.ui_count
      != px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid peer count received. Expected %d, Got %d",
         px_dimutex_ctxt->x_init_params.ui_no_nodes,
         px_est_peers->x_peers.ui_count);
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   ui_exp_size += sizeof(NODE_MSG_ESTABLISH_PEERS_X);
   ui_exp_size +=
      (px_est_peers->x_peers.ui_count * sizeof(NODE_MSG_PEER_DATA_X));
   ui_exp_size = ui_exp_size - sizeof(px_est_peers->x_hdr);

   if (ui_exp_size != px_est_peers->x_hdr.ui_msg_pay_len)
   {
      DMUT_LOG_LOW("Malformed message. Expected payload size: %d bytes, "
         "Got %d bytes.", ui_exp_size, px_est_peers->x_hdr.ui_msg_pay_len);
      e_error = eDIMUTEX_RET_INVALID_MESSAGE;
      goto CLEAN_RETURN;
   }

   puc_peer_offset = (uint8_t *) &(px_est_peers->x_peers.ui_count);
   puc_peer_offset += sizeof(px_est_peers->x_peers.ui_count);
   for (ui_index = 0; ui_index < px_est_peers->x_peers.ui_count; ui_index++)
   {
      if (0 == ui_index)
      {
         DMUT_LOG_LOW("Connection exists for node: %d (leader)", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;

         px_node = &(px_dimutex_ctxt->x_nodes.xa_nodes[ui_index]);
         px_peer_data = (NODE_MSG_PEER_DATA_X *) puc_peer_offset;
         px_node->e_node_type = px_peer_data->e_node_type;

         puc_peer_offset += sizeof(NODE_MSG_PEER_DATA_X);
         continue;
      }

      if (ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
      {
         DMUT_LOG_LOW("Connection exists for node: %d (myself)", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         puc_peer_offset += sizeof(NODE_MSG_PEER_DATA_X);
         continue;
      }

      if (NULL != px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index])
      {
         if (eDIMUTEX_NODE_STATE_JOINED
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state)
         {
            DMUT_LOG_LOW("Connection exists for node: %d. But not joined.",
               ui_index);
         }
         DMUT_LOG_LOW("Connection exists for node: %d", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         puc_peer_offset += sizeof(NODE_MSG_PEER_DATA_X);
         continue;
      }

      px_node = &(px_dimutex_ctxt->x_nodes.xa_nodes[ui_index]);
      px_peer_data = (NODE_MSG_PEER_DATA_X *) puc_peer_offset;

      px_node->ui_node_ip_addr_ho = px_peer_data->ui_ip_addr_ho;
      px_node->us_node_listen_port_ho = px_peer_data->us_port_no_ho;
      px_node->e_node_type = px_peer_data->e_node_type;
      pal_strncpy (px_node->uca_node_dns_name_str,
         px_peer_data->uca_host_name_str,
         sizeof(px_node->uca_node_dns_name_str));

      puc_peer_offset += sizeof(NODE_MSG_PEER_DATA_X);

      e_error = dimutex_node_establish_conn_to_node (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_register_sock failed: %d", e_error);
         break;
      }
   }

   b_joined = dimutex_check_all_nodes_have_joined (px_dimutex_ctxt);
   if (true == b_joined)
   {
      e_error = dimutex_node_send_establish_done_to_node (px_dimutex_ctxt,
         DIMUTEX_LEADER_NODE_INDEX);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_establish_done_to_node failed: %d",
            e_error);
      }
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_setup_establish_done (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index] == *phl_act_sock_hdl)
      {
         break;
      }
   }

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW(
         "Got eNODE_MSG_ID_SETUP_ESTABLISH_DONE response from %d node", ui_index);

      if ((ui_index + 1) < px_dimutex_ctxt->x_init_params.ui_no_nodes)
      {
         DMUT_LOG_LOW("Sending establish peer to %d node", ui_index + 1);
         e_error = dimutex_node_send_establish_peers_to_node (px_dimutex_ctxt,
            (ui_index + 1));
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_send_establish_peers_to_node failed: %d "
            "to node %d", e_error, (ui_index + 1));
         }
      }
      else
      {
         DMUT_LOG_LOW("All nodes are done with establishing peer connections");
         DMUT_LOG_LOW("Sending Algo Setup to all.");
         e_error = dimutex_node_send_algo_setup_to_all (px_dimutex_ctxt);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_LOW("dimutex_node_send_algo_setup_to_all failed: %d", e_error);
         }
      }
   }
   else
   {
      DMUT_LOG_LOW("*********************************************************");
      DMUT_LOG_LOW("*************Fatal Error: Socket IDs Mismatch************");
      DMUT_LOG_LOW("*********************************************************");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_setup_ping (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   NODE_MSG_PING_X *px_ping_msg = NULL;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_ping_msg = (NODE_MSG_PING_X *) px_msg_header;

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index] == *phl_act_sock_hdl)
      {
         break;
      }
   }

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Got ping from %d node. Ping initiated @ %d ms at the node.",
         ui_index, px_ping_msg->ui_ping_local_ts_ms);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   /*
    * 1. Create algorithm thread.
    * 2. Send a eNODE_MSG_ID_ALGO_SETUP_COMPLETE to the leader.
    */
   e_error = dimutex_algo_setup (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_algo_setup failed: %d", e_error);
   }
   e_error = dimutex_node_send_algo_setup_complete (px_dimutex_ctxt, 0);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_node_send_algo_setup_complete failed: %d", e_error);
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_setup_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   uint32_t ui_count = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index] == *phl_act_sock_hdl)
      {
         break;
      }
   }

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW(
         "Got eNODE_MSG_ID_ALGO_SETUP_COMPLETE response from %d node",
         ui_index);
      DMUT_LOG_LOW( "Setting node %d state to eDIMUTEX_NODE_STATE_ALGO_READY",
         ui_index);
      px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state =
         eDIMUTEX_NODE_STATE_ALGO_READY;

      for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
               ui_index++)
      {
         if ((eDIMUTEX_NODE_TYPE_CLIENT
            == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
            && (px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state
               != eDIMUTEX_NODE_STATE_ALGO_READY))
         {
            DMUT_LOG_LOW("Waiting for node %d eDIMUTEX_NODE_STATE_ALGO_READY",
               ui_index);
         }
         else
         {
            ui_count++;
         }
      }

      if (ui_count == px_dimutex_ctxt->x_init_params.ui_no_nodes)
      {
         DMUT_LOG_LOW("Status of all nodes is eDIMUTEX_NODE_STATE_ALGO_READY");
         DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_START to all nodes.");
         e_error = dimutex_node_send_algo_start_to_all (px_dimutex_ctxt);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_HIGH("dimutex_node_send_algo_start_to_all failed: %d",
               e_error);
         }
      }
      else
      {
         e_error = eDIMUTEX_RET_SUCCESS;
      }
   }
   else
   {
      DMUT_LOG_LOW("*********************************************************");
      DMUT_LOG_LOW("*************Fatal Error: Socket IDs Mismatch************");
      DMUT_LOG_LOW("*********************************************************");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_error = dimutex_algo_teardown (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_algo_teardown failed: %d", e_error);
   }
   e_error = dimutex_node_send_algo_teardown_complete (px_dimutex_ctxt, 0);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_node_send_algo_teardown_complete failed: %d",
         e_error);
   }

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_teardown_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index] == *phl_act_sock_hdl)
      {
         break;
      }
   }

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW(
         "Got eNODE_MSG_ID_ALGO_TEARDOWN_COMPLETE response from %d node",
         ui_index);
      DMUT_LOG_LOW( "Setting node %d state to eDIMUTEX_NODE_STATE_JOINED",
         ui_index);
      px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state =
         eDIMUTEX_NODE_STATE_JOINED;
      e_error = eDIMUTEX_RET_SUCCESS;
   }
   else
   {
      DMUT_LOG_LOW("*********************************************************");
      DMUT_LOG_LOW("*************Fatal Error: Socket IDs Mismatch************");
      DMUT_LOG_LOW("*********************************************************");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_error = dimutex_algo_start (px_dimutex_ctxt);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_HIGH("dimutex_algo_start failed: %d", e_error);
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_stop (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   /*
    * State update to the algo thread to IDLE.
    */

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   NODE_MSG_ALGO_COMPLETE_X *px_algo_complete = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index] == *phl_act_sock_hdl)
      {
         break;
      }
   }

   px_algo_complete = (NODE_MSG_ALGO_COMPLETE_X *) px_msg_header;

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      e_error = dimutex_node_algo_complete (px_dimutex_ctxt,
         &(px_algo_complete->x_stats), ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_algo_complete failed: %d", e_error);
      }
   }
   else
   {
      DMUT_LOG_LOW("*********************************************************");
      DMUT_LOG_LOW("*************Fatal Error: Socket IDs Mismatch************");
      DMUT_LOG_LOW("*********************************************************");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_forward_msg (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   MSGQ_DATA_X x_data = {NULL};
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_MSG_DATA_X *px_msg_data = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index]
         == *phl_act_sock_hdl)
      {
         break;
      }
   }

   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {

   }
   else
   {
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_msg_data = pal_malloc (sizeof(DIMUTEX_ALGO_MSG_DATA_X), NULL);
   if (NULL == px_msg_data)
   {
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   px_msg_data->ui_src_node_index = ui_index;
   px_msg_data->ui_data_len =
      (sizeof(NODE_MSG_HDR_X) + px_msg_header->ui_msg_pay_len);
   px_msg_data->puc_data = pal_malloc (px_msg_data->ui_data_len, NULL );
   if (NULL == px_msg_data->puc_data)
   {
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   (void) pal_memmove (px_msg_data->puc_data, px_msg_header,
      px_msg_data->ui_data_len);

   x_data.p_data = px_msg_data;
   x_data.ui_data_size = sizeof(DIMUTEX_ALGO_MSG_DATA_X);
   e_task_ret = task_add_msg_to_q (
      px_dimutex_ctxt->x_algo.x_resources.hl_messaging_task_hdl, &x_data,
      DIMUTEX_TASK_Q_WAIT_TIMEOUT);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      DMUT_LOG_HIGH("task_add_msg_to_q failed: %d",
         e_task_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_server_serialize_data_req (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   NODE_SERVER_SERIALIZE_DATA_REQ_X *px_serialize = NULL;
   DIMUTEX_NODE_SERIALIZE_DATA_X x_serialization_data = {{0}};
   uint8_t *puc_temp = NULL;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (0 == px_msg_header->ui_msg_pay_len)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_serialize = (NODE_SERVER_SERIALIZE_DATA_REQ_X *) px_msg_header;

   if (0 == px_serialize->ui_serialize_data_len)
   {
      DMUT_LOG_LOW("No Data to Serialize");
      e_error = eDIMUTEX_RET_SUCCESS;
   }
   else
   {
      puc_temp = (uint8_t *) px_serialize;
      puc_temp += sizeof(NODE_SERVER_SERIALIZE_DATA_REQ_X);

      x_serialization_data.puc_serialization_data = puc_temp;
      x_serialization_data.ui_serialization_data_len =
         px_serialize->ui_serialize_data_len;
      (void) pal_strncpy (x_serialization_data.uca_serialize_res_id_str,
         px_serialize->uca_serialize_res_id_str,
         sizeof(x_serialization_data.uca_serialize_res_id_str));
      e_error = dimutex_node_serialize_client_data (px_dimutex_ctxt,
         &x_serialization_data);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_serialize_client_data failed: %d",
            e_error);
         goto CLEAN_RETURN;
      }

      e_error = dimutex_node_get_active_sock_index (px_dimutex_ctxt,
         phl_act_sock_hdl, &ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_get_active_sock_index failed: %d", e_error);
         goto CLEAN_RETURN;
      }

      e_error = dimutex_node_send_serialize_data_rsp_to_client (px_dimutex_ctxt,
         ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW(
            "dimutex_node_send_serialize_data_rsp_to_client failed: %d",
            e_error);
      }
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_handle_server_serialize_data_rsp (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == px_msg_header))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

CLEAN_RETURN:
   return e_error;
}
