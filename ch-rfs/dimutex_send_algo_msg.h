/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_send_algo_msg.h
 * \author sandeepprakash
 *
 * \date   28-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_SEND_ALGO_MSG_H__
#define __DIMUTEX_SEND_ALGO_MSG_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_send_algo_setup_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_send_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_setup_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_start_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_send_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_request (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_reply (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_teardown_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_send_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_algo_teardown_complete (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_SEND_ALGO_MSG_H__ */
