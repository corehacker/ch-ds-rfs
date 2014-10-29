/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_send_server_msg.c
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
DIMUTEX_RET_E dimutex_node_send_serialize_data_to_all_servers (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialization_data)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_DB_X *px_db = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_serialization_data)
      || (NULL == px_serialization_data->puc_serialization_data)
      || (0 == px_serialization_data->ui_serialization_data_len))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_db = &(px_dimutex_ctxt->x_algo.x_db);

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (ui_index == px_dimutex_ctxt->x_nodes.ui_this_node_idx)
      {
         e_error = eDIMUTEX_RET_SUCCESS;
         continue;
      }

      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         e_error = dimutex_node_send_serialize_data_req_to_server (
            px_dimutex_ctxt, ui_index, px_serialization_data);
         if (eDIMUTEX_RET_SUCCESS != e_error)
         {
            DMUT_LOG_HIGH("dimutex_node_send_serialize_data_to_server to node "
               "%d failed: %d", ui_index, e_error);
            break;
         }
         else
         {
            px_db->ba_awaiting_ser_rsp[ui_index] = true;
         }
      }
   }

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_serialize_data_req_to_server (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialization_data)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   NODE_SERVER_SERIALIZE_DATA_REQ_X *px_serialize = NULL;
   uint32_t ui_alloc_size = 0;
   uint8_t *puc_temp = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_serialization_data)
      || (NULL == px_serialization_data->puc_serialization_data)
      || (0 == px_serialization_data->ui_serialization_data_len))
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

   ui_alloc_size = sizeof(NODE_SERVER_SERIALIZE_DATA_REQ_X);
   ui_alloc_size += px_serialization_data->ui_serialization_data_len;
   px_serialize = pal_malloc (ui_alloc_size, NULL);
   if (NULL == px_serialize)
   {
      DMUT_LOG_LOW("pal_malloc failed");
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   px_serialize->x_hdr.ui_msg_id = eNODE_MSG_ID_SERVER_SERIALIZE_DATA_REQ;
   px_serialize->x_hdr.ui_msg_pay_len = ui_alloc_size - sizeof(NODE_MSG_HDR_X);

   (void) pal_strncpy(px_serialize->uca_serialize_res_id_str,
      px_serialization_data->uca_serialize_res_id_str,
      sizeof(px_serialize->uca_serialize_res_id_str));
   px_serialize->ui_serialize_data_len =
      px_serialization_data->ui_serialization_data_len;
   puc_temp = (uint8_t *) px_serialize;
   puc_temp += sizeof(NODE_SERVER_SERIALIZE_DATA_REQ_X);
   (void) pal_memmove (puc_temp, px_serialization_data->puc_serialization_data,
      px_serialization_data->ui_serialization_data_len);
   e_error = dimutex_node_send_msg (*phl_node_sock_hdl, &(px_serialize->x_hdr));
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_send_msg to node %d failed: %d",
         ui_index, e_error);
   }
CLEAN_RETURN:
   if (NULL != px_serialize)
   {
      pal_free (px_serialize);
      px_serialize = NULL;
   }
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_serialize_data_rsp_to_client (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   NODE_SERVER_SERIALIZE_DATA_RSP_X x_rsp = {{0}};
   uint32_t ui_send_size = 0;

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

   DMUT_LOG_LOW("Sending eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP to node %d",
      ui_index);

   x_rsp.x_hdr.ui_msg_id = eNODE_MSG_ID_SERVER_SERIALIZE_DATA_RSP;
   x_rsp.x_hdr.ui_msg_pay_len = sizeof(x_rsp) - sizeof(x_rsp.x_hdr);

   ui_send_size = sizeof(x_rsp);
   e_pal_ret = pal_sock_send_fixed (*phl_node_sock_hdl,
      (uint8_t *) &x_rsp, &ui_send_size, 0, DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d failed: %d",
         ui_index, e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed to node %d success: %d bytes", ui_index,
         ui_send_size);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}
