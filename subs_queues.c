#include "stdafx.h"
#include "subs_error.h"
// #include "subs_hash.h"
#include "subs.h"

//-----------------------------------------
/*  ѕротокол:  publish <num> || subscribe <num>
 *  ќтвет:     ok <num> || error <err_text>
 *  
 *  
 *  ≈сть сервер, у него есть N очередей сообщений.
 *  ≈сть сервер-клиент, который может положить сообщение на сервер в определЄнную очередь
 *  или подписатьс€ на прослушивание очереди.
 */
void  subscribe_handling (/* hashtable *ht, */ struct evbuffer *buf_in, struct evbuffer *buf_out)
{
  const char * const  err_prefix = "subscribe handling";
  const char * const  cmd_get = "subscribe";
  const char * const  cmd_set = "publish";
  const char *err_answer = "";
  int   no_answer = 0;
  //-----------------------------------------------------------------
  char   *cmd = NULL;
  size_t  cmd_length = 0U;

  while ( (cmd = evbuffer_readln (buf_in, &cmd_length, EVBUFFER_EOL_ANY)) )
  {
    char*   cmd_cur_char = cmd;
    size_t  n = 0;
    //-----------------------------------------------------------------
    ht_rec hd = { 0 };
    char cmd_head[3];

    /* пробуем считать команду */
    if ( 1 == sscanf (cmd_cur_char, "%3s%n", cmd_head, &n) )
    {
      cmd_cur_char += n;

      if ( !strcmp (cmd_head, cmd_get) )
      {
        if ( 1 == sscanf (cmd_cur_char, "%d%n", &hd.key, &n) )
        {
          cmd_cur_char += n;

          if ( hashtable_get (ht, &hd) )
          {
            no_answer = 1;
            err_answer = "no such key in table";
          }
        }
        else
        {
          no_answer = 2;
          err_answer = "incorrect command to server";
        }
      }
      else if ( !strcmp (cmd_head, cmd_set) )
      {
        int32_t ttl = 0;
        /* пробуем считать параметры */
        if ( 3 == sscanf (cmd_cur_char, "%d%d%d%n", &ttl, &hd.key, &hd.val, &n) )
        {
          cmd_cur_char += n;

          hd.ttl = ttl_converted (ttl);
          if ( hashtable_set (ht, &hd) )
          {
            no_answer = 1;
            err_answer = "cannot insert the key";
          }
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
#ifdef _DEBUG
    printf ("key=% 4d ttl=% 3.2lf answ=%d\n", hd.key,
            !hd.ttl ? 0. : difftime (time (NULL), hd.ttl), !no_answer);
#endif // _DEBUG
    //-----------------------------------------------------------------
    if ( !no_answer )
    {
      evbuffer_add_printf (buf_out, "ok %d %d\n", hd.key, hd.val);
    }
    else
    {
      evbuffer_add_printf (buf_out, "error : %s\n", err_answer);
      //-----------------------------------------------------------------
      if ( no_answer != 1 )
      {
        my_errno = SRV_ERR_RCMMN;
        fprintf (stderr, "%s%s", err_prefix, strmyerror ());
      }
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