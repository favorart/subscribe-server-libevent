#include "stdafx.h"
// #include "subs_error.h"
// #include "subs_server.h"


#ifndef _SUBS_QUEUES_H_
#define _SUBS_QUEUES_H_
//-----------------------------------------
struct client;

class QueuesManager
{
  size_t n_;
  bool news_;
  std::vector< std::queue<std::string> > queues_;
  
public:
  QueuesManager (size_t n) : n_ (n), queues_ (n) {}

  void     n (size_t n)
  {
    if ( !n ) n = 1;
    n_ = n;
    queues_.reserve (n);
  }
  size_t   n () { return n_; }
  bool  news () { return news_; }

  void publish (size_t n, char *message)
  { queues_[n].push (message);
    news_ = true;
  }

  void  broadcast (struct server_config *config);
};
//-----------------------------------------
#endif // _SUBS_QUEUES_H_