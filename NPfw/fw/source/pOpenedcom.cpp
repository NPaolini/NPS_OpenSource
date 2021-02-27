//------------------ POpenedCom.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <process.h>
//-----------------------------------------------------------
#include "POpenedCom.h"
#include "p_util.h"
//-----------------------------------------------------------
struct dualBaud
{
  DWORD b;
  DWORD maxB;
};
//-----------------------------------------------------------
static const dualBaud vBaud[] =
{
  { CBR_110,    BAUD_075    },
  { CBR_110,    BAUD_110    },
  { CBR_110,    BAUD_134_5  },
  { CBR_110,    BAUD_150    },
  { CBR_300,    BAUD_300    },
  { CBR_600,    BAUD_600    },
  { CBR_1200,   BAUD_1200   },
  { CBR_1200,   BAUD_1800   },
  { CBR_2400,   BAUD_2400   },
  { CBR_4800,   BAUD_4800   },
  { CBR_4800,   BAUD_7200   },
  { CBR_9600,   BAUD_9600   },
  { CBR_14400,  BAUD_14400  },
  { CBR_19200,  BAUD_19200  },
  { CBR_38400,  BAUD_38400  },
  { CBR_56000,  BAUD_56K    },
  { CBR_128000, BAUD_128K   },
  { CBR_115200, BAUD_115200 },
  { CBR_57600,  BAUD_57600  },
  { CBR_256000, BAUD_USER   },
};
//-----------------------------------------------------------
#define PURGE_ALL (PURGE_TXABORT | PURGE_RXABORT | \
                   PURGE_TXCLEAR | PURGE_RXCLEAR)
//-----------------------------------------------------------
bool POpenedCom::open()
{
  HANDLE id = getIdCom();
  if(id == INVALID_HANDLE_VALUE)
    return false;
//    return baseClass::open();

  BYTE propBuff[sizeof(COMMPROP) + sizeof(MODEMDEVCAPS) + 2];
  COMMPROP* commprop = (COMMPROP *)propBuff;
  ZeroMemory(propBuff, sizeof(propBuff));
  // se Win9x vuole il buffer azzerato e funziona, se fallisce siamo in winNT
  // e vuole un buffer più grande ed inizializzato
  if(!GetCommProperties(id, commprop)) {
    if(122 == GetLastError()) {
      DWORD dwSize = sizeof(COMMPROP) + sizeof(MODEMDEVCAPS);
      commprop->wPacketLength = (WORD)dwSize;
      commprop->dwProvSubType = PST_MODEM;
      commprop->dwProvSpec1 = COMMPROP_INITIALIZED;
      if(!GetCommProperties(id, commprop))
        return false;
      }
    }

  SetCommMask(id, EV_RXCHAR);
//  SetupComm(id, 8192, 8192);

  PurgeComm(id, PURGE_ALL);

//  COMMTIMEOUTS sTimeoutPolling={ 100, 10, 1500, 10, 1500 };
  int to = 0;
  for(uint i = 0; i < SIZE_A(vBaud); ++i)
    if(vBaud[i].maxB == commprop->dwMaxBaud) {
      to = 2 * CBR_9600 / vBaud[i].b;
      break;
      }
  COMMTIMEOUTS sTimeoutEvent={ 0xffffffff, 0, 1000, to, 0 };
  SetCommTimeouts(id, &sTimeoutEvent);

  connected = true;

#if 0
  HANDLE hThread = (HANDLE)_beginthreadex(0, 0, CommProc, this, 0, &idThread);
#else
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CommProc,
                  this, 0, (LPDWORD)&idThread);
#endif
  if(!hThread) {
    connected = false;
//    CloseHandle(id);
    return false;
    }
  // da provare
  SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
//  SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
  CloseHandle(hThread);

  criticalLock crtLck(cS);
  return true;
}
//------------------------------------------------
/*
void POpenedCom::close()
{
  if(getIdCom() != INVALID_HANDLE_VALUE) {
    HANDLE id = getIdCom();
    connected = false;
    SetCommMask(id, 0);
    while(idThread != 0) {
      SetCommMask(id, 0);
      Sleep(100);
      }
    EscapeCommFunction(id, CLRDTR);
    PurgeComm(id, PURGE_ALL);
    CloseHandle(id);
    setIdCom(INVALID_HANDLE_VALUE);
    }
}
*/
//-----------------------------------------------------------
