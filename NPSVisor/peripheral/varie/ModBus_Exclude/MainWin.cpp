//--------------- MainWin.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "commidt.h"
#include "gperifwn.h"
#include "hdrmsg.h"
//----------------------------------------------------------------------------
PVect<DWORD> DisabledAddr;
//----------------------------------------------------------------------------
const PVect<DWORD>& getDisabled() { return DisabledAddr; }
//----------------------------------------------------------------------------
bool isValidAddr(DWORD addr)
{
  int nElem = DisabledAddr.getElem();
  for(int i = 0; i < nElem; ++i)
    if(addr == DisabledAddr[i])
      return false;
  return true;
}
//----------------------------------------------------------------------------
class modBus_Exclude_Window : public TgPerif_Window
{
  private:
    typedef TgPerif_Window baseClass;
  public:
    modBus_Exclude_Window(LPCTSTR title = 0, HINSTANCE hInst = 0) : TgPerif_Window(title, hInst)
    {          }

    // numero di cicli chiusi massimi che effettua se non ci sono errori,
    // né messaggi nella coda del thread, e ci sono azioni nella coda della classe
//    virtual uint getMoreCycleCount() const { return 0; } // default 5

    // se un driver ha necessità diverse può variare la frequenza con cui viene
    // richiamata la routine di gestione
//    virtual uint getTimerTick() const { return 500; }    // default 200
  protected:
    virtual LRESULT processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void processSVisorEnableDisable(bool enable, DWORD addr);

};
//----------------------------------------------------------------------------
LRESULT modBus_Exclude_Window::processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_MAIN_PRG == message) {
    switch(LOWORD(wParam)) {
      case MSG_ENABLE_DISABLE:
        processSVisorEnableDisable(toBool(HIWORD(wParam)), (DWORD)lParam);
        break;
      }
    }

  return baseClass::processSVisorMsg(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void modBus_Exclude_Window::processSVisorEnableDisable(bool enable, DWORD addr)
{
  int nElem = DisabledAddr.getElem();
  for(int i = 0; i < nElem; ++i)
    if(addr == DisabledAddr[i]) {
      if(enable)
        DisabledAddr.remove(i);
      return;
      }
  if(!enable)
    DisabledAddr[nElem] = addr;
}
//----------------------------------------------------------------------------
gestCommWindow* commIdentity::getMainWindow(LPCTSTR title, HINSTANCE hInst)
{
  return new modBus_Exclude_Window(title, hInst);
}


