/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_utils.h
 * \author sandeepprakash
 *
 * \date   29-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_NODE_UTILS_H__
#define __DIMUTEX_NODE_UTILS_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_NODE_SERVER_SERIALIZE_FILENAME_STR_LEN   (512)

#define DIMUTEX_NODE_SERVER_SERIALIZE_FILE_SUFFIX        ".txt"

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_establish_conn_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_register_sock (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL    hl_sock_hdl);

DIMUTEX_RET_E dimutex_node_deregister_sock (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL    hl_sock_hdl);

DIMUTEX_RET_E dimutex_node_cleanup_socks (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_read_msg (
   PAL_SOCK_HDL hl_sock_hdl,
   uint8_t *puc_msg_buf,
   uint32_t ui_msg_buf_len,
   uint32_t *pui_msg_size);

DIMUTEX_RET_E dimutex_node_send_msg (
   PAL_SOCK_HDL hl_sock_hdl,
   NODE_MSG_HDR_X *px_msg_hdr);

void dimutex_node_log_status(
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_serialize_client_data (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialize_data);

DIMUTEX_RET_E dimutex_node_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_ALGO_STATS_X *px_stats,
   uint32_t ui_index);

bool dimutex_check_all_nodes_have_joined (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_get_active_sock_index (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   uint32_t *pui_index);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_NODE_UTILS_H__ */
