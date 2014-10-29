/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node.h
 * \author sandeepprakash
 *
 * \date   28-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_NODE_H__
#define __DIMUTEX_NODE_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_NODE_DNS_NAME_STR_MAX_LEN          (512)

#define DIMUTEX_LEADER_NODE_INDEX                  (0)

/******************************** ENUMERATIONS ********************************/
typedef enum _DIMUTEX_NODE_STATE_E
{
   eDIMUTEX_NODE_STATE_INVALID                  = 0x00000000,

   eDIMUTEX_NODE_STATE_JOINED,

   eDIMUTEX_NODE_STATE_ALGO_READY,

   eDIMUTEX_NODE_STATE_ALGO_COMPLETE,

   eDIMUTEX_NODE_STATE_MAX,
} DIMUTEX_NODE_STATE_E;

typedef enum _DIMUTEX_NODE_TYPE_E
{
   eDIMUTEX_NODE_TYPE_INVALID               = 0x00000000,

   eDIMUTEX_NODE_TYPE_CLIENT,

   eDIMUTEX_NODE_TYPE_SERVER,

   eDIMUTEX_NODE_TYPE_MAX
} DIMUTEX_NODE_TYPE_E;

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _DIMUTEX_NODE_CTXT_X
{
   uint32_t             ui_node_index;

   uint32_t             ui_node_ip_addr_ho;

   DIMUTEX_NODE_STATE_E e_state;

   uint8_t              uca_node_dns_name_str[DIMUTEX_NODE_DNS_NAME_STR_MAX_LEN];

   uint16_t             us_node_listen_port_ho;

   uint8_t              uca_padding[2];

   DIMUTEX_NODE_TYPE_E  e_node_type;

   uint8_t              uca_reserved[60];
} DIMUTEX_NODE_CTXT_X;

typedef struct _DIMUTEX_NODES_X
{
   DIMUTEX_NODE_CTXT_X     xa_nodes[DIMUTEX_MAX_NODES];

   PAL_SOCK_HDL            hla_node_sock[DIMUTEX_MAX_NODES];

   uint32_t                ui_no_nodes;

   uint32_t                ui_this_node_idx;
} DIMUTEX_NODES_X;

/***************************** FUNCTION PROTOTYPES ****************************/

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_NODE_H__ */
