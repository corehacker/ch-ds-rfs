/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_send_msg.h
 * \author sandeepprakash
 *
 * \date   07-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_SEND_MSG_H__
#define __DIMUTEX_SEND_MSG_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_send_join_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_join_done_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_establish_peers_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_establish_done_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

DIMUTEX_RET_E dimutex_node_send_ping_to_all (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_send_ping_to_node (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_SEND_MSG_H__ */
