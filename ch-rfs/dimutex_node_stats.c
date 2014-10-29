/*******************************************************************************
 * Copyright (c) 2012, Sandeep Prakash <123sandy@gmail.com>
 * All rights reserved.
 *
 * \file   dimutex_node_stats.c
 * \author sandeepprakash
 *
 * \date   29-Oct-2012
 *
 * \brief  
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include "dimutex_env.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
DIMUTEX_RET_E dimutex_node_aggregate_stats (
   DIMUTEX_CTXT_X *px_dimutex_ctxt,
   uint32_t ui_node_idx,
   DIMUTEX_ALGO_STATS_X *px_stats)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;
   uint32_t ui_first_20_total = 0;
   uint32_t ui_last_20_total = 0;
   uint32_t ui_req_grant_diff = 0;
   DIMUTEX_ALGO_STATS_X *px_stats_temp = NULL;

   if ((NULL == px_dimutex_ctxt) || (NULL == px_stats)
      || (ui_node_idx >= px_dimutex_ctxt->x_init_params.ui_no_nodes))
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_stats_temp = &(px_dimutex_ctxt->x_stats.xa_algo_stats [ui_node_idx]);

   (void) pal_memmove (px_stats_temp, px_stats, sizeof(*px_stats_temp));

   for (ui_index = 0; ui_index < px_stats->ui_count; ui_index++)
   {
      px_stat_entry = &(px_stats->xa_cs_entry_stat [ui_index]);
      printf ("[STATS]Node %d: CS Entry: Current Count: %d, Req @: %d ms, "
         "Granted @: %d ms, Req Sent: %d, Reply Recvd: %d\n", ui_node_idx,
         (ui_index + 1), px_stat_entry->ui_cs_req_time_ms,
         px_stat_entry->ui_cs_grant_time_ms,
         px_stat_entry->ui_no_request_msgs_sent,
         px_stat_entry->ui_no_reply_msgs_received);
      DMUT_LOG_LOW("[STATS]Node %d: CS Entry: Current Count: %d, Req @: %d ms, "
      "Granted @: %d ms, Req Sent: %d, Reply Recvd: %d",
         ui_node_idx, (ui_index + 1), px_stat_entry->ui_cs_req_time_ms,
         px_stat_entry->ui_cs_grant_time_ms,
         px_stat_entry->ui_no_request_msgs_sent,
         px_stat_entry->ui_no_reply_msgs_received);

      if (0 == px_stats_temp->ui_min_msgs)
      {
         px_stats_temp->ui_min_msgs = (px_stat_entry->ui_no_request_msgs_sent
            + px_stat_entry->ui_no_reply_msgs_received);
      }

      if ((px_stat_entry->ui_no_request_msgs_sent
         + px_stat_entry->ui_no_reply_msgs_received)
         < px_stats_temp->ui_min_msgs)
      {
         px_stats_temp->ui_min_msgs = (px_stat_entry->ui_no_request_msgs_sent
            + px_stat_entry->ui_no_reply_msgs_received);
      }

      if ((px_stat_entry->ui_no_request_msgs_sent
         + px_stat_entry->ui_no_reply_msgs_received)
         > px_stats_temp->ui_max_msgs)
      {
         px_stats_temp->ui_max_msgs = (px_stat_entry->ui_no_request_msgs_sent
            + px_stat_entry->ui_no_reply_msgs_received);
      }

      ui_req_grant_diff = px_stat_entry->ui_cs_grant_time_ms
         - px_stat_entry->ui_cs_req_time_ms;

      if (ui_index < DIMUTEX_ALGO_CS_REQUEST_RATE_CHANGE_AFTER)
      {
         ui_first_20_total += ui_req_grant_diff;
      }
      else
      {
         ui_last_20_total += ui_req_grant_diff;
      }
   }

   px_stats_temp->ui_first_20_avg = ui_first_20_total
      / DIMUTEX_ALGO_CS_REQUEST_RATE_CHANGE_AFTER;
   px_stats_temp->ui_last_20_avg = ui_last_20_total
      / (px_stats_temp->ui_count - DIMUTEX_ALGO_CS_REQUEST_RATE_CHANGE_AFTER);
   px_stats_temp->ui_full_avg = (ui_first_20_total + ui_last_20_total)
      / px_stats_temp->ui_count;
   e_error = eDIMUTEX_RET_SUCCESS;
CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_summarize_stats (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint32_t ui_index = 0;
   uint32_t ui_stats_index = 0;
   DIMUTEX_ALGO_STATS_X *px_node_stats = NULL;
   DIMUTEX_ALGO_CS_ENTRY_STAT_X *px_stat_entry = NULL;
   FILE *fp_stats_summary = NULL;

   uint32_t ui_odd_first_20_avg = 0;
   uint32_t ui_odd_last_20_avg = 0;
   uint32_t ui_odd_full_avg = 0;

   uint32_t ui_even_first_20_avg = 0;
   uint32_t ui_even_last_20_avg = 0;
   uint32_t ui_even_full_avg = 0;

   uint32_t ui_all_first_20_avg = 0;
   uint32_t ui_all_last_20_avg = 0;
   uint32_t ui_all_full_avg = 0;

   uint32_t ui_odd_first_20_total = 0;
   uint32_t ui_odd_last_20_total = 0;
   uint32_t ui_odd_full_total = 0;

   uint32_t ui_even_first_20_total = 0;
   uint32_t ui_even_last_20_total = 0;
   uint32_t ui_even_full_total = 0;

   uint32_t ui_all_first_20_total = 0;
   uint32_t ui_all_last_20_total = 0;
   uint32_t ui_all_full_total = 0;

   uint32_t ui_total_msgs_count = 0;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   fp_stats_summary = fopen ("dimutex_stats_summary.txt", "w+");
   if (NULL == fp_stats_summary)
   {
      DMUT_LOG_LOW("fopen failed");
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   printf ("Node:Request No:Requested:Granted:Elapsed:Sent:Received\n");
   fprintf (fp_stats_summary,
      "Node:Request No:Requested:Granted:Elapsed:Sent:Received\n");

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      px_node_stats = &(px_dimutex_ctxt->x_stats.xa_algo_stats [ui_index]);

      for (ui_stats_index = 0; ui_stats_index < px_node_stats->ui_count; ui_stats_index++)
      {
         px_stat_entry = &(px_node_stats->xa_cs_entry_stat [ui_stats_index]);
         printf ("%d:%d:%d:%d:%d:%d:%d\n", ui_index, ui_stats_index + 1,
            px_stat_entry->ui_cs_req_time_ms,
            px_stat_entry->ui_cs_grant_time_ms,
            (px_stat_entry->ui_cs_grant_time_ms
               - px_stat_entry->ui_cs_req_time_ms),
            px_stat_entry->ui_no_request_msgs_sent,
            px_stat_entry->ui_no_reply_msgs_received);

         fprintf (fp_stats_summary, "%d:%d:%d:%d:%d:%d:%d\n", ui_index,
            ui_stats_index + 1, px_stat_entry->ui_cs_req_time_ms,
            px_stat_entry->ui_cs_grant_time_ms,
            (px_stat_entry->ui_cs_grant_time_ms
               - px_stat_entry->ui_cs_req_time_ms),
            px_stat_entry->ui_no_request_msgs_sent,
            px_stat_entry->ui_no_reply_msgs_received);

         ui_total_msgs_count += px_stat_entry->ui_no_request_msgs_sent;
         ui_total_msgs_count += px_stat_entry->ui_no_request_msgs_sent;
      }
   }

   printf ("------------------------------------------------------------------"
      "------------------\n");
   printf ("| %5s | %12s | %12s | %12s | %12s | %12s |\n", "Node",
      "First 20 Avg", "Last 20 Avg", "Overall Avg", "Min Messages",
      "Max Messages");
   printf ("|-%5s-+-%12s-+-%12s-+-%12s-+-%12s-+-%12s-+\n", "-----",
      "------------", "------------", "------------", "------------",
      "------------");

   fprintf (fp_stats_summary,
      "------------------------------------------------------------------"
         "------------------\n");
   fprintf (fp_stats_summary, "| %5s | %12s | %12s | %12s | %12s | %12s |\n",
      "Node", "First 20 Avg", "Last 20 Avg", "Overall Avg", "Min Messages",
      "Max Messages");
   fprintf (fp_stats_summary, "|-%5s-+-%12s-+-%12s-+-%12s-+-%12s-+-%12s-+\n",
      "-----", "------------", "------------", "------------", "------------",
      "------------");
   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
         ui_index++)
   {
      px_node_stats = &(px_dimutex_ctxt->x_stats.xa_algo_stats [ui_index]);

      printf ("| %5d | %12d | %12d | %12d | %12d | %12d |\n", ui_index,
         px_node_stats->ui_first_20_avg, px_node_stats->ui_last_20_avg,
         px_node_stats->ui_full_avg, px_node_stats->ui_min_msgs,
         px_node_stats->ui_max_msgs);

      fprintf (fp_stats_summary, "| %5d | %12d | %12d | %12d | %12d | %12d |\n",
         ui_index, px_node_stats->ui_first_20_avg,
         px_node_stats->ui_last_20_avg, px_node_stats->ui_full_avg,
         px_node_stats->ui_min_msgs, px_node_stats->ui_max_msgs);

      if (0 == (ui_index % 2))
      {
         ui_even_first_20_total += px_node_stats->ui_first_20_avg;
         ui_even_last_20_total += px_node_stats->ui_last_20_avg;
         ui_even_full_total += (px_node_stats->ui_first_20_avg
            + px_node_stats->ui_last_20_avg);
      }
      else
      {
         ui_odd_first_20_total += px_node_stats->ui_first_20_avg;
         ui_odd_last_20_total += px_node_stats->ui_last_20_avg;
         ui_odd_full_total += (px_node_stats->ui_first_20_avg
            + px_node_stats->ui_last_20_avg);
      }

      ui_all_first_20_total += px_node_stats->ui_first_20_avg;
      ui_all_last_20_total += px_node_stats->ui_last_20_avg;
      ui_all_full_total += (px_node_stats->ui_first_20_avg
         + px_node_stats->ui_last_20_avg);
   }
   printf ("------------------------------------------------------------------"
      "------------------\n");
   fprintf (fp_stats_summary,
      "------------------------------------------------------------------"
         "------------------\n");

   ui_even_first_20_avg = ui_even_first_20_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2);
   ui_even_last_20_avg = ui_even_last_20_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2);
   ui_even_full_avg = (ui_even_full_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2) / 2);

   ui_odd_first_20_avg = ui_odd_first_20_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2);
   ui_odd_last_20_avg = ui_odd_last_20_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2);
   ui_odd_full_avg = (ui_odd_full_total
      / (px_dimutex_ctxt->x_init_params.ui_no_nodes / 2) / 2);

   ui_all_first_20_avg = (ui_all_first_20_total
      / px_dimutex_ctxt->x_init_params.ui_no_nodes);
   ui_all_last_20_avg = (ui_all_last_20_total
      / px_dimutex_ctxt->x_init_params.ui_no_nodes);
   ui_all_full_avg = ((ui_all_full_total
      / px_dimutex_ctxt->x_init_params.ui_no_nodes) / 2);

   printf ("\n\n\n");
   printf ("************************************************************\n");
   printf ("***************Dimutex Run Statistics Summary***************\n");
   printf (
      "\nGeneral Information:\n"
      "\tConfigured Unit Time          : %d\n"
      "\tOdd Numbered Nodes First 20   : [10, 20] * %d ms\n"
      "\tOdd Numbered Nodes Last 20    : [10, 20] * %d ms\n"
      "\tEven Numbered Nodes First 20  : [10, 20] * %d ms\n"
      "\tEven Numbered Nodes Last 20   : [40, 50] * %d ms\n",
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms);
   printf ("\nEven Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n",
      ui_even_first_20_avg, ui_even_last_20_avg, ui_even_full_avg);
   printf ("\nOdd Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n",
      ui_odd_first_20_avg, ui_odd_last_20_avg, ui_odd_full_avg);
   printf ("\nAll Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n"
      "\tTotal Message Count           : %d\n",
      ui_all_first_20_avg, ui_all_last_20_avg, ui_all_full_avg,
      ui_total_msgs_count);
   printf ("************************************************************\n");

   fprintf (fp_stats_summary,
      "\n\n\n");
   fprintf (fp_stats_summary,
      "************************************************************\n");
   fprintf (fp_stats_summary,
      "***************Dimutex Run Statistics Summary***************\n");
   fprintf (fp_stats_summary,
      "\nGeneral Information:\n"
      "\tConfigured Unit Time          : %d\n"
      "\tOdd Numbered Nodes First 20   : [10, 20] * %d ms\n"
      "\tOdd Numbered Nodes Last 20    : [10, 20] * %d ms\n"
      "\tEven Numbered Nodes First 20  : [10, 20] * %d ms\n"
      "\tEven Numbered Nodes Last 20   : [40, 50] * %d ms\n",
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms,
      px_dimutex_ctxt->x_init_params.ui_unit_time_ms);
   fprintf (fp_stats_summary,
      "\nEven Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n",
      ui_even_first_20_avg, ui_even_last_20_avg, ui_even_full_avg);
   fprintf (fp_stats_summary,
      "\nOdd Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n",
      ui_odd_first_20_avg, ui_odd_last_20_avg, ui_odd_full_avg);
   fprintf (fp_stats_summary,
      "\nAll Nodes:\n"
      "\tFirst 20 Entries Average      : %d ms\n"
      "\tLast 20 Entries Average       : %d ms\n"
      "\tOverall Average               : %d ms\n"
      "\tTotal Message Count           : %d\n",
      ui_all_first_20_avg, ui_all_last_20_avg, ui_all_full_avg,
      ui_total_msgs_count);
   fprintf (fp_stats_summary,
      "************************************************************\n");

   fflush (fp_stats_summary);
   fclose (fp_stats_summary);
   fp_stats_summary = NULL;

CLEAN_RETURN:
   return e_error;
}

DIMUTEX_RET_E dimutex_node_check_server_file_consistency (
   DIMUTEX_CTXT_X *px_dimutex_ctxt)
{
   DIMUTEX_RET_E e_error = eDIMUTEX_RET_FAILURE;
   uint8_t uca_server_filename1 [DIMUTEX_NODE_SERVER_SERIALIZE_FILENAME_STR_LEN] =
      { 0 };
   uint8_t uca_server_filename2 [DIMUTEX_NODE_SERVER_SERIALIZE_FILENAME_STR_LEN] =
      { 0 };
   uint8_t uca_sys_cmd [1024] = { 0 };
   uint32_t ui_index = 0;
   uint32_t ui_index1 = 0;
   int i_sys_ret = -1;

   if (NULL == px_dimutex_ctxt)
   {
      DMUT_LOG_LOW("Invalid Args");
      e_error = eDIMUTEX_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
   {
      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         (void) snprintf ((char *) uca_server_filename1,
            sizeof(uca_server_filename1), "%s_%d_%s%s",
            DIMUTEX_SERIALIZATION_RES_ID_STR, ui_index,
            px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].uca_node_dns_name_str,
            DIMUTEX_NODE_SERVER_SERIALIZE_FILE_SUFFIX);
         ui_index1 = ui_index;
         break;
      }
   }

   for (ui_index = 0; ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes;
            ui_index++)
   {
      if (ui_index == ui_index1)
      {
         continue;
      }
      if (eDIMUTEX_NODE_TYPE_SERVER
         == px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].e_node_type)
      {
         (void) pal_memset (uca_server_filename2, 0x00,
            sizeof(uca_server_filename2));
         (void) pal_memset (uca_sys_cmd, 0x00, sizeof(uca_sys_cmd));
         (void) snprintf ((char *) uca_server_filename2,
            sizeof(uca_server_filename2), "%s_%d_%s%s",
            DIMUTEX_SERIALIZATION_RES_ID_STR, ui_index,
            px_dimutex_ctxt->x_nodes.xa_nodes [ui_index].uca_node_dns_name_str,
            DIMUTEX_NODE_SERVER_SERIALIZE_FILE_SUFFIX);

         (void) snprintf ((char *) uca_sys_cmd, sizeof(uca_sys_cmd),
            "diff %s %s", uca_server_filename1, uca_server_filename2);

         printf ("Executing Command: \"%s\"\n", uca_sys_cmd);
         DMUT_LOG_LOW ("Executing Command: \"%s\"", uca_sys_cmd);
         i_sys_ret = system ((const char *) uca_sys_cmd);
         if (0 != i_sys_ret)
         {
            DMUT_LOG_LOW("Data file of server %d (%s) is NOT CONSISTENT"
            " with file of server %d (%s)",
               ui_index1, uca_server_filename1, ui_index, uca_server_filename2);
            printf ("Data file of server %d (%s) is NOT CONSISTENT"
               " with file of server %d (%s)\n\n", ui_index1,
               uca_server_filename1, ui_index, uca_server_filename2);
            break;
         }
         else
         {
            DMUT_LOG_LOW("Data file of server %d (%s) is CONSISTENT"
            " with file of server %d (%s)",
               ui_index1, uca_server_filename1, ui_index, uca_server_filename2);
            printf ("Data file of server %d (%s) is CONSISTENT"
               " with file of server %d (%s)\n\n", ui_index1,
               uca_server_filename1, ui_index, uca_server_filename2);
         }
      }
   }
   if (ui_index < px_dimutex_ctxt->x_init_params.ui_no_nodes)
   {
      e_error = eDIMUTEX_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_error = eDIMUTEX_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_error;
}
