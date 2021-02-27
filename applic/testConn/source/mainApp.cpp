//-------- mainapp.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "mainApp.h"
#include "mainclient.h"
//----------------------------------------------------------------------------
// se attiva non permette l'avvio di più applicazioni
bool ONLY_ONE = false;
//----------------------------------------------------------------------------
PWin* allocMainClient(PWin* parent, HINSTANCE hInstance)
{
  mainClient* client = new mainClient(parent, IDD_CLIENT, hInstance);
  return client;
}
//----------------------------------------------------------------------------
void customizeMain(PWin *w)
{
  w->Attr.style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
//  w->Attr.style &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
  w->Attr.style |= WS_CLIPCHILDREN | WS_MINIMIZEBOX;
  w->Attr.w = 30;
  w->Attr.h = 30;
}
//----------------------------------------------------------------------------
// deve tornare il nome della classe
LPCTSTR getClassName()
{
  return _T("pAppl - Test Conn");
}
//----------------------------------------------------------------------------
//---------------------------------------------------------
