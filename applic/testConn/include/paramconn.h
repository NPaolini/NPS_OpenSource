//--------- paramconn.h --------------------------------------------
//-----------------------------------------------------------------
#ifndef PARAMCONN_H_
#define PARAMCONN_H_
//-----------------------------------------------------------------
#include "precompHeader.h"
//----------------------------------------------------------------------------
/* These baudrates work on windows, but are not part of the windows headers */
#ifndef CBR_230400
#define CBR_230400 230400
#endif
#ifndef CBR_460800
#define CBR_460800 460800
#endif
#ifndef CBR_921600
#define CBR_921600 921600
#endif
//-----------------------------------------------------------------
// struttura tipica per una comunicazione seriale
// per altri tipi di comunicazione può essere comunque usato (cast)
// il valore DWORD per altri usi
struct paramConn
{
  // da usare per settare i valori
  enum ePort { Com1, Com2, Com3, Com4, Com5, Com6, Com7, Com8 };
  enum eBaudRate {
    _110, _300, _600, _1200, _2400, _4800, _9600, _14400, _19200,
    _38400, _56000, _57600, _115200, _128000, _256000, _230400, _460800, _921600 };
  enum eParity {  EVEN, MARK, NO_PARITY,  ODD, SPACE };
  enum eBitStop { ONE, ONE5, TWO };
  enum eBitLen  { _5, _6, _7, _8 };
  enum eDtrDsr  { NO_DTR_DSR, ON_DTR_DSR, HANDSHAKE_DTR_DSR };
  enum eRtsCts  { NO_RTS_CTS, ON_RTS_CTS, HANDSHAKE_RTS_CTS, TOGGLE_RTS_CTS };
  enum eXonXoff { NO_XON_XOFF, XON_XOFF };

  struct _b {
    private:
      DWORD port    : 4; // 0 == COM1, per compatibilità col passato
    public:
     // i valori rappresentano indici delle tabelle
      DWORD baud    : 5;
      DWORD stop    : 2;
      DWORD parity  : 3;
      DWORD len     : 2;
      DWORD dtr_dsr : 2;
      DWORD rts_cts : 2;
      DWORD xon_xoff: 1;
    private:
      DWORD portH   : 4; // servono più bit e ora si usa questa insieme all'altra
      DWORD dummy   : 7; // serve per azzerare la parte restante della DWORD
    public:
      _b(DWORD p, eBaudRate b, eBitStop s, eParity pr, eBitLen l, eDtrDsr dtr,
           eRtsCts rts, eXonXoff x) :
           baud(b), stop(s), parity(pr), len(l),
           dtr_dsr(dtr), rts_cts(rts), xon_xoff(x), dummy(0) { setPort(p); }
      _b() : dummy(0) {}
      void setPort(DWORD p) { port = p & 0xf; portH = (p >> 4) & 0xf; }
      DWORD getPort() { return port | ((DWORD)portH << 4); }
      } b;

  paramConn(ePort p = Com2, eBaudRate b = _9600, eBitStop s = ONE,
           eParity pr = NO_PARITY, eBitLen l = _8, eDtrDsr dtr = HANDSHAKE_DTR_DSR,
           eRtsCts rts = HANDSHAKE_RTS_CTS, eXonXoff x = NO_XON_XOFF) :
           b(p, b, s, pr, l, dtr, rts, x) {}
  paramConn(_b v) : b(v) {}
  paramConn(DWORD v) : b(*reinterpret_cast<_b*>(&v)) {}
  operator DWORD() const { return *reinterpret_cast<const DWORD*>(&b); }

  // usare queste per accedere alla porta
  void setPort(DWORD p) { b.setPort(p);  }
  DWORD getPort() { return b.getPort(); }
};
//-----------------------------------------------------------------
#endif
