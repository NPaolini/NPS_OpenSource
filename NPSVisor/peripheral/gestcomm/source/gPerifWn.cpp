//------------ gPerifWn.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "commidt.h"
#include "gPerifWn.h"

#include <stdio.h>

#include "comgPerif.h"
#include "PServerDriver.h"
//----------------------------------------------------------------------------
//--------------------------------------------------------
TgPerif_Window::TgPerif_Window(LPCTSTR title, HINSTANCE hInst)
    : gestCommWindow(title, hInst) { }
//--------------------------------------------------------
TgPerif_Window::~TgPerif_Window()
{
  destroy();
}
//--------------------------------------------------------
void TgPerif_Window::processSVisorMsgEx(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, DWORD id)
{
  if(!gComm)
    return;

  gestCommgPerif *periph = reinterpret_cast<gestCommgPerif*>(gComm);
  switch(LOWORD(wParam)) {
    case MSG_READ_CUSTOM:
      periph->addCommand(gestCommgPerif::readCustom, HIWORD(wParam), lParam, id);
      break;
    case MSG_WRITE_CUSTOM:
      periph->addCommand(gestCommgPerif::writeCustom, HIWORD(wParam), lParam, id);
      break;
    case MSG_WRITE_BITS:
      periph->addCommand(gestCommgPerif::writeBits, HIWORD(wParam), lParam, id);
      break;
    case MSG_WRITE_BITS_16:
      periph->addCommand(gestCommgPerif::writeBits16, HIWORD(wParam), lParam, id);
      break;
    case MSG_ENABLE_READ:
      if(periph->addReadReq(HIWORD(wParam), (DWORD)lParam)) {
        commIdentity *cI = getIdentity();
        HWND hwnd = cI->getHSuperVisor();
        PostMessage(hwnd, WM_MAIN_PRG, wParam, lParam);
        }
      break;
    case MSG_ENABLE_ONE_READ:
      if(periph->addOneReadReq(HIWORD(wParam), (DWORD)lParam)) {
        commIdentity *cI = getIdentity();
        HWND hwnd = cI->getHSuperVisor();
        PostMessage(hwnd, WM_MAIN_PRG, wParam, lParam);
        }
      break;
    case MSG_WRITE_SINGLE_VAR:
      periph->addSingleWrite((DWORD)lParam, HIWORD(wParam));
      break;
    }
}
//--------------------------------------------------------
LRESULT TgPerif_Window::processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_MAIN_PRG == message) {
    switch(LOWORD(wParam)) {
      case MSG_READ_CUSTOM:
      case MSG_WRITE_CUSTOM:
      case MSG_WRITE_BITS:
      case MSG_WRITE_BITS_16:
      case MSG_ENABLE_READ:
      case MSG_ENABLE_ONE_READ:
      case MSG_WRITE_SINGLE_VAR:
        processSVisorMsgEx(hwnd, message, wParam, lParam, 0);
        break;
      case MSG_REMOVE_ID:
        do {
          commIdentity *cI = getIdentity();
          cI->removeId((uint)lParam);
          } while(false);
        break;
      case MSG_ADD_ID:
        do {
          commIdentity *cI = getIdentity();
          cI->addId((uint)lParam);
          } while(false);
        break;
      case MSG_HIDE_SHOW_DRIVER:
        do {
          bool hide = toBool(HIWORD(wParam));
          commIdentity *cI = getIdentity();
          if(cI->needHide() != hide) {
            cI->setNeddHide(hide);
            ShowWindow(*this, hide ? SW_HIDE : SW_SHOWNA);
            }
          } while(false);
        break;
      }
    }
  else if(WM_COPYDATA == message) {
    do {
      COPYDATASTRUCT cds = *(COPYDATASTRUCT*)lParam;

      commIdentity *cI = getIdentity();
      if(cds.dwData != cI->get_WM())
        break;

      dataForServer data = *(dataForServer*)cds.lpData;

      ReplyMessage(LRESULT (1));

      dataMsg& msg = data.Msg;
#if 0
      if(MSG_REMOVE_ID == LOWORD(msg.wP))
        cI->removeId(data.id);

      else
#endif
        if(cI->setHWND((HWND)wParam, data.id) >= 0)
          processSVisorMsgEx(hwnd, msg.Msg, msg.wP, msg.lP, data.id);

      } while(false);
    return 1;
    }

  return gestCommWindow::processSVisorMsg(hwnd, message, wParam, lParam);
}
//--------------------------------------------------------
//------------------------------------------------------------------
static commIdentity gen_Identity;
//------------------------------------------------------------------
commIdentity *getIdentity()
{
  return &gen_Identity;
}
//------------------------------------------------------------------


