//---------- P_backup.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_backup.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "p_param.h"
#include "p_param_v.h"
#include "p_crypt.h"
#include "p_file.h"
#include "p_util.h"
#include "pOpensave.h"
//----------------------------------------------------------------------------
static TCHAR gPsw[64];
static TCHAR gFile[_MAX_PATH];
//----------------------------------------------------------------------------
P_backup::P_backup(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_SEND, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDOK, Bmp[0]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[1]);
}
//----------------------------------------------------------------------------
P_backup::~P_backup()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
bool P_backup::create()
{
  if(!baseClass::create())
    return false;
  Conn->passThrough(cReset, 0);
  SET_CHECK(ID_RADIO_BACKUP);
  enable();
  SET_TEXT(IDC_EDIT_PSW, gPsw);
  SET_TEXT(IDC_EDIT_FILE, gFile);
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_backup::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          BNClickedOk();
          break;
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        case IDC_EDIT_PSW:
        case IDC_EDIT_FILE:
          if(EN_CHANGE == HIWORD(wParam))
            enable();
          break;
        case IDC_BUTTON_SEARCH:
          open_File();
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_backup::enable()
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PSW, t);
  bool ok = toBool(*t);
  GET_TEXT(IDC_EDIT_FILE, t);
  ok &= toBool(*t);
  ENABLE(IDOK, ok);
}
//----------------------------------------------------------------------------
void P_backup::BNClickedOk()
{
  GET_TEXT(IDC_EDIT_PSW, gPsw);
  GET_TEXT(IDC_EDIT_FILE, gFile);
  if(IS_CHECKED(ID_RADIO_BACKUP))
    saveBackup();
  else
    sendRestore();
}
//----------------------------------------------------------------------------
struct backupPacket
{
  char psw[16];
  int dim1;
  int dim2;
  backupPacket() : dim1(0), dim2(0) { ZeroMemory(psw, sizeof(psw)); }
};
//----------------------------------------------------------------------------
#define MATCH_RESTORE "r=restore:"
#define DIM_MATCH_RESTORE (strlen(MATCH_RESTORE))
static LPCTSTR MSG_SHOW_SEND_FILE = _T("Sending restore ...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
//----------------------------------------------------------------------------
void P_backup::sendRestore()
{
  TCHAR file[_MAX_PATH];
  GET_TEXT(IDC_EDIT_FILE, file);
  char psw[32];
  GetDlgItemTextA(*this, IDC_EDIT_PSW, psw, SIZE_A(psw));
  infoFileCrCopyOne ifcc = {
    0,
    0,
    0,
    (LPCBYTE)psw,
    strlen(psw),
    1,
  (LPCBYTE)"NPD",
    3
    };
  bool success = decryptFromFile(file, ifcc);
  if(!success) {
    return;
    }

  do {
    backupPacket* pbp = (backupPacket*)ifcc.buff;
    if(strcmp(psw, pbp->psw)) {
      MessageBox(*this, _T("Invalid password"), _T("Restore"), MB_OK  | MB_ICONERROR);
      break;
      }
    LPSTR p = (LPSTR)ifcc.buff + sizeof(backupPacket);
    char tmp[64];
    wsprintfA(tmp, "cmd=restore:%d,%d#", pbp->dim1, pbp->dim2);
    progressBar bar(this, MSG_SHOW_SEND_FILE, pbp->dim1 + pbp->dim2 + strlen(tmp), BARCOLOR_CHAR);
    bar.create();
    UpdateWindow(bar);
    connSend cs(Conn, &bar);
    if(!cs.send(tmp, strlen(tmp))) {
      MessageBox(*this, _T("Aborted"), _T("Restore"), MB_OK  | MB_ICONERROR);
      break;
      }
    if(!connFindMatch(Conn, ACK_, strlen(ACK_)))
      break;

    if(!cs.send(p, pbp->dim1)) {
      MessageBox(*this, _T("Aborted"), _T("Restore"), MB_OK  | MB_ICONERROR);
      break;
      }
    if(!connFindMatch(Conn, ACK_, strlen(ACK_)))
      break;

    if(!cs.send(p + pbp->dim1, pbp->dim2)) {
      MessageBox(*this, _T("Aborted"), _T("Restore"), MB_OK  | MB_ICONERROR);
      break;
      }
    char buff[256];
    DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
    buff[readed] = 0;
    int pos = checkPos(buff, MATCH_RESTORE, DIM_MATCH_RESTORE);
    if(pos < 0)
      break;
    bool success = strstr(buff + pos, "Ok");
    MessageBoxA(*this, buff + pos, "Restore", MB_OK  | (success ? MB_ICONINFORMATION : MB_ICONERROR));
  } while(false);
  delete []ifcc.buff;
}
//----------------------------------------------------------------------------
#define MATCH_BACKUP "r=backup:"
#define DIM_MATCH_BACKUP (strlen(MATCH_BACKUP))
//----------------------------------------------------------------------------
void P_backup::saveBackup()
{
  char buff[4096] = "cmd=backup:#";
  backupPacket bp;
  GetDlgItemTextA(*this, IDC_EDIT_PSW, bp.psw, SIZE_A(bp.psw));

  connSend cs(Conn);
  cs.send(buff, strlen(buff));

  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;

  int pos = checkPos(buff, MATCH_BACKUP, DIM_MATCH_BACKUP);
  if(pos < 0)
    return;
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if(n != 2)
    return;
  bp.dim1 = atoi(&target[0]);
  bp.dim2 = atoi(&target[1]);
  if(bp.dim1 + bp.dim2 >= sizeof(buff))
    return;

  memcpy_s(buff, sizeof(buff), &bp, sizeof(bp));
  Conn->read_string((LPBYTE)buff + sizeof(bp), bp.dim1 + bp.dim2);

  TCHAR file[_MAX_PATH];
  GET_TEXT(IDC_EDIT_FILE, file);
  infoFileCrCopyOne ifcc = {
    (LPBYTE)buff,
    sizeof(bp) + bp.dim1 + bp.dim2,
    0,
  (LPCBYTE)bp.psw,
    strlen(bp.psw),
    1,
  (LPCBYTE)"NPD",
    3
    };
  bool success = cryptToFile(file, ifcc);
  MessageBox(*this, success ? _T("Ok") : _T("Failed"), _T("Backup"), MB_OK | (success ? MB_ICONINFORMATION : MB_ICONERROR));
}
//----------------------------------------------------------------------------
static LPCTSTR filterExt[] = { _T(".epk"), _T(""), 0 };
//----------------------------------------------------------------------------
static LPCTSTR filterOpen =
  _T("File Epk (*.epk)\0*.epk\0")
  _T("All Files (*.*)\0*.*\0");

//----------------------------------------------------------------------------
static LPCTSTR filterSaveAs =
   _T("File Epk (*.epk)\0")
    _T("*.epk\0")
  _T("All Files (*.*)\0*.*\0");
//----------------------------------------------------------------------------
void P_backup::open_File()
{
  POpenSave open(*this);
  infoOpenSave::whichAction act = infoOpenSave::OPEN_F;
  if(IS_CHECKED(ID_RADIO_BACKUP))
    act = infoOpenSave::SAVE_AS;

  infoOpenSave Info(filterExt, filterOpen, act, 0);
  if(open.run(Info))
    SET_TEXT(IDC_EDIT_FILE, open.getFile());
}
