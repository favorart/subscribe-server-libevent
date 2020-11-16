#include "stdafx.h"

#ifndef _SRV_ERROR_H_
#define _SRV_ERROR_H_

// #define _DEBUG
// #define MULTI_PROCESS
//-----------------------------------------
typedef enum
{ SRV_ERR_NONE,  SRV_ERR_PARAM,
  SRV_ERR_INPUT, SRV_ERR_LIBEV,
  SRV_ERR_RCMMN, SRV_ERR_RFREE,
  SRV_ERR_FDTRS
} myerr;
const char*  strmyerror (myerr err);
//-----------------------------------------
#endif // _SRV_ERROR_H_
