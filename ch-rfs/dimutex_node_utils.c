/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_utils.c
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
#define DIMUTEX_NODE_READ_MSG_TIMEOUT_MS                 (5000)

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static DIMUTEX_RET_E dimutex_node_connect_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

static void dimutex_cs_entry_animate (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialize_data);

/****************************** LOCAL FUNCTIONS *******************************/
DIMUTEX_RET_E dimutex_node_establish_conn_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;

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

   e_error = dimutex_node_connect_to_node (px_dimutex_ctxt, ui_index);
   if (eDIMUTEX_RET_SUCCESS != e_error)
   {
      DMUT_LOG_LOW("dimutex_node_connect_to_node failed: %d", e_error);
   }
   else
   {
      e_error = dimutex_node_send_join_to_node (px_dimutex_ctxt, ui_index);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_send_credentials_to_node failed: %d",
            e_error);
      }
   }
CLEAN_RETURN:
   return e_error;
}

static DIMUTEX_RET_E dimutex_node_connect_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_SOCK_HDL *phl_node_sock_hdl = NULL;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   PAL_SOCK_ADDR_IN_X x_local_sock_addr = {0};
   PAL_SOCK_IN_ADDR_X x_in_addr = {0};
   DIMUTEX_NODE_CTXT_X *px_node = NULL;
   uint32_t ui_dns_name_len = 0;
   uint32_t ui_ip_addr_no = 0;

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

   px_node = &(px_dimutex_ctxt->x_nodes.xa_nodes[ui_index]);
   phl_node_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);

   if (0 == px_node->us_node_listen_port_ho)
   {
      DMUT_LOG_LOW("Invalid Args. Leader port: %d/Name: %s not set.",
         px_node->us_node_listen_port_ho, px_node->uca_node_dns_name_str);
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_sock_create(phl_node_sock_hdl, ePAL_SOCK_DOMAIN_AF_INET,
      ePAL_SOCK_TYPE_SOCK_STREAM, ePAL_SOCK_PROTOCOL_DEFAULT);
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (NULL == *phl_node_sock_hdl))
   {
      DMUT_LOG_LOW("pal_sock_create failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   ui_dns_name_len = pal_strnlen(px_node->uca_node_dns_name_str,
      sizeof(px_node->uca_node_dns_name_str));
   if (ui_dns_name_len > 0)
   {
      e_pal_ret = pal_gethostbyname (px_node->uca_node_dns_name_str,
         &x_in_addr);
      if ((ePAL_RET_SUCCESS != e_pal_ret) || (0 == x_in_addr.ui_ip_addr_no))
      {
         DMUT_LOG_LOW("pal_gethostbyname failed: %d", e_pal_ret);
         DMUT_LOG_LOW("Node Hostname : %s malformed/not reachable. "
         "Trying with IP address if set.", px_node->uca_node_dns_name_str);
         if (0 == px_node->ui_node_ip_addr_ho)
         {
            DMUT_LOG_LOW("Invalid Args. Node Ip: %d not set.",
               px_node->ui_node_ip_addr_ho);
            e_error = eDIMUTEX_RET_INVALID_ARGS;
            goto CLEAN_RETURN;
         }
         else
         {
            ui_ip_addr_no = pal_htonl (px_node->ui_node_ip_addr_ho);
            DMUT_LOG_LOW("Trying connection to host with IP: %d.",
               px_node->uca_node_dns_name_str, ui_ip_addr_no);
         }
      }
      else
      {
         ui_ip_addr_no = x_in_addr.ui_ip_addr_no;
         DMUT_LOG_LOW("Trying connection to \"%s\" with IP: %d.",
            px_node->uca_node_dns_name_str, ui_ip_addr_no);
      }
   }
   else
   {
      DMUT_LOG_LOW("Node Hostname : %s not set. Trying with IP address.",
         px_node->uca_node_dns_name_str);
      if (0 == px_node->ui_node_ip_addr_ho)
      {
         DMUT_LOG_LOW("Invalid Args. Node Ip: %d not set.",
            px_node->ui_node_ip_addr_ho);
         e_error = eDIMUTEX_RET_INVALID_ARGS;
         goto CLEAN_RETURN;
      }
      else
      {
         ui_ip_addr_no = pal_htonl (px_node->ui_node_ip_addr_ho);
         DMUT_LOG_LOW("Trying connection to host with IP: %d.",
            px_node->uca_node_dns_name_str, ui_ip_addr_no);
      }
   }
   x_local_sock_addr.us_sin_port_no = pal_htons (
      px_node->us_node_listen_port_ho);
   x_local_sock_addr.x_sin_addr.ui_ip_addr_no = ui_ip_addr_no;
   e_pal_ret = pal_sock_connect(*phl_node_sock_hdl, &x_local_sock_addr,
      ePAL_SOCK_CONN_MODE_BLOCKING, 0);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_connect to node %d failed: %d", ui_index,
         e_pal_ret);
      pal_sock_close(*phl_node_sock_hdl);
      *phl_node_sock_hdl = NULL;
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   else
   {
      DMUT_LOG_LOW("Connect to node %d success. Registering the socket.",
         ui_index);

      e_error = dimutex_node_register_sock (px_dimutex_ctxt,
         *phl_node_sock_hdl);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW("dimutex_node_register_sock failed: %d", e_error);
         e_pal_ret = pal_sock_close(*phl_node_sock_hdl);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_sock_close failed: %d", e_pal_ret);
         }
         *phl_node_sock_hdl = NULL;
      }
      else
      {
         e_error = eDIMUTEX_RET_SUCCESS;
      }
   }

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_register_sock (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL    hl_sock_hdl)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   SOCKMON_REGISTER_DATA_X x_register_data = {NULL};

   if ((NULL == px_dimutex_ctxt) || (NULL == hl_sock_hdl))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_register_data.hl_sock_hdl = hl_sock_hdl;
   x_register_data.fn_active_sock_cbk =
         dimutex_node_sockmon_active_sock_cbk;
   x_register_data.p_app_data = px_dimutex_ctxt;
   e_sockmon_ret = sockmon_register_sock (
      px_dimutex_ctxt->x_init_params.hl_sockmon_hdl,
      &x_register_data);
   if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
   {
      DMUT_LOG_LOW("sockmon_register_sock failed: %d", e_sockmon_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_MED("sockmon_register_sock success");
      e_error = eDIMUTEX_RET_SUCCESS;
   }

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_deregister_sock (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL    hl_sock_hdl)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   SOCKMON_REGISTER_DATA_X x_register_data = {NULL};

   if ((NULL == px_dimutex_ctxt) || (NULL == hl_sock_hdl))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_register_data.hl_sock_hdl = hl_sock_hdl;
   e_sockmon_ret = sockmon_deregister_sock (
      px_dimutex_ctxt->x_init_params.hl_sockmon_hdl, &x_register_data);
   if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
   {
      DMUT_LOG_LOW("sockmon_deregister_sock failed: %d", e_sockmon_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_MED("sockmon_register_sock success");
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_cleanup_socks (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   SOCKMON_REGISTER_DATA_X x_register_data = {NULL};
   PAL_RET_E e_pal_ret = ePAL_RET_SUCCESS;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (NULL != px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index])
      {
         DMUT_LOG_LOW("Deregistering socket of node %d", ui_index);
         x_register_data.hl_sock_hdl =
            px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index];
         e_sockmon_ret = sockmon_deregister_sock (
            px_dimutex_ctxt->x_init_params.hl_sockmon_hdl, &x_register_data);
         if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
         {
            DMUT_LOG_LOW("sockmon_deregister_sock failed: %d", e_sockmon_ret);
         }
         DMUT_LOG_LOW("Closing socket of node %d", ui_index);
         e_pal_ret = pal_sock_close (
            px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index]);
         if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            DMUT_LOG_LOW("pal_sock_close failed: %d", e_pal_ret);
         }
         px_dimutex_ctxt->x_nodes.hla_node_sock [ui_index] = NULL;
      }
   }
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_read_msg (
   PAL_SOCK_HDL hl_sock_hdl,
   uint8_t *puc_msg_buf,
   uint32_t ui_msg_buf_len,
   uint32_t *pui_msg_size)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_recv_len = 0;
   uint32_t ui_hdr_len = 0;
   NODE_MSG_HDR_X *px_msg_hdr = NULL;

   if ((NULL == hl_sock_hdl) || (NULL == puc_msg_buf) || (0 == ui_msg_buf_len)
      || (NULL == pui_msg_size) || (ui_msg_buf_len < sizeof(NODE_MSG_HDR_X)))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   /*
    * 1. Read the message header first.
    * 2. Get the payload size from the header.
    * 3. Read the payload size number of bytes from the socket.
    * 4. That will give the complete message.
    */
   ui_hdr_len = sizeof(NODE_MSG_HDR_X);
   ui_recv_len = ui_hdr_len;
   e_pal_ret = pal_sock_recv_fixed (hl_sock_hdl, puc_msg_buf, &ui_recv_len,
      0, DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (ui_recv_len != ui_hdr_len))
   {
      if (ePAL_RET_SOCK_CLOSED == e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed failed. Connection closed by peer.",
            DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
      }
      else if (ePAL_RET_OPERATION_TIMEDOUT == e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed timedout after %d ms: %d",
            DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
      }
      else
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed for header failed: 0x%x, "
            "Header len: %d, Received: %d", e_pal_ret, ui_hdr_len, ui_recv_len);
      }
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_HIGH("Read header of %d bytes.", ui_recv_len);

   px_msg_hdr = (NODE_MSG_HDR_X *) puc_msg_buf;

   ui_recv_len = px_msg_hdr->ui_msg_pay_len;
   e_pal_ret = pal_sock_recv_fixed (hl_sock_hdl, (puc_msg_buf + ui_hdr_len),
      &ui_recv_len, 0, DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
   if ((ePAL_RET_SUCCESS != e_pal_ret)
      || (ui_recv_len != px_msg_hdr->ui_msg_pay_len))
   {
      if (ePAL_RET_SOCK_CLOSED == e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed failed. Connection closed by peer.",
            DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
      }
      else if (ePAL_RET_OPERATION_TIMEDOUT == e_pal_ret)
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed timedout after %d ms: %d",
            DIMUTEX_NODE_READ_MSG_TIMEOUT_MS);
      }
      else
      {
         DMUT_LOG_LOW("pal_sock_recv_fixed for payload failed: %d, "
         "Header len: %d, Received: %d",
            e_pal_ret, px_msg_hdr->ui_msg_pay_len, ui_recv_len);
      }
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_HIGH("Read payload of %d bytes.", ui_recv_len);
      *pui_msg_size = ui_hdr_len + px_msg_hdr->ui_msg_pay_len;
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_send_msg (
   PAL_SOCK_HDL hl_sock_hdl,
   NODE_MSG_HDR_X *px_msg_hdr)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_send_len = 0;

   if ((NULL == hl_sock_hdl) || (NULL == px_msg_hdr))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   ui_send_len = sizeof(*px_msg_hdr) + px_msg_hdr->ui_msg_pay_len;
   e_pal_ret = pal_sock_send_fixed (hl_sock_hdl, (uint8_t *) px_msg_hdr,
      &ui_send_len, 0, DIMUTEX_SEND_MSG_TIMEOUT_MS);
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      DMUT_LOG_LOW("pal_sock_send_fixed failed: %d", e_pal_ret);
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      DMUT_LOG_LOW("pal_sock_send_fixed success: %d bytes", ui_send_len);
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}

void dimutex_node_log_status(
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
#if 0
   uint32_t ui_index = 0;
   DIMUTEX_NODE_CTXT_X *px_node_ctxt = NULL;
   PAL_SOCK_HDL *phl_sock_hdl = NULL;

   if (NULL != px_dimutex_ctxt)
   {
      printf ("\n\n**********************************************************"
         "**********************************************************");
      printf ("\n**********************************************************"
         "**********************************************************");
      printf ("\n%6s | %32s | %10s | %5s | %10s\n",
         "ID", "DNS Name", "IP Address", "Port", "Connection");
      printf ("__________________________________________________________"
         "__________________________________________________________\n");
      for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
      {
         px_node_ctxt = &(px_dimutex_ctxt->x_nodes.xa_nodes[ui_index]);
         phl_sock_hdl = &(px_dimutex_ctxt->x_nodes.hla_node_sock[ui_index]);
         if (0 == ui_index)
         {
            printf ("%6s", "Leader");
         }
         else if (px_dimutex_ctxt->x_nodes.ui_this_node_idx == ui_index)
         {
            printf ("%6s", "Myself");
         }
         else
         {
            printf ("%6s", "Other");
         }
         printf (" | %32s | 0x%8x | %5d | %10s\n",
            px_node_ctxt->uca_node_dns_name_str,
            px_node_ctxt->ui_node_ip_addr_ho,
            px_node_ctxt->us_node_listen_port_ho,
            ((NULL != *phl_sock_hdl) ? ("Connected") : ("No")));
      }
      printf ("\n**********************************************************"
         "**********************************************************");
      printf ("\n**********************************************************"
         "**********************************************************");
      printf ("\n\n");
   }
#endif
}

bool dimutex_check_all_nodes_have_joined (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   bool b_joined = false;
   uint32_t ui_index = 0;
   DIMUTEX_NODE_CTXT_X *px_node_ctxt = NULL;

   if (NULL != px_dimutex_ctxt)
   {
      for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
      {
         if (px_dimutex_ctxt->x_nodes.ui_this_node_idx == ui_index)
         {
            continue;
         }
         px_node_ctxt = &(px_dimutex_ctxt->x_nodes.xa_nodes [ui_index]);
         if (eDIMUTEX_NODE_STATE_JOINED != px_node_ctxt->e_state)
         {
            break;
         }
      }

      if (ui_index == px_dimutex_ctxt->x_init_params.ui_no_nodes)
      {
         b_joined = true;
      }
   }
   else
   {
      DMUT_LOG_LOW("Invalid Args");
   }
   return b_joined;
}

DIMUTEX_RET_E dimutex_node_get_active_sock_index (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint32_t *pui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == phl_act_sock_hdl)
      || (NULL == pui_index) || (NULL == *phl_act_sock_hdl))
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
      *pui_index = ui_index;
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

static void dimutex_cs_entry_animate (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialize_data)
{
   uint32_t ui_node_idx = 0;
   uint32_t ui_seq_no = 0;
   uint8_t uca_temp[256] = {0};
   uint32_t ui_index = 0;
   int i_ret_val = 0;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_serialize_data))
   {
      DMUT_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   i_ret_val = system ("clear"); (void) i_ret_val;
   fflush (stdout);
   (void) sscanf ((const char *) px_serialize_data->puc_serialization_data,
      "%d : %d : %s\n", &ui_node_idx, &ui_seq_no, uca_temp);

   for (ui_index = 0; (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes * 5);
         ui_index++)
   {
      printf ("%s", "*");fflush (stdout);
   }
   printf ("\n");fflush (stdout);
   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      printf (" %2d |", ui_index);fflush (stdout);
   }
   printf ("\n");fflush (stdout);
   for (ui_index = 0; (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes * 5);
         ui_index++)
   {
      printf ("%s", "-");fflush (stdout);
   }
   printf ("\n");fflush (stdout);
   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (ui_index == ui_node_idx)
      {
         printf (" %2s |", "*");fflush (stdout);
      }
      else
      {
         printf ("    |");fflush (stdout);
      }
   }
   printf ("\n");fflush (stdout);
   for (ui_index = 0; (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes * 5);
         ui_index++)
   {
      printf ("%s", "*");fflush (stdout);
   }
   printf ("\n");fflush (stdout);


CLEAN_RETURN:
   return;
}

DIMUTEX_RET_E dimutex_node_serialize_client_data (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialize_data)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   FILE *fp_server_file = NULL;
   uint8_t
      uca_server_filename [DIMUTEX_NODE_SERVER_SERIALIZE_FILENAME_STR_LEN] =
         {0};
   uint32_t ui_temp_len = 0;
   DIMUTEX_NODES_X *px_nodes = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_serialize_data))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   DMUT_LOG_LOW("Serialize: \"%s\"", px_serialize_data->puc_serialization_data);

   (void) dimutex_cs_entry_animate (px_dimutex_ctxt, px_serialize_data);

   ui_temp_len = pal_strnlen(px_serialize_data->uca_serialize_res_id_str,
      sizeof(px_serialize_data->uca_serialize_res_id_str));

   px_nodes = &(px_dimutex_ctxt->x_nodes);
   if (ui_temp_len > 0)
   {
      (void) snprintf ((char *) uca_server_filename, sizeof(uca_server_filename),
         "%s_%d_%s%s", px_serialize_data->uca_serialize_res_id_str,
         px_nodes->ui_this_node_idx,
         px_nodes->xa_nodes [px_nodes->ui_this_node_idx].uca_node_dns_name_str,
         DIMUTEX_NODE_SERVER_SERIALIZE_FILE_SUFFIX);
   }
   else
   {
      (void) snprintf ((char *) uca_server_filename, sizeof(uca_server_filename),
         "%d_%s%s", px_nodes->ui_this_node_idx,
         px_nodes->xa_nodes [px_nodes->ui_this_node_idx].uca_node_dns_name_str,
         DIMUTEX_NODE_SERVER_SERIALIZE_FILE_SUFFIX);
   }
   fp_server_file = fopen ((char *) uca_server_filename, "a+");
   if (NULL == fp_server_file)
   {
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   fprintf(fp_server_file, "%s", px_serialize_data->puc_serialization_data);

   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   if (NULL != fp_server_file)
   {
      fclose (fp_server_file);
      fp_server_file = NULL;
   }
   return e_error;
}

DIMUTEX_RET_E dimutex_node_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_ALGO_STATS_X *px_stats,
   uint32_t ui_index)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   DIMUTEX_NODE_CTXT_X *px_node_ctxt = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_stats))
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

   DMUT_LOG_LOW( "Got eNODE_MSG_ID_ALGO_COMPLETE from %d node", ui_index);

   (void) dimutex_node_aggregate_stats (px_dimutex_ctxt, ui_index, px_stats);

   px_node_ctxt = &(px_dimutex_ctxt->x_nodes.xa_nodes[ui_index]);
   DMUT_LOG_LOW("Setting node %d state to eDIMUTEX_NODE_STATE_ALGO_COMPLETE",
      ui_index);
   px_node_ctxt->e_state = eDIMUTEX_NODE_STATE_ALGO_COMPLETE;

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         continue;
      }
      if (eDIMUTEX_NODE_STATE_ALGO_COMPLETE
         != px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_state)
      {
         break;
      }
   }

   if (ui_index == px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      DMUT_LOG_LOW("All nodes have completed algorithm execution.");
      DMUT_LOG_LOW("Sending teardown to all nodes.");
      e_error = dimutex_node_send_algo_teardown_to_all (px_dimutex_ctxt);
      if (eDIMUTEX_RET_SUCCESS != e_error)
      {
         DMUT_LOG_LOW( "dimutex_node_send_algo_teardown_to_all failed: %d",
            e_error);
      }

      (void) dimutex_node_summarize_stats (px_dimutex_ctxt);
      (void) dimutex_node_check_server_file_consistency (px_dimutex_ctxt);
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}
