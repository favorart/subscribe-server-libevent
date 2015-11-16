#include "stdafx.h"

#ifndef _SUBS_H_
#define _SUBS_H_

/*
 *  ! LINUX АРХИТЕКТУРА !
 *
 *  v   Сборка через make.
 *  v   Сервер при старте создаёт 4 дочерних
 *      процесса обработки комманд клиентов.
 *  v   Запуск: ./subs 
 *                     -d/--dir     <dir>
 *                     -h/--ip      <ip>
 *                     -p/--port    <port>
 *                     -w/--workers <num>
 *                     -q/--queues  <num>
 *                     -c/--conf    <path>
 *
 */
//-----------------------------------------
#define CHWMSG_MAXLEN 5
typedef enum
{ CHWMSG_NONE = 0,
  CHWMSG_TERM,
  CHWMSG_TASK
} chwmsg_enum;

typedef uint8_t               wc_t;
typedef int                   fd_t;
typedef struct child_worker  chw_t;
struct child_worker
{ pid_t pid;
  fd_t  fd;
};
//-----------------------------------------
int  child_worker_init (chw_t *chw);
void child_worker_free (chw_t *chw);
int  child_worker_send (chw_t *chw, chwmsg_enum  msg, fd_t  fd);
int  child_worker_recv (chw_t *chw, chwmsg_enum *msg, fd_t *fd);
//-----------------------------------------
#define  SRV_SERVER_NAME   "subscribe-server"
#define  SRV_IPSTRLENGTH   16U
#define  SRV_MAX_WORKERS   16U
#define  SRV_MAX_NQUEUES  255U
#define  SRV_BUF_LOWMARK  128U

typedef struct server_config  srv_conf;
struct  server_config
{
  //-----------------------
  char *ptr_server_name;
  //-----------------------
  uint16_t  port;
  char      ip[SRV_IPSTRLENGTH];
  //-----------------------
  wc_t      n_queues;
  wc_t      n_workers;
  chw_t    *myself;
  chw_t    *child_workers;
  //-----------------------
  /* defines FILENAME_MAX in <stdio.h> */
  char  config_path[FILENAME_MAX];
  char  server_path[FILENAME_MAX];
  //-----------------------
  struct event_base  *base;
  // !!! n-queues !!!!
  // struct hash_table  *ht;
  //-----------------------
};
extern struct server_config  server_conf;
//-----------------------------------------
int   server_config_init  (srv_conf *conf, char *port, char *ip, char *workers, char *queues);
void  server_config_print (srv_conf *conf, FILE *stream);
void  server_config_free  (srv_conf *conf);
//-----------------------------------------
int   parse_console_parameters (int argc, char **argv, srv_conf *conf);
//-----------------------------------------
int   set_nonblock (evutil_socket_t fd);
//-----------------------------------------
typedef struct hash_table hashtable;
struct client
{
  /* bufferevent already has separate
   * two buffers for input and output.
   */
  struct bufferevent *b_ev;
  struct event_base  *base;
  //---------------------------
  // !!! m-queue
  // struct hash_table  *ht;
};
//-----------------------------------------
void  subs_accept_cb  (evutil_socket_t fd, short ev, void *arg);
void  subs_ac_err_cb  (evutil_socket_t fd, short ev, void *arg);
//-----------------------------------------
void  subs_connect_cb (evutil_socket_t fd, short ev, void *arg);
//-----------------------------------------
void  subs_read_cb    (struct bufferevent *b_ev, void *arg);
void  subs_error_cb   (struct bufferevent *b_ev, short events, void *arg);
//-----------------------------------------
#endif // _SUBS_H_
