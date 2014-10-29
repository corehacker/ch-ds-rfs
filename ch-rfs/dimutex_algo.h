/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_algo.h
 * \author sandeepprakash
 *
 * \date   06-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __DIMUTEX_ALGO_H__
#define __DIMUTEX_ALGO_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
DIMUTEX_RET_E dimutex_algo_setup (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_algo_teardown (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

DIMUTEX_RET_E dimutex_algo_start (
   DIMUTEX_CTXT_X *px_dimutex_ctxt);

#ifdef   __cplusplus
}
#endif

#endif /* __DIMUTEX_ALGO_H__ */
