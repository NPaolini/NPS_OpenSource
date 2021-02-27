//-------------------- gestComm.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "gestComm.h"
#include "p_util.h"
//------------------------------------------------
//------------------------------------------------
gestComm::gestComm(LPCTSTR file)
    : LastErr(NoErr), Logoff(false), Com(0),  FileName(str_newdup(file)),
      Dirty(false), RespData(0),
      sResponce(MAKE_NO_RESP), idReq(0), needResponce(false)
{ }
//-------------------------------------------------
gestComm::~gestComm()
{
  delete []FileName;
  delete Com;
}
//-------------------------------------------------
