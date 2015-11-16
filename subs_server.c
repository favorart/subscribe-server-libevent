#include "stdafx.h"
#include "subs_error.h"
// #include "subs_hash.h"
#include "subs.h"

void  subscribe_handling (/* hashtable *ht, */ struct evbuffer *buf_in, struct evbuffer *buf_out);
//-----------------------------------------
void  subs_ac_err_cb (evutil_socket_t fd, short ev, void *arg)
{
  // struct event_base *base = (struct event_base*) arg;
  struct server_config *server_conf = (struct server_config*) arg;
  //----------------------------------------------------------------------
  int err = EVUTIL_SOCKET_ERROR ();
  fprintf (stderr, "Got an error %d (%s) on the listener. "
           "Shutting down.\n", err, evutil_socket_error_to_string (err));
  //----------------------------------------------------------------------
  event_base_loopexit (server_conf->base, NULL);
}
void  subs_accept_cb (evutil_socket_t fd, short ev, void *arg)
{
  struct server_config *server_conf = (struct server_config*) arg;
  //----------------------------------------------------------------------
  int  SlaveSocket = accept (fd, 0, 0);
  if ( SlaveSocket == -1 )
  { fprintf (stderr, "%s\n", strerror (errno));
    return;
  }
  //----------------------------------------------------------------------
  set_nonblock (SlaveSocket);
  //----------------------------------------------------------------------
  // !!! <--- UNIFORM SERVERS DISTRIBUTION
  wc_t r = (rand () % server_conf->n_workers);
  child_worker_send (&server_conf->child_workers[r], CHWMSG_TASK, SlaveSocket);
  //----------------------------------------------------------------------
#ifdef _DEBUG
  printf ("connection accepted\n");
#endif
  //----------------------------------------------------------------------
}
//-----------------------------------------
void  subs_connect_cb (evutil_socket_t fd, short ev, void *arg)
{
  struct server_config *server_conf = (struct server_config*) arg;
  //----------------------------------------------------------------------
  /* Making the new client */
  struct client *Client = (struct client*) calloc (1U, sizeof (*server_conf));
  if ( !Client )
  { fprintf (stderr, "%s\n", strerror (errno));
    return;
  }

  Client->base = server_conf->base;
  // !!! <----
  // Client->ht   = server_conf->ht;
  //----------------------------------------------------------------------
  evutil_socket_t  SlaveSocket = -1;
  chwmsg_enum msg;
  child_worker_recv (server_conf->myself, &msg, &SlaveSocket);

  if ( msg == CHWMSG_TERM )
  { event_base_loopexit (server_conf->base, NULL); }
  else
  {
    //----------------------------------------------------------------------
    /* Create new bufferized event, linked with client's socket */
    Client->b_ev = bufferevent_socket_new (server_conf->base, SlaveSocket, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb (Client->b_ev, subs_read_cb, NULL, subs_error_cb, Client);
    /* Ready to get data */
    bufferevent_enable (Client->b_ev, EV_READ | EV_WRITE | EV_PERSIST);
    //----------------------------------------------------------------------
    bufferevent_setwatermark (Client->b_ev, EV_WRITE, SRV_BUF_LOWMARK, 0);
    //----------------------------------------------------------------------
#ifdef _DEBUG
    printf ("connection to worker # %d\n", server_conf->n_workers);
#endif
  }
}
//-----------------------------------------
void  subs_error_cb (struct bufferevent *b_ev, short events, void *arg)
{
  /*   В заголовочном файле <event2/bufferevent.h> определены флаги:
   *   соединение было установлено (BEV_EVENT_CONNECTED 0x80)б
   *   обнаружен признак конца файла (BEV_EVENT_EOF 0x10),
   *   произошёл критический сбой (BEV_EVENT_ERROR 0x20),
   *   истёк заданный интервал (BEV_EVENT_TIMEOUT 0x40).
   */
       if ( events & BEV_EVENT_CONNECTED )
  {
#ifdef _DEBUG
    printf ("connection established\n");
#endif
  }
  else if ( events & BEV_EVENT_EOF       )
  {
    struct client   *Client = (struct client*) arg;
    
#ifdef _DEBUG
    printf ("got a close. length = %u\n", evbuffer_get_length (bufferevent_get_input (Client->b_ev)) );
#endif // _DEBUG
    
    shutdown (bufferevent_getfd (Client->b_ev), SHUT_RDWR);
    bufferevent_free (Client->b_ev);
    free (Client);

#ifdef _DEBUG
    printf ("connection closed\n");
#endif // _DEBUG

  }
  else if ( events & BEV_EVENT_ERROR     )
  {
    struct client *Client = (struct client*) arg;

    fprintf (stderr, "Error from bufferevent: '%s'\n",
             evutil_socket_error_to_string (EVUTIL_SOCKET_ERROR ()) );
    
    bufferevent_free (Client->b_ev);
    free (Client);

#ifdef _DEBUG
    printf ("connection closed");
#endif // _DEBUG
  }
}
void  subs_read_cb  (struct bufferevent *b_ev, void *arg)
{

#ifdef _DEBUG
  printf ("data reseived\n");
#endif // _DEBUG

  /* This callback is invoked when there is data to read on b_ev_read */
  struct client  *Client = (struct client*) arg;
  
  struct evbuffer *buf_in  = bufferevent_get_input  (b_ev);
  struct evbuffer *buf_out = bufferevent_get_output (b_ev);

  // !!! <---
  subscribe_handling (/* Client->ht, */ buf_in, buf_out);
  
#ifdef _DEBUG
  printf ("response ready\n");
#endif // _DEBUG
}
//-----------------------------------------
