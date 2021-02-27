//---------- P_Setup.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Setup.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "popensave.h"
#include "mainwin.h"
#include <Shlwapi.h>
#pragma comment(linker, "/defaultlib:Shlwapi.lib")
//----------------------------------------------------------------------------
P_Setup::P_Setup(PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_FOLDER, IDB_BITMAP_OK, IDB_BITMAP_CANC };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDC_BUTTON_IMG_FOLDER, Bmp[0]);
  new POwnBtnImageStd(this, IDC_BUTTON_SOUND, Bmp[0]);
  new POwnBtnImageStd(this, IDC_BUTTON_SOUND_ALERT, Bmp[0]);
  new POwnBtnImageStd(this, IDOK, Bmp[1]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[2]);
}
//----------------------------------------------------------------------------
P_Setup::~P_Setup()
{
  destroy();
  flushPV(Bmp);
}
//----------------------------------------------------------------------------
bool P_Setup::compareAndSave(LPCTSTR keyname, uint idc, uint typePath)
{
  TCHAR buff1[_MAX_PATH];
  TCHAR buff2[_MAX_PATH];
  getKeyPath(keyname, buff1);
  GET_TEXT(idc, buff2);
  if(typePath)
    makeRelPath(buff2, SIZE_A(buff2));
  if(_tcsicmp(buff1, buff2)) {
    setKeyPath(keyname, buff2);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void P_Setup::BNClickedOk()
{
  mainWin* mw = getParentWin<mainWin>(this);
  if(!mw)
    return;

  bool needRefresh = compareAndSave(IMG_PATH, IDC_EDIT_IMG_FOLDER, 1);
  if(needRefresh)
    mw->refreshFolderImg();
  needRefresh = compareAndSave(AUDIO_BASE_FOLDER, IDC_EDIT_SOUND, 2);
  if(true || needRefresh)
    mw->refreshAudioBase(0);
  needRefresh = compareAndSave(AUDIO_ALERT_FOLDER, IDC_EDIT_SOUND_ALERT, 2);
  if(true || needRefresh)
    mw->refreshAudioBase(1);
  needRefresh = compareAndSave(AUDIO_ALERT_TIME, IDC_EDIT_AUDIO_ALERT_TIME, 0);
  if(needRefresh)
    mw->refreshAudioBaseTime(1);

  SYSTEMTIME st;
  DWORD result = DateTime_GetSystemtime(GetDlgItem(*this, IDC_DATETIMEPICKER1), &st);
  if(GDT_VALID == result) {
    TCHAR t[256];
    wsprintf(t, _T("%d,%d,%d"), st.wHour, st.wMinute, st.wSecond);
    TCHAR t2[256];
    getKeyPath(MAX_TIME_TIMER, t2);
    if(_tcsicmp(t, t2)) {
      setKeyPath(MAX_TIME_TIMER, t);
      mw->resetTimer();
      }
    }
}
//----------------------------------------------------------------------------
bool P_Setup::create()
{
  if(!baseClass::create())
    return false;

  fillCtrl();
  return true;
}
//----------------------------------------------------------------------------
void P_Setup::fillCtrl()
{
  TCHAR buff[_MAX_PATH];
  getKeyPath(IMG_PATH, buff);
  SET_TEXT(IDC_EDIT_IMG_FOLDER, buff);

  getKeyPath(AUDIO_BASE_FOLDER, buff);
  SET_TEXT(IDC_EDIT_SOUND, buff);

  getKeyPath(AUDIO_ALERT_FOLDER, buff);
  SET_TEXT(IDC_EDIT_SOUND_ALERT, buff);

  getKeyPath(AUDIO_ALERT_TIME, buff);
  SET_TEXT(IDC_EDIT_AUDIO_ALERT_TIME, buff);

  getKeyPath(MAX_TIME_TIMER, buff);
  SYSTEMTIME st;
  GetSystemTime(&st);
  int t[3];
  _stscanf_s(buff, _T("%d,%d,%d"), &t[0], &t[1], &t[2]);
  st.wHour = t[0];
  st.wMinute = t[1];
  st.wSecond = t[2];
  DateTime_SetSystemtime(GetDlgItem(*this, IDC_DATETIMEPICKER1), GDT_VALID, &st);
}
//----------------------------------------------------------------------------
LRESULT P_Setup::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_IMG_FOLDER:
          SelectImgFolder();
          break;
        case IDC_BUTTON_SOUND:
          SelectSoundFolder(IDC_EDIT_SOUND, AUDIO_BASE_FOLDER);
          break;
        case IDC_BUTTON_SOUND_ALERT:
          SelectSoundFolder(IDC_EDIT_SOUND_ALERT, AUDIO_ALERT_FOLDER);
          break;

        case IDOK:
          BNClickedOk();
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_Setup::SelectImgFolder()
{
  TCHAR path[_MAX_PATH];
  getKeyPath(IMG_PATH, path);

  if(PChooseFolder(*this, path)) {
    SET_TEXT(IDC_EDIT_IMG_FOLDER, path);
  }
}
//----------------------------------------------------------------------------
LPCTSTR filterExtAudio[] = { _T(".mp3"), _T(".*"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterAudio =
  _T("%s (mp3)\0*.mp3\0")
  _T("%s (*.*)\0*.*\0");
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
static void makeFilterAudio(LPTSTR flt, size_t dim)
{
  _tcscpy_s(flt, dim, getStringOrDef(IDT_FILE_TRD, _T("Audio File")));
  _tcscat_s(flt, dim, _T(" (mp3)"));
  LPTSTR p = flt + _tcslen(flt) + 1;
  _tcscpy_s(p, dim - (p - flt), _T("*.mp3"));
  p += _tcslen(p) + 1;
  _tcscpy_s(p, dim - (p - flt), getStringOrDef(IDT_FILE_ALL, _T("Tutti i File")));
  p += _tcslen(p);
  _tcscpy_s(p, dim - (p - flt), _T(" (*.*)"));
  p += _tcslen(p) + 1;
  _tcscpy_s(p, dim - (p - flt), _T("*.*"));
  p += _tcslen(p) + 1;
  *p = 0;
}
//----------------------------------------------------------------------------
bool openFileAudio(HWND owner, LPTSTR file)
{
#if true
  return PChooseFolder(owner, file);
#else
  TCHAR flt[_MAX_PATH];
  makeFilterAudio(flt, SIZE_A(flt));

  infoOpenSave Info(filterExtAudio, flt, infoOpenSave::OPEN_F, lastIx, file);
  POpenSave open(owner);

  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
#endif
}
//----------------------------------------------------------------------------
void P_Setup::SelectSoundFolder(uint idc, LPCTSTR key)
{
  TCHAR t[_MAX_PATH];
  getKeyPath(key, t);
  if(openFileAudio(*this, t))
    SET_TEXT(idc, t);
}
