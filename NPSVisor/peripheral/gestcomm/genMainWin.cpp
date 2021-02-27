//--------------- genMainWin.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "commidt.h"
//#include "comgplc.h"
#include "gperifwn.h"
//----------------------------------------------------------------------------
// da non usare se si specializza la classe TPLC_Window
// occorre creare un nuovo file (o inserire direttamente la chiamata
// nella definizione della nuova classe) e rimuovere questo dal target
gestCommWindow* commIdentity::getMainWindow(LPCTSTR title, HINSTANCE hInst)
{
  return new TgPerif_Window(title, hInst);
}
//----------------------------------------------------------------------------
