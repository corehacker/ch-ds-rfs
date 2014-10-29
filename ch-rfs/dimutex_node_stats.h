/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_stats.h
 * \author sandeepprakash
 *
 * \date   29-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_NODE_STATS_H__
#define __DIMUTEX_NODE_STATS_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_node_aggregate_stats (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_node_idx,
   DIMUTEX_ALGO_STATS_X *px_stats);

DIMUTEX_RET_E dimutex_node_summarize_stats (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_node_check_server_file_consistency (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_NODE_STATS_H__ */
