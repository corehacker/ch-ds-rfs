/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_send_server_msg.h
 * \author sandeepprakash
 *
 * \date   28-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_SEND_SERVER_MSG_H__
#define __DIMUTEX_SEND_SERVER_MSG_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_send_serialize_data_to_all_servers (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialization_data);

DIMUTEX_RET_E dimutex_node_send_serialize_data_req_to_server (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index,
   DIMUTEX_NODE_SERIALIZE_DATA_X *px_serialization_data);

DIMUTEX_RET_E dimutex_node_send_serialize_data_rsp_to_client (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_index);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_SEND_SERVER_MSG_H__ */
