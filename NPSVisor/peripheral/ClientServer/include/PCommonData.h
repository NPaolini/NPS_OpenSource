//--------- PCommonData.h --------------------------------------------------------
//--------------------------------------------------------------------------------
#ifndef PCOMMONDATA_H_
#define PCOMMONDATA_H_
//--------------------------------------------------------------------------------
#include "hdrmsg.h"
//--------------------------------------------------------------------------------
#define MAX_BUFF_SOCK 8192
//--------------------------------------------------------------------------------
struct dataClientInfo
{
  LPCSTR className;
  LPCSTR msgName;
  LPCSTR fileCommand;
  LPCSTR fileData;
  int lenData;
  dataClientInfo() : className(0), msgName(0), fileCommand(0), fileData(0), lenData(0) {}
};
//--------------------------------------------------------------------------------
struct dataMsg
{
  DWORD Msg;
  WPARAM wP;
  LPARAM lP;
  dataMsg(DWORD msg = 0, WPARAM wp = 0, LPARAM lp = 0) :
    Msg(msg), wP(wp), lP(lp) {}
};
//--------------------------------------------------------------------------------
struct dataMsgForChanged
{
  dataMsg Data;
  int init;
  int end;

  bool isEqual(const dataMsgForChanged& other);

  // esegue l'unione delle due strutture (solo nel range)
//  const dataMsgForChanged&
  void unionRange(const dataMsgForChanged& other);

  dataMsgForChanged() : init(-1), end(-1) {}
};
//--------------------------------------------------------------------------------
#define OFFS_DATA_INIT sizeof(dataMsgForChanged)
//--------------------------------------------------------------------------------
class msgBaseData
{
    public:
      msgBaseData() : len(0)  { U.nextFree = 0;   }
      union {
        msgBaseData* nextFree;
        BYTE Buff[MAX_BUFF_SOCK];
        } U;

      LPBYTE getInitData() { return U.Buff + OFFS_DATA_INIT; }
      dataMsg& getStdMsg() { return *(dataMsg*)U.Buff; }
      dataMsgForChanged& getDataMsg() { return *(dataMsgForChanged*)U.Buff; }

      uint len;
};
//--------------------------------------------------------------------------------
typedef msgBaseData* pMsgBaseData;
//--------------------------------------------------------------------------------
#endif