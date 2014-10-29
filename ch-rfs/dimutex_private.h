/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_private.h
 * \author sandeepprakash
 *
 * \date   24-Sep-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_PRIVATE_H__
#define __DIMUTEX_PRIVATE_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DIMUTEX_LEADER_NODE_INDEX                  (0)

#define DIMUTEX_TASK_MSGQ_SIZE                     (20)

#define DIMUTEX_TASK_PING_PEERS_INTERVAL_MS        (1200000)

#define DIMUTEX_SEND_MSG_TIMEOUT_MS                (9000)

#define DIMUTEX_RECV_MSG_TIMEOUT_MS                (9000)

#define NODE_MSG_MAX_MSG_LEN                       (64 * 1024)

#define DIMUTEX_ALGO_CS_MAX_REQUEST_COUNT          (40)

#define DIMUTEX_ALGO_CS_REQUEST_RATE_CHANGE_AFTER  (20)

#define DIMUTEX_ALGO_CS_EXECUTION_MAX_UNITS        (0)

#define NODE_SERIALIZE_RES_ID_STR_MAX_LEN          (256)

#define DMUT_LOG_STR                         "DMUT"

#define DMUT_LOG_LOW(format,...)                                              \
do                                                                            \
{                                                                             \
   LOG_LOW (DMUT_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,               \
      ##__VA_ARGS__);                                                         \
} while (0)

#define DMUT_LOG_MED(format,...)                                              \
do                                                                            \
{                                                                             \
   LOG_MED (DMUT_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,               \
      ##__VA_ARGS__);                                                         \
} while (0)

#define DMUT_LOG_HIGH(format,...)                                             \
do                                                                            \
{                                                                             \
   LOG_HIGH (DMUT_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,              \
      ##__VA_ARGS__);                                                         \
} while (0)

#define DMUT_LOG_FULL(format,...)                                             \
do                                                                            \
{                                                                             \
   LOG_FULL (DMUT_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,              \
      ##__VA_ARGS__);                                                         \
} while (0)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _DIMUTEX_ALGO_STATS_AGGREGATE_X
{
   DIMUTEX_ALGO_STATS_X       xa_algo_stats[DIMUTEX_MAX_NODES];

   uint32_t ui_odd_nodes_first_20_avg;

   uint32_t ui_odd_nodes_last_20_avg;

   uint32_t ui_even_nodes_first_20_avg;

   uint32_t ui_even_nodes_last_20_avg;

   uint32_t ui_all_nodes_first_20_avg;

   uint32_t ui_all_nodes_last_20_avg;
} DIMUTEX_ALGO_STATS_AGGREGATE_X;

typedef struct _DIMUTEX_ALGO_RESOURCES_X
{
   TASK_HDL                hl_resource_task_hdl;

   TASK_HDL                hl_messaging_task_hdl;

   PAL_MUTEX_HDL           hl_algo_mutex_hdl;

   PAL_SEM_HDL             hl_algo_sem_hdl;

   PAL_SEM_HDL             hl_algo_ser_hdl;

   uint32_t                ui_next_entry_in_ms;

   bool                    b_send_complete;
} DIMUTEX_ALGO_RESOURCES_X;

typedef struct _DIMUTEX_ALGO_DB_X
{
   uint32_t                ui_our_seq_no;

   uint32_t                ui_highest_seq_no;

   bool                    ba_a[DIMUTEX_MAX_NODES];

   bool                    ba_reply_referred[DIMUTEX_MAX_NODES];

   bool                    ba_awaiting_ser_rsp[DIMUTEX_MAX_NODES];

   bool                    b_using;

   bool                    b_waiting;
} DIMUTEX_ALGO_DB_X;

typedef struct _DIMUTEX_ALGO_CTXT_X
{
   DIMUTEX_ALGO_RESOURCES_X   x_resources;

   DIMUTEX_ALGO_DB_X          x_db;

   DIMUTEX_ALGO_STATS_X       x_algo_stats;
} DIMUTEX_ALGO_CTXT_X;

typedef struct _DIMUTEX_CTXT_X
{
   DIMUTEX_INIT_PARAMS_X            x_init_params;

   TASK_HDL                         hl_listner_task_hdl;

   PAL_SOCK_HDL                     hl_listner_sock_hdl;

   DIMUTEX_NODES_X                  x_nodes;

   PAL_SOCK_HDL                     hla_temp_node_sock[DIMUTEX_MAX_NODES];

   uint8_t                          uca_temp_sock_buf[NODE_MSG_MAX_MSG_LEN];

   uint32_t                         ui_last_ping_time_ms;

   DIMUTEX_ALGO_CTXT_X              x_algo;

   DIMUTEX_ALGO_STATS_AGGREGATE_X   x_stats;
} DIMUTEX_CTXT_X;

typedef struct _DIMUTEX_NODE_SERIALIZE_DATA_X
{
   uint8_t uca_serialize_res_id_str[NODE_SERIALIZE_RES_ID_STR_MAX_LEN];

   uint32_t ui_serialization_data_len;

   uint8_t *puc_serialization_data;
} DIMUTEX_NODE_SERIALIZE_DATA_X;

/***************************** FUNCTION PROTOTYPES ****************************/

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_PRIVATE_H__ */
