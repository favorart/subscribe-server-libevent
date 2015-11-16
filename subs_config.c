#include "stdafx.h"
#include "subs_error.h"
#include "subs.h"

//-----------------------------------------
// !!! <---
void  signal_handler (int sig)
{
  if ( sig == SIGINT )
  {
    for ( wc_t i = 0; i < server_conf.n_workers; ++i )
    { kill (server_conf.child_workers[i].pid, SIGTERM); }
    kill (getpid (), SIGTERM);
  }
  else if ( sig == SIGCHLD )
  {
    while ( true )
    {
      int status;
      int pid = waitpid (WAIT_ANY, &status, WNOHANG);
      if ( pid < 0 )
      { perror ("waitpid");
        break;
      }
      // else if ( pid == 0 )
      { break; }
      // notice_termination (pid, status);
    }
  }
  // else if ( sig == SIGTERM )
  // { event_loopexit (base, NULL); }
}
//-----------------------------------------
const char*  strmyerror ()
{
  const char* strerr;
  switch ( my_errno )
  {
    default:            strerr = NULL;                              break;
    case SRV_ERR_PARAM: strerr = " : Invalid function argument.\n"; break;
    case SRV_ERR_INPUT: strerr = " : Input incorrect user data.\n"; break;
    case SRV_ERR_LIBEV: strerr = " : Incorrect libevent entity.\n"; break;
    case SRV_ERR_RCMMN: strerr = " : Incorrect request command.\n"; break;
    case SRV_ERR_RFREE: strerr = " : Dispatched a free request.\n"; break;
    case SRV_ERR_FDTRS: strerr = " : Failure in transmiting fd.\n"; break;
  }
  my_errno = SRV_ERR_NONE;
  return strerr;
}
//-----------------------------------------
int   set_nonblock (evutil_socket_t fd)
{
  int flags;
#ifdef O_NONBLOCK
  if ( -1 == (flags = fcntl (fd, F_GETFL, 0)) )
    flags = 0;
  return fcntl (fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  return ioctl (fd, FIOBIO, &flags);
#endif
}
//-----------------------------------------
int   server_config_init  (srv_conf *conf, char *port, char *ip,
                           char  *workers, char *queues)
{
  bool fail = false;
  //-----------------------
  const uint16_t  max_port = 65535U;
  if ( !port || (conf->port = atoi (port)) > max_port )
  { my_errno = SRV_ERR_INPUT;
    fprintf (stderr, "%s\n", strmyerror ());
    conf->port = 8080;
  }
  //-----------------------
  if ( ip && strlen (ip) < SRV_IPSTRLENGTH )
  { strcpy (conf->ip, ip); }
  else
  { my_errno = SRV_ERR_INPUT;
    fprintf (stderr, "%s\n", strmyerror ());
    strcpy (conf->ip, "0.0.0.0");
  }
  //-----------------------
  conf->ptr_server_name = SRV_SERVER_NAME;
  //-----------------------
  int  bytes = 0U;
  if ( 0 >= (bytes = readlink ("/proc/self/exe", conf->server_path, FILENAME_MAX - 1U)) )
  { fprintf (stderr, "%s\n", strerror (errno));
    strcpy (conf->server_path, "");
  }
  else conf->server_path[bytes] = '\0';
  //-----------------------
  if ( workers )
  { conf->n_workers = atoi (workers); }
  else
  { my_errno = SRV_ERR_INPUT;
    fprintf (stderr, "%s\n", strmyerror ());
    conf->n_workers = 4U;
  }

  if ( queues )
  { conf->n_queues = atoi (queues); }
  else
  { my_errno = SRV_ERR_INPUT;
    fprintf (stderr, "%s\n", strmyerror ());
    conf->n_queues = 10U;
  }
  //-----------------------
  if ( !(conf->child_workers = calloc (conf->n_workers, sizeof (*conf->child_workers))) )
  { perror ("child_workers");
    exit (EXIT_FAILURE);
  }

  for ( wc_t i = 0U; i < conf->n_workers; ++i )
  {
    if ( !child_worker_init (&conf->child_workers[i]) )
    {
      conf->myself = &conf->child_workers[i];
      conf->n_workers = (i + 1);
      break;
    }
    else (conf->myself = NULL);
  }
  //-----------------------
CONF_FREE:;
  if ( fail )
    server_config_free (conf);
  //-----------------------
  return fail;
}
void  server_config_print (srv_conf *conf, FILE *stream)
{
  fprintf (stream, ">>> http server config:\n\n\tname: '%s'\tserver path: '%s'\n"
                   "\tport: %u\tip: '%s'\nworkers: %d\tqueues: %d\n\n",
                   conf->ptr_server_name, conf->server_path,
                   conf->port, conf->ip, conf->n_workers, conf->n_queues);
}
void  server_config_free  (srv_conf *conf)
{
  //-----------------------
  for ( wc_t i = 0U; i < conf->n_workers; ++i )
    child_worker_free (&conf->child_workers[i]);
  free (conf->child_workers);
  //-----------------------
  memset (conf, 0, sizeof (*conf));
}
//-----------------------------------------
#include <getopt.h>   /* for getopt_long finction */
int   parse_console_parameters (int argc, char **argv, srv_conf *conf)
{
  char   *dir_opt = NULL,  *port_opt = NULL,   *ip_opt = NULL,
        *work_opt = NULL, *queue_opt = NULL, *conf_opt = NULL;
  static struct option long_options[] =
  { {     "dir", required_argument, 0, 'd' },
    {      "ip", required_argument, 0, 'h' },
    {    "port", required_argument, 0, 'p' },
    { "workers", required_argument, 0, 'w' },
    {  "queues", required_argument, 0, 'q' },
    {    "conf", required_argument, 0, 'c' },
    { 0, 0, 0, 0 }
  };
  //-----------------------
  int c, option_index = 0;
  while ( -1 != (c = getopt_long (argc, argv, "c:",
                                  long_options, &option_index)) )
  {
    switch ( c )
    {
      case 0:
        printf ("option %s", long_options[option_index].name);
        if ( optarg )
          printf (" with arg %s", optarg);
        printf ("\n");
        //-----------------------
        switch ( option_index )
        {
          case 0:  dir_opt = optarg; break;
          case 1:   ip_opt = optarg; break;
          case 2: port_opt = optarg; break;
          case 3: work_opt = optarg; break;
          case 4:
            printf ("using:\n\t./wwwd -d|--dir <dir> -h|--ip <ip> -p|--port <port>\n"
                    "\t-w|--workers <num> -q|--queues <num> -c|--conf <conf>\n\n");
            break;
        }
        break;

      case 'd':   dir_opt = optarg; break;
      case 'h':    ip_opt = optarg; break;
      case 'p':  port_opt = optarg; break;
      case 'w':  work_opt = optarg; break;
      case 'q': queue_opt = optarg; break;
      case 'c':  conf_opt = optarg; break;

      case '?':
        /* getopt_long already printed an error message. */
        printf ("using:\n\t./wwwd -d|--dir <dir> -h|--ip <ip> -p|--port <port>\n"
                "\t-w|--workers <num> -q|--queues <num> -c|--conf <conf>\n\n");
        break;

      default:
        printf ("?? getopt returned character code 0%o ??\n", c);
        break;
    }
  }
  //-----------------------
  if ( optind < argc )
  {
    printf ("non-option ARGV-elements: ");
    while ( optind < argc )
      printf ("%s ", argv[optind++]);
    printf ("\n");
  }
  //-----------------------
  return  server_config_init (conf, dir_opt, ip_opt, port_opt,
                                   work_opt, queue_opt);
}
//-----------------------------------------
