/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex.h
 * \author sandeepprakash
 *
 * \date   24-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_H__
#define __DIMUTEX_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_LEADER_HOST_NAME_STR_MAX_LEN       (512)

#define DIMUTEX_MAX_NODES                          (50)

/******************************** ENUMERATIONS ********************************/
typedef enum _DIMUTEX_RET_E
{
   eDIMUTEX_RET_SUCCESS                   = 0x00000000,

   eDIMUTEX_RET_FAILURE,

   eDIMUTEX_RET_INVALID_ARGS,

   eDIMUTEX_RET_INVALID_HANDLE,

   eDIMUTEX_RET_RESOURCE_FAILURE,

   eDIMUTEX_RET_INVALID_MESSAGE,

   eDIMUTEX_RET_MAX
} DIMUTEX_RET_E;

typedef enum _DIMUTEX_MSG_ID_E
{
   eDIMUTEX_MSG_ID_INVALID                      = 0x00000000,

   eDIMUTEX_MSG_ID_LISTENER_START               = 0x00001001,

   eDIMUTEX_MSG_ID_LISTENER_SOCK_ACT            = 0x00001002,
   /*!< #_DIMUTEX_LISTENER_SOCK_ACT_DATA_X */

   eDIMUTEX_MSG_ID_ALGO_COMPLETE                = 0x00001003,
   /*!< #_DIMUTEX_ALGO_COMPLETE_X. */

   eDIMUTEX_MSG_ID_LISTENER_END                 = 0x00002000,

   eDIMUTEX_MSG_ID_MAX,
} DIMUTEX_MSG_ID_E;

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct DIMUTEX_CTXT_X    *DIMUTEX_HDL;

typedef struct _DIMUTEX_INIT_PARAMS_X
{
   uint32_t    ui_node_index;

   uint32_t    ui_no_nodes;

   uint32_t    ui_unit_time_ms;

   SOCKMON_HDL hl_sockmon_hdl;

   uint8_t     uca_leader_host_name_str[DIMUTEX_LEADER_HOST_NAME_STR_MAX_LEN];

   uint16_t    us_listen_port_start_ho;

   bool        b_act_as_server;
} DIMUTEX_INIT_PARAMS_X;

typedef struct _DIMUTEX_MSG_HDR_X
{
   uint32_t ui_msg_id;

   uint32_t ui_msg_pay_len;

   uint8_t uca_reserved[56];
} DIMUTEX_MSG_HDR_X;

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_init (
   DIMUTEX_HDL *phl_dimutex_hdl,
   DIMUTEX_INIT_PARAMS_X *px_init_params);

DIMUTEX_RET_E dimutex_deinit (
   DIMUTEX_HDL hl_dimutex_hdl);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_H__ */
