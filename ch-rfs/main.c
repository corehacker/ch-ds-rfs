/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   main.c
 * \author sandeepprakash
 *
 * \date   24-Sep-2012
 *
 * \brief
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include <ch-pal/exp_pal.h>
#include <ch-sockmon/exp_sockmon.h>
#include "dimutex.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define APP_LEADER_HOST_NAME_STR_MAX_LEN           (512)

#define APP_MAX_MONITORED_SOCKETS                  (DIMUTEX_MAX_NODES + 1)

#define APP_TOTAL_MANDATORY_ARGUMENTS              (8)

#define APP_ARG_NO_OF_NODES_INDEX                  (1)
#define APP_ARG_NODE_INDEX_INDEX                   (2)
#define APP_ARG_ACT_AS_SERVER_INDEX                (3)
#define APP_ARG_LISTEN_PORT_START_INDEX            (4)
#define APP_ARG_LEADER_HOSTNAME_INDEX              (5)
#define APP_ARG_UNIT_TIME_INDEX                    (6)
#define APP_ARG_ENABLE_CONSOLE_LOGGING_INDEX       (7)
#define APP_ARG_APP_INTERNAL_PORT_INDEX            (8)

/******************************** ENUMERATIONS ********************************/
typedef enum _APP_RET_E
{
   eAPP_RET_FAILURE              = -1,

   eAPP_RET_SUCCESS              = 0x00000000
} APP_RET_E;

/************************* STRUCTURE/UNION DATA TYPES *************************/
typedef struct _APP_ARGS_X
{
   uint32_t    ui_node_index;

   uint32_t    ui_no_nodes;

   uint32_t    ui_act_as_server;

   uint32_t    ui_enable_console_logging;

   uint32_t    ui_unit_time_ms;

   uint16_t    us_listen_port_start_ho;

   uint16_t    us_app_internal_port_ho;

   uint8_t     uca_leader_hostname[APP_LEADER_HOST_NAME_STR_MAX_LEN];
} APP_ARGS_X;

typedef struct _APP_CTXT_X
{
   APP_ARGS_X  x_app_args;

   SOCKMON_HDL hl_sockmon_hdl;

   DIMUTEX_HDL hl_dimutex_hdl;
} APP_CTXT_X;

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
APP_RET_E app_env_init (
   APP_CTXT_X *px_app_ctxt)
{
   APP_RET_E e_main_ret = eAPP_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   DIMUTEX_RET_E e_dimutex_ret = eDIMUTEX_RET_FAILURE;
   SOCKMON_CREATE_PARAMS_X x_sockmon_params = {0};
   DIMUTEX_INIT_PARAMS_X x_dimutex_params = {0};

   if (NULL == px_app_ctxt)
   {
      goto CLEAN_RETURN;
   }

   x_sockmon_params.us_port_range_start =
      px_app_ctxt->x_app_args.us_app_internal_port_ho;
   x_sockmon_params.us_port_range_end =
      px_app_ctxt->x_app_args.us_app_internal_port_ho;
   x_sockmon_params.ui_max_monitored_socks = APP_MAX_MONITORED_SOCKETS;
   e_sockmon_ret = sockmon_create (&(px_app_ctxt->hl_sockmon_hdl),
      &x_sockmon_params);
   if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
   {
      goto CLEAN_RETURN;
   }

   x_dimutex_params.hl_sockmon_hdl = px_app_ctxt->hl_sockmon_hdl;
   x_dimutex_params.us_listen_port_start_ho =
         px_app_ctxt->x_app_args.us_listen_port_start_ho;
   x_dimutex_params.ui_no_nodes = px_app_ctxt->x_app_args.ui_no_nodes;
   x_dimutex_params.ui_node_index = px_app_ctxt->x_app_args.ui_node_index;
   (void) pal_strncpy (x_dimutex_params.uca_leader_host_name_str,
      px_app_ctxt->x_app_args.uca_leader_hostname,
      sizeof(x_dimutex_params.uca_leader_host_name_str));
   x_dimutex_params.ui_unit_time_ms = px_app_ctxt->x_app_args.ui_unit_time_ms;
   x_dimutex_params.b_act_as_server = (
         (1 == px_app_ctxt->x_app_args.ui_act_as_server) ? (true) : (false));
   e_dimutex_ret = dimutex_init (&(px_app_ctxt->hl_dimutex_hdl),
      &x_dimutex_params);
   if (eDIMUTEX_RET_SUCCESS != e_dimutex_ret)
   {
      e_sockmon_ret = sockmon_destroy (px_app_ctxt->hl_sockmon_hdl);
      if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
      {
      }
   }
   else
   {
      e_main_ret = eAPP_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_main_ret;
}

APP_RET_E app_env_deinit (
   APP_CTXT_X *px_app_ctxt)
{
   APP_RET_E e_main_ret = eAPP_RET_FAILURE;
   DIMUTEX_RET_E e_dimutex_ret = eDIMUTEX_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;

   if (NULL == px_app_ctxt)
   {
      goto CLEAN_RETURN;
   }

   if (NULL != px_app_ctxt->hl_dimutex_hdl)
   {
      e_dimutex_ret = dimutex_deinit (px_app_ctxt->hl_dimutex_hdl);
      if (eDIMUTEX_RET_SUCCESS != e_dimutex_ret)
      {

      }
      px_app_ctxt->hl_dimutex_hdl = NULL;
   }

   if (NULL != px_app_ctxt->hl_sockmon_hdl)
   {
      e_sockmon_ret = sockmon_destroy (px_app_ctxt->hl_sockmon_hdl);
      if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
      {

      }
      px_app_ctxt->hl_sockmon_hdl = NULL;
   }
   e_main_ret = eAPP_RET_SUCCESS;
CLEAN_RETURN:
   return e_main_ret;
}

void app_print_usage(
   int i_argc,
   char **ppuc_argv)
{

   printf ("Usage: \n\t%s "
      "<Total Nodes> "
      "<Node Index> "
      "<Act as Server> "
      "<Listen Port Range Start> "
      "<Leader Host Name> "
      "<Unit Time> "
      "<Console Logging> "
      "[<Port No For Internal Use>]"
      "\n\n", ppuc_argv[0]);

   printf ("Parameter Details: \n"
      "\t<Total Nodes, Range: 2 - 10> - Mandatory\n"
      "\t<Node Index, Range: 0 - 9; 0th index will mapped to leader.> - Mandatory\n"
      "\t<0 - Will act as a client, 1 - Will act as server> - Mandatory\n"
      "\t<Listen Port Range Start (> 15000 Preferred)> - Mandatory\n"
      "\t<Leader Host Name (Host Name of Node 0)> - Mandatory\n"
      "\t<Unit Time, in milliseconds. This will be used internally by "
      "algorithm for various waits conditions> - Mandatory\n"
      "\t<Console Logging, 1 - Enable, 0 - Disable> - Mandatory\n"
      "\t[<Port No For Internal Use (Defaults to 19000)>] - Optional. But "
      "required if bind fails for the internal port\n"
      "\n");

}

APP_RET_E app_parse_cmd_line(
   APP_CTXT_X *px_app_ctxt,
   int i_argc,
   char **ppuc_argv)
{
   APP_RET_E e_main_ret = eAPP_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   int32_t i_temp = 0;

   if ((NULL == px_app_ctxt) || (NULL == ppuc_argv) || (0 == i_argc))
   {
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   if (i_argc < APP_TOTAL_MANDATORY_ARGUMENTS)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   if ((NULL == ppuc_argv [APP_ARG_NO_OF_NODES_INDEX])
      || (NULL == ppuc_argv [APP_ARG_NODE_INDEX_INDEX])
      || (NULL == ppuc_argv [APP_ARG_ACT_AS_SERVER_INDEX])
      || (NULL == ppuc_argv [APP_ARG_LISTEN_PORT_START_INDEX])
      || (NULL == ppuc_argv [APP_ARG_LEADER_HOSTNAME_INDEX])
      || (NULL == ppuc_argv [APP_ARG_UNIT_TIME_INDEX])
      || (NULL == ppuc_argv [APP_ARG_ENABLE_CONSOLE_LOGGING_INDEX]))
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_atoi((uint8_t *) ppuc_argv[APP_ARG_NO_OF_NODES_INDEX],
      (int32_t *) &(px_app_ctxt->x_app_args.ui_no_nodes));
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_atoi((uint8_t *) ppuc_argv[APP_ARG_NODE_INDEX_INDEX],
      (int32_t *) &(px_app_ctxt->x_app_args.ui_node_index));
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_atoi((uint8_t *) ppuc_argv[APP_ARG_ACT_AS_SERVER_INDEX],
      (int32_t *) &(px_app_ctxt->x_app_args.ui_act_as_server));
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_atoi (
      (uint8_t *) ppuc_argv [APP_ARG_LISTEN_PORT_START_INDEX], &(i_temp));
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (i_temp > USHRT_MAX))
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }
   px_app_ctxt->x_app_args.us_listen_port_start_ho = (uint16_t) i_temp;

   (void) pal_strncpy (px_app_ctxt->x_app_args.uca_leader_hostname,
      (const uint8_t *) ppuc_argv [APP_ARG_LEADER_HOSTNAME_INDEX],
      sizeof(px_app_ctxt->x_app_args.uca_leader_hostname));

   e_pal_ret = pal_atoi ((uint8_t *) ppuc_argv [APP_ARG_UNIT_TIME_INDEX],
      (int32_t *) &(px_app_ctxt->x_app_args.ui_unit_time_ms));
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_atoi (
      (uint8_t *) ppuc_argv [APP_ARG_ENABLE_CONSOLE_LOGGING_INDEX],
      (int32_t *) &(px_app_ctxt->x_app_args.ui_enable_console_logging));
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      app_print_usage (i_argc, ppuc_argv);
      e_main_ret = eAPP_RET_FAILURE;
      goto CLEAN_RETURN;
   }

   if ((9 == i_argc) && (NULL != ppuc_argv[APP_ARG_APP_INTERNAL_PORT_INDEX]))
   {
      e_pal_ret = pal_atoi (
         (uint8_t *) ppuc_argv [APP_ARG_APP_INTERNAL_PORT_INDEX], &(i_temp));
      if ((ePAL_RET_SUCCESS != e_pal_ret) || (i_temp > USHRT_MAX))
      {
         app_print_usage (i_argc, ppuc_argv);
         e_main_ret = eAPP_RET_FAILURE;
         goto CLEAN_RETURN;
      }
      px_app_ctxt->x_app_args.us_app_internal_port_ho = (uint16_t) i_temp;
   }

   printf ("User Entered:\n"
      "\tTotal Nodes             : %d\n"
      "\tThis Node Index         : %d\n"
      "\tAct as Server           : %d\n"
      "\tListen Port Range Start : %d\n"
      "\tLeader Host Name        : %s\n"
      "\tUnit Time               : %d\n"
      "\tEnable Console Logging  : %d\n"
      "\tPort For Internal Use   : %d\n",
      px_app_ctxt->x_app_args.ui_no_nodes,
      px_app_ctxt->x_app_args.ui_node_index,
      px_app_ctxt->x_app_args.ui_act_as_server,
      px_app_ctxt->x_app_args.us_listen_port_start_ho,
      px_app_ctxt->x_app_args.uca_leader_hostname,
      px_app_ctxt->x_app_args.ui_unit_time_ms,
      px_app_ctxt->x_app_args.ui_enable_console_logging,
      px_app_ctxt->x_app_args.us_app_internal_port_ho);

   printf ("\n\nEnter 1 to Quit Application\n\n");

   e_main_ret = eAPP_RET_SUCCESS;
CLEAN_RETURN:
   return e_main_ret;
}

int main (int argc, char **argv)
{
   APP_RET_E e_main_ret = eAPP_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   APP_CTXT_X *px_app_ctxt = NULL;
   bool b_pal_init = false;
   int32_t i_command = -1;
   int32_t i_ret = -1;
   PAL_LOGGER_INIT_PARAMS_X x_logger_params = {false};

   e_pal_ret = pal_env_init();
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      goto CLEAN_RETURN;
   }
   b_pal_init = true;

   px_app_ctxt = pal_malloc(sizeof(APP_CTXT_X), NULL);
   if (NULL == px_app_ctxt)
   {
      goto CLEAN_RETURN;
   }

   e_main_ret = app_parse_cmd_line (px_app_ctxt, argc, argv);
   if (eAPP_RET_SUCCESS != e_main_ret)
   {
      goto CLEAN_RETURN;
   }

   x_logger_params.b_enable_console_logging =
         (0 == px_app_ctxt->x_app_args.ui_enable_console_logging) ?
               false : true;
   x_logger_params.b_enable_file_logging = true;
   (void) pal_strncpy (x_logger_params.uca_filename_prefix,
      (uint8_t *) "dimutex_log_node_",
      sizeof(x_logger_params.uca_filename_prefix));
   x_logger_params.ui_file_name_suffix = px_app_ctxt->x_app_args.ui_node_index;
   pal_logger_env_init(&x_logger_params);

   e_main_ret = app_env_init (px_app_ctxt);
   if (eAPP_RET_SUCCESS != e_main_ret)
   {
      goto CLEAN_RETURN;
   }

   while (1)
   {
      i_ret = scanf ("%d", &i_command); (void) i_ret;
      if (1 == i_command)
      {
         break;
      }
   }

   e_main_ret = app_env_deinit (px_app_ctxt);
   if (eAPP_RET_SUCCESS != e_main_ret)
   {
   }

   pal_logger_env_deinit ();
CLEAN_RETURN:
   if (NULL != px_app_ctxt)
   {
      pal_free(px_app_ctxt);
      px_app_ctxt = NULL;
   }
   if (true == b_pal_init)
   {
      e_pal_ret = pal_env_deinit();
      if (ePAL_RET_SUCCESS != e_pal_ret)
      {
         e_main_ret = eAPP_RET_FAILURE;
      }
      b_pal_init = false;
   }
   return (int) e_main_ret;
}
