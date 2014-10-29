/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_send_algo_msg.c
 * \author sandeepprakash
 *
 * \date   28-Oct-2012
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
DIMUTEX_RET_E dimutex_node_send_algo_setup_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if ((ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
         && (eDIMUTEX_NODE_TYPE_SERVER
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type))
      {
         DMUT_LOG_LOW("Will not send algo setup message to node: %d (myself)",
            ui_index);
         DMUT_LOG_LOW("Will initiate algo setup directly");

         e_error = dimutex_algo_setup (px_dimutex_ctxt);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_HIGH("dimutex_algo_setup to node %d failed: %d",
               ui_index, e_error);
            break;
         }
         else
         {
            px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state =
               eDIMUTEX_NODE_STATE_ALGO_READY;
            continue;
         }
      }

      if ((NULL == px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index])
         || (eDIMUTEX_NODE_STATE_JOINED
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state))
      {
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         DMUT_LOG_LOW("Will not send algo setup message to node: %d "
            "because it is a server", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      e_error = dimutex_node_send_algo_setup (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_algo_setup to node %d failed: %d",
            ui_index, e_error);
         break;
      }
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_SETUP_X x_algo_setup = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_SETUP to node %d", ui_index);
   x_algo_setup.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_SETUP;
   x_algo_setup.x_hdr.ui_msg_pay_len = sizeof(x_algo_setup)
      - sizeof(x_algo_setup.x_hdr);

   ui_send_size = sizeof(x_algo_setup);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_setup, &ui_send_size, 0, DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_setup_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_SETUP_COMPLETE_X x_algo_setup_comp = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_SETUP_COMPLETE to node %d",
      ui_index);
   x_algo_setup_comp.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_SETUP_COMPLETE;
   x_algo_setup_comp.x_hdr.ui_msg_pay_len = sizeof(x_algo_setup_comp)
      - sizeof(x_algo_setup_comp.x_hdr);

   ui_send_size = sizeof(x_algo_setup_comp);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_setup_comp, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_start_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if ((ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
         && (eDIMUTEX_NODE_TYPE_SERVER
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type))
      {
         DMUT_LOG_LOW("Will not send algo start message to node: %d (myself)",
            ui_index);
         DMUT_LOG_LOW("Will initiate algo start directly");

         e_error = dimutex_algo_start (px_dimutex_ctxt);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_HIGH("dimutex_algo_start to node %d failed: %d",
               ui_index, e_error);
            break;
         }
         else
         {
            continue;
         }
      }

      if ((NULL == px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index])
         || (eDIMUTEX_NODE_STATE_ALGO_READY
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state))
      {
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         DMUT_LOG_LOW("Will not send algo start message to node: %d "
            "because it is a server", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      e_error = dimutex_node_send_algo_start (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_HIGH("dimutex_node_send_algo_start to node %d failed: %d",
            ui_index, e_error);
         break;
      }
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_START_X x_algo_start = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_START to node %d", ui_index);
   x_algo_start.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_START;
   x_algo_start.x_hdr.ui_msg_pay_len = sizeof(x_algo_start)
      - sizeof(x_algo_start.x_hdr);

   ui_send_size = sizeof(x_algo_start);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_start, &ui_send_size, 0, DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_COMPLETE_X x_algo_complete = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_COMPLETE to node %d", ui_index);
   x_algo_complete.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_COMPLETE;
   x_algo_complete.x_hdr.ui_msg_pay_len = sizeof(x_algo_complete)
      - sizeof(x_algo_complete.x_hdr);

   (void) pal_memmove(&(x_algo_complete.x_stats),
      &(px_dimutex_ctxt->x_algo.x_algo_stats), sizeof(x_algo_complete.x_stats));

   ui_send_size = sizeof(x_algo_complete);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_complete, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_request (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_REQUEST_X x_algo_request = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);


   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_REQUEST to node %d", ui_index);
   x_algo_request.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_REQUEST;
   x_algo_request.x_hdr.ui_msg_pay_len = sizeof(x_algo_request)
      - sizeof(x_algo_request.x_hdr);

   x_algo_request.ui_seq_number = px_dimutex_ctxt->x_algo.x_db.ui_our_seq_no;

   ui_send_size = sizeof(x_algo_request);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_request, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_reply (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_ALGO_MSG_REPLY_X x_algo_reply = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_REPLY to node %d", ui_index);
   x_algo_reply.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_REPLY;
   x_algo_reply.x_hdr.ui_msg_pay_len = sizeof(x_algo_reply)
      - sizeof(x_algo_reply.x_hdr);

   ui_send_size = sizeof(x_algo_reply);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_reply, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_teardown_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if ((ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
         && (eDIMUTEX_NODE_TYPE_SERVER
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type))
      {
         DMUT_LOG_LOW(
            "Will not send algo teardown message to node: %d (myself)",
            ui_index);
         DMUT_LOG_LOW("Will initiate algo teardown directly");

         e_error = dimutex_algo_teardown (px_dimutex_ctxt);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_HIGH("dimutex_algo_teardown to node %d failed: %d",
               ui_index, e_error);
         }
         else
         {
            px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state =
               eDIMUTEX_NODE_STATE_JOINED;
            continue;
         }
      }

      if ((NULL == px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index])
         || (eDIMUTEX_NODE_STATE_ALGO_COMPLETE
            != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state))
      {
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         DMUT_LOG_LOW("Will not send algo teardown message to node: %d "
         "because it is a server", ui_index);
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      e_error = dimutex_node_send_algo_teardown (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_HIGH("dimutex_node_send_algo_teardown to node %d failed: %d",
            ui_index, e_error);
      }
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_TEARDOWN_X x_algo_teardown = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_TEARDOWN to node %d", ui_index);
   x_algo_teardown.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_TEARDOWN;
   x_algo_teardown.x_hdr.ui_msg_pay_len = sizeof(x_algo_teardown)
      - sizeof(x_algo_teardown.x_hdr);

   ui_send_size = sizeof(x_algo_teardown);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_teardown, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_algo_teardown_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   uint32_t ui_send_size = 0;
   NODE_MSG_ALGO_TEARDOWN_COMPLETE_X x_algo_tear_comp = {{0}};

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (ui_index > px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_ALGO_TEARDOWN_COMPLETE to node %d",
      ui_index);
   x_algo_tear_comp.x_hdr.ui_msg_id = eNODE_MSG_ID_ALGO_TEARDOWN_COMPLETE;
   x_algo_tear_comp.x_hdr.ui_msg_pay_len = sizeof(x_algo_tear_comp)
      - sizeof(x_algo_tear_comp.x_hdr);

   ui_send_size = sizeof(x_algo_tear_comp);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_algo_tear_comp, &ui_send_size, 0,
      DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes",
         ui_index, ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}
