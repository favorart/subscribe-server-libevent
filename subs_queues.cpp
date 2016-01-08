#include "stdafx.h"
#include "subs_error.h"
#include "subs_server.h"
#include "subs_queues.h"


void  QueuesManager::broadcast (struct server_config *config)
{
  if ( !news_ ) return;

  for ( size_t i = 0U; i < n_; ++i )
  {
    while ( !queues_[i].empty () )
    {
      std::string message = queues_[i].front ();
      for ( client *Client : config->clients )
      {
        struct evbuffer *buf_out = bufferevent_get_output (Client->b_ev);
        if ( Client->queues_subscribed->find (i) != Client->queues_subscribed->end () )
          evbuffer_add_printf (buf_out, "<que %u>  %s%s", i, message.c_str (), (message[message.size () - 1] == '\n') ? "" : "\n" );
      }
      queues_[i].pop ();
    }
  }
  news_ = false;
}
//-----------------------------------------
/*  ѕротокол:  publish <num> "message" || subscribe <num>
 *  ќтвет:     ok <num> || error <err_text>
 *  
 *  ≈сть сервер, у него есть N очередей сообщений.
 *  ≈сть сервер-клиент, который может положить сообщение на сервер в определЄнную очередь
 *  или подписатьс€ на прослушивание очереди.
 */
void  subscribe_handling (client *Client, struct evbuffer *buf_in,
                                          struct evbuffer *buf_out)
{
  const char * const  err_prefix = "subscribe handling";
  const char * const  cmd_sub = "subscribe";
  const char * const  cmd_pub = "publish";

  const char *err_answer = "";
  int   no_answer = 0;
  //-----------------------------------------------------------------
  char   *cmd = NULL;
  size_t  cmd_length = 0U;
  
  while ( (cmd = evbuffer_readln (buf_in, &cmd_length, EVBUFFER_EOL_ANY)) )
  {
    char*   cmd_cur_char = cmd;
    size_t  n = 0U;
    //-----------------------------------------------------------------
    size_t  queue_no = 0U;
    
    char cmd_head[10];
    /* пробуем считать команду */
    if ( 1 == sscanf (cmd_cur_char, "%9s%n", cmd_head, &n) )
    {
      cmd_cur_char += n;

      /* subscribe */
      if ( !strcmp (cmd_head, cmd_sub) )
      {
        if ( 1 == sscanf (cmd_cur_char, "%u%n", &queue_no, &n) )
        {
          cmd_cur_char += n;

          if ( queue_no >= Client->config->queues_manager->n () )
          { no_answer = 1;
            err_answer = "no such query to subscribe";
          }
          else
          {
            Client->queues_subscribed->insert (queue_no);
          }
#ifdef _DEBUG
          printf ("%s %u answ=%d\n", cmd_sub, queue_no, !no_answer);
#endif // _DEBUG
        }
        else
        {
          no_answer = 2;
          err_answer = "incorrect command to server";
        }
      }

      /* publish */
      else if ( !strncmp (cmd_head, cmd_pub, 7U) )
      {
        /* пробуем считать параметры */
        if ( 1 == sscanf (cmd_cur_char, "%u%n", &queue_no, &n) )
        {
          cmd_cur_char += n;

          if ( queue_no >= Client->config->queues_manager->n () )
          {
            no_answer = 1;
            err_answer = "cannot insert the key";
          }
          else
          {
            Client->config->queues_manager->publish (queue_no, cmd_cur_char);
          }
#ifdef _DEBUG
          printf ("%s %u %s answ=%d\n", cmd_pub, queue_no, cmd_cur_char, !no_answer);
#endif // _DEBUG
        }
        else
        {
          no_answer = 2;
          err_answer = "incorrect command to server";
        } // end else        
      } // end if strcmp
      else
      {
        no_answer = 2;
        err_answer = "incorrect command to server";
      }
    } // end if scanf
    else
    {
      no_answer = 2;
      err_answer = "incorrect command to server";
    }
    //-----------------------------------------------------------------
    if ( !no_answer )
    { evbuffer_add_printf (buf_out, "ok <que %u>\n", queue_no); }
    else
    {
      evbuffer_add_printf (buf_out, "error : %s\n", err_answer);
      //-----------------------------------------------------------------
      if ( no_answer != 1 )
      { fprintf (stderr, "%s%s", err_prefix, strmyerror (SRV_ERR_RCMMN)); }
    }
    //-----------------------------------------------------------------
    free (cmd);
  }
  //-----------------------------------------------------------------
HNDL_FREE:;
  free (cmd);
  evbuffer_drain (buf_in, -1);
  return;
}
//-----------------------------------------



