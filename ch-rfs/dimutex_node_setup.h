/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_setup.h
 * \author sandeepprakash
 *
 * \date   29-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_NODE_SETUP_H__
#define __DIMUTEX_NODE_SETUP_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_handle_setup_join (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_setup_join_done (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_setup_establish_peers (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_setup_establish_done (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_setup_ping (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_setup_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_teardown_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_stop (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_forward_msg (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_server_serialize_data_req (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

DIMUTEX_RET_E dimutex_node_handle_server_serialize_data_rsp (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   PAL_SOCK_HDL *phl_act_sock_hdl,
   NODE_MSG_HDR_X *px_msg_header);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_NODE_SETUP_H__ */
