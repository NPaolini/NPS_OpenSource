//---------- P_Site.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Site.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "p_param.h"
//#include "p_param_v.h"
#include "p_util.h"
#include "p_file.h"
#include "pOpensave.h"
//----------------------------------------------------------------------------
#define SIZE_LINE_LB (MAX_NAME_DIM + MAX_NAME_EXT + MAX_FILE_DIM + 4)
#define BKG_COLOR_EDIT RGB(240, 240, 220)

#define BKG_COLOR_SEL RGB(255, 255, 220)
#define FG_COLOR_SEL RGB(0x3f, 0x0, 0x0)

#define BKG_COLOR RGB(255, 255, 255)
#define FG_COLOR RGB(0x0, 0x0, 0x0)
//----------------------------------------------------------------------------
P_Site::P_Site(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  Lb = new PListBox(this, IDC_LIST_FILES);

  int t[] = {  MAX_NAME_DIM, MAX_NAME_EXT, MAX_FILE_DIM };
  Lb->SetTabStop(SIZE_A(t), t, 0);
  Lb->setAlign(2, PListBox::aRight);

  Lb->SetColorSel(FG_COLOR_SEL, BKG_COLOR_SEL);
  Lb->SetColor(FG_COLOR, BKG_COLOR);

  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_SEND, IDB_BITMAP_LOAD, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDC_BUTTON_UPLOAD, Bmp[0]);
  new POwnBtnImageStd(this, IDC_BUTTON_DOWNLOAD, Bmp[0]);
  new POwnBtnImageStd(this, IDC_BUTTON_RELOAD, Bmp[1]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[2]);
}
//----------------------------------------------------------------------------
P_Site::~P_Site()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
void P_Site::loadCurr()
{
  SendMessage(*Lb, LB_RESETCONTENT, 0, 0);
  char buff[4096] = "cmd=site_list:#";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff) - 1);
  buff[readed] = 0;
  parseBuff(buff);
}
//----------------------------------------------------------------------------
bool P_Site::create()
{
  if(!baseClass::create())
    return false;

  Conn->passThrough(cReset, 0);
  Lb->rescale(1, 2, 1.2);

  loadCurr();
  return true;
}
//----------------------------------------------------------------------------
#define SET_TEXT_A(idc, txt) SetDlgItemTextA(*this, idc, txt)
//----------------------------------------------------------------------------
LRESULT P_Site::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        case IDC_BUTTON_UPLOAD_FILE:
          openFilename();
          break;
        case IDC_BUTTON_DOWNLOAD_PATH:
          openPath();
          break;

        case IDC_BUTTON_UPLOAD:
          upload();
          break;
        case IDC_BUTTON_DOWNLOAD:
          download();
          break;
        case IDC_BUTTON_RELOAD:
          loadCurr();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int myCmp(LPCSTR c1, LPCSTR c2, size_t len)
{
  for(size_t i = 0; i < len; ++i) {
    int diff = c1[i] - c2[i];
    if(diff)
      return diff;
    }
  return 0;
}
//----------------------------------------------------------------------------
#define MATCH_DOWNLOAD_FILE "r=get_site_file:"
#define DIM_MATCH_DOWNLOAD_FILE (strlen(MATCH_DOWNLOAD_FILE))
//----------------------------------------------------------------------------
bool P_Site::retrieveFile(progressBar& bar, infoFile& iF)
{
  Sleep(100);
  Conn->passThrough(cReset, 0);
  char buff[512] = "cmd=get_site_file:";
  strcat_s(buff, iF.name);
  strcat_s(buff, ".");
  strcat_s(buff, iF.ext);
  strcat_s(buff, ",#");
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff) - 1);
  buff[readed] = 0;

  int pos = checkPos(buff, MATCH_DOWNLOAD_FILE, DIM_MATCH_DOWNLOAD_FILE);
  if (pos < 0) {
    return false;
    }
  int pos2 = strlen(iF.name);
  if(myCmp(buff + pos, iF.name, pos2))
    return false;

  if(myCmp(buff + pos + pos2 + 1, iF.ext, strlen(iF.ext)))
    return false;
  LPSTR p = buff + pos + pos2 + 1 + strlen(iF.ext) + 1;
  size_t dim = atoi(p);
  if(dim != iF.dim)
    return false;
  wsprintfA(buff, "%s.%s", iF.name, iF.ext);

  LPTSTR tmp = (LPTSTR)(buff + strlen(buff) + 1);
  copyStrZ(tmp, buff);
  P_File pf(tmp, P_CREAT);
  if(!pf.P_open())
    return false;
  do {
    size_t len = min(dim, sizeof(buff));
    len = Conn->read_string((LPBYTE)buff, len);
    pf.P_write(buff, len);
    dim -= len;
    if(!bar.addCount(len))
      return false;
    while(true) {
      if(!getAppl()->pumpMessages())
      break;
      }
    } while(dim > 0);
  return true;
}
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_DOWNLOAD_FILE = _T("Downloading File ...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
//----------------------------------------------------------------------------
void P_Site::download()
{
  int sel = SendMessage(*Lb, LB_GETSELCOUNT, 0, 0);
  if(LB_ERR == sel || !sel)
    return;
  int* select = new int[sel];
  SendMessage(*Lb, LB_GETSELITEMS, sel, (LPARAM)select);
  infoFile* iF = new infoFile[sel];
  long dim = 0;
  for(int i = 0; i < sel; ++i) {
    getFromLb(iF[i], Lb,select[i]);
    dim += iF[i].dim;
    }
  progressBar bar(this, MSG_SHOW_DOWNLOAD_FILE, dim, BARCOLOR_CHAR);
  bar.create();
  UpdateWindow(bar);
  TCHAR currPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, currPath);
  do {
    TCHAR path[_MAX_PATH];
    GET_TEXT(IDC_EDIT_DOWNLOAD, path);
    SetCurrentDirectory(path);
    }  while(false);
  for(int i = 0; i < sel; ++i) {
    if(!retrieveFile(bar, iF[i]))
      if(IDNO == MessageBoxA(*this, "Unable to retrive file, Continue anyway?", iF[i].name, MB_YESNO | MB_ICONSTOP))
        break;
    }
  delete []iF;
  delete []select;
  SetCurrentDirectory(currPath);
}
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_UPLOAD_FILE = _T("Uploading File ...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
//----------------------------------------------------------------------------
#define MATCH_UPLOAD_FILE "r=set_site_file:"
#define DIM_MATCH_UPLOAD_FILE (strlen(MATCH_UPLOAD_FILE))
//----------------------------------------------------------------------------
void P_Site::upload()
{
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_UPLOAD, path);
  TCHAR ext[_MAX_EXT];
  TCHAR file[_MAX_FNAME];
  _tsplitpath_s(path, 0, 0, 0, 0, file, SIZE_A(file), ext, SIZE_A(ext));
  if(_tcslen(file) > 8 || _tcslen(ext) > 4) {
    MessageBox(*this, _T("Only 8.3 file"), _T("Not Valid!"), MB_OK | MB_ICONSTOP);
    return;
    }
  P_File pf(path, P_READ_ONLY);
  if(!pf.P_open()) {
    MessageBox(*this, _T("Unable to open file"), _T("Error!"), MB_OK | MB_ICONSTOP);
    return;
    }
  size_t dim = pf.get_len();
  progressBar bar(this, MSG_SHOW_UPLOAD_FILE, dim, BARCOLOR_CHAR);
  bar.create();
  UpdateWindow(bar);

  Conn->passThrough(cReset, 0);
  char buff[1024] = "cmd=set_site_file:";
  LPSTR p = buff + strlen(buff);
  copyStrZ(p, file);
  p += strlen(p);
  copyStrZ(p, ext);
  p += strlen(p);
  wsprintfA(p, ",%d#", dim);

  connSend cs(Conn, &bar);
  cs.send(buff, strlen(buff));
  if (connFindMatch(Conn, ACK_, strlen(ACK_))) {
    do {
      size_t len = min(dim, sizeof(buff));
      len = pf.P_read(buff, len);
      if(!cs.sendWaitAck(buff, len))
        break;
      dim -= len;
      } while(dim > 0);
    }
  if(!dim) {
    DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff) - 1);
    buff[readed] = 0;
    }
  int pos = checkPos(buff, MATCH_UPLOAD_FILE, DIM_MATCH_UPLOAD_FILE);
  MessageBoxA(*this, pos >= 0 ? buff + pos : "No responce", "Upload file", MB_OK | MB_ICONINFORMATION);
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static LPSTR get_line(LPSTR buff, size_t len)
{
  if((int)len < 0)
    return 0;
  size_t i;
  LPSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == '\r' || U_(*p) == '\n')
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != '\r' && U_(*p) != '\n')
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
#define MATCH_SITE "r=site_list:"
#define DIM_MATCH_SITE (strlen(MATCH_SITE))
//----------------------------------------------------------------------------
void P_Site::parseBuff(LPSTR buff)
{
  int pos = checkPos(buff, MATCH_SITE, DIM_MATCH_SITE);
  if (pos < 0) {
    return;
    }
  size_t offs = 0;
  LPSTR p = buff + pos;
  while(*p) {
    if (*p > ' ')
      break;
    ++p;
    }
  size_t dim = strlen(p);
  fileList FL;
  do {
    LPSTR next = get_line(p, dim - offs);
    size_t len = next ? next - p : dim - offs;
    offs += len;

    parseRow(FL, p);
    p = next;
    } while(p);
  fillLB(FL);
}
//----------------------------------------------------------------------------
static int fillWithTab(LPTSTR target, LPCSTR source, int maxLen)
{
  int i;
  for(i = 0; i < maxLen; ++i) {
    if(!source[i])
      break;
    target[i] = source[i];
    }
  int len = i + 1;
  for(; i < maxLen; ++i)
    target[i] = _T(' ');

  target[i] = _T('\t');
  return len;
}
//----------------------------------------------------------------------------
static int fillInfoFile(LPSTR target, LPCSTR source, int maxLen, char stop)
{
  int i;
  for(i = 0; i < maxLen; ++i) {
    if(!source[i] || stop == source[i])
      break;
    target[i] = source[i];
    }
  int len = i + 1;
  for(; i < maxLen; ++i)
    target[i] = 0;

  target[i] = 0;
  return len;
}
//----------------------------------------------------------------------------
void P_Site::fillLB(fileList& fl)
{
  if(!fl.setFirst())
    return;
  TCHAR t[SIZE_LINE_LB];
  do {
    infoFile* _if = fl.getCurrData();
    LPTSTR t2 = t;

    fillWithTab(t2, _if->name, MAX_NAME_DIM);
    t2 += MAX_NAME_DIM + 1;

    fillWithTab(t2, _if->ext, MAX_NAME_EXT);
    t2 += MAX_NAME_EXT + 1;

    char dim[16];
    wsprintfA(dim, "%d", _if->dim);
    fillWithTab(t2, dim, MAX_FILE_DIM);
    t2 += MAX_FILE_DIM + 1;
    *t2 = 0;

    SendMessage(*Lb, LB_ADDSTRING, 0, (LPARAM) t);
    } while(fl.setNext());
}
//----------------------------------------------------------------------------
void P_Site::parseRow(fileList& fl, LPSTR row)
{
  infoFile iF;
  int len = fillInfoFile(iF.name, row, MAX_NAME_DIM, '.');
  len += fillInfoFile(iF.ext, row + len, MAX_NAME_EXT, ',');
  iF.dim = atoi(row + len);
  fl.addData(iF);
}
//----------------------------------------------------------------------------
static void trimWithTab(LPSTR target, LPCTSTR source, int maxLen)
{
  int i = maxLen;
  for(--i; i >= 0; --i)
    if(source[i] > _T(' '))
      break;
  target[i + 1] = 0;

  for(; i >= 0; --i)
    target[i] = source[i];
}
//----------------------------------------------------------------------------
int P_Site::getFromLb(infoFile& iF, PListBox* lb, int sel)
{
  if(sel < 0)
    sel = SendMessage(*lb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return sel;

  TCHAR t[SIZE_LINE_LB];
  if(LB_ERR == SendMessage(*lb, LB_GETTEXT, sel, (LPARAM)t))
    return -1;

  LPTSTR t2 = t;
  trimWithTab(iF.name, t2, MAX_NAME_DIM);
  t2 += MAX_NAME_DIM + 1;
  trimWithTab(iF.ext, t2, MAX_NAME_EXT);
  t2 += MAX_NAME_EXT + 1;
  iF.dim = _ttoi(t2);
  return sel;
}
//----------------------------------------------------------------------------
void P_Site::openPath()
{
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_DOWNLOAD, path);
  if(PChooseFolder(*this, path, _T("Download folder")))
    SET_TEXT(IDC_EDIT_DOWNLOAD, path);
}
//----------------------------------------------------------------------------
static LPCTSTR filterExt[] = { _T(".*"), _T(""), 0 };
//----------------------------------------------------------------------------
static LPCTSTR filterOpen =
  _T("All Files (*.*)\0*.*\0");
//----------------------------------------------------------------------------
void P_Site::openFilename()
{
  POpenSave open(*this);
  infoOpenSave::whichAction act = infoOpenSave::OPEN_F;

  infoOpenSave Info(filterExt, filterOpen, act, 0);
  if (open.run(Info))
    SET_TEXT(IDC_EDIT_UPLOAD, open.getFile());
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int fileList::Cmp(const TreeData toCheck, const TreeData current) const
{
  infoFile* check = reinterpret_cast<infoFile*>(toCheck);
  infoFile* curr = reinterpret_cast<infoFile*>(current);
  int diff = _stricmp(check->ext, curr->ext);
  if(diff)
    return diff;
  return _stricmp(check->name, curr->name);
}
//----------------------------------------------------------------------------
const infoFile* fileList::getCurrData() const
{
  return reinterpret_cast<infoFile*>(getCurr());
}
//----------------------------------------------------------------------------
infoFile* fileList::getCurrData()
{
  return reinterpret_cast<infoFile*>(getCurr());
}
//----------------------------------------------------------------------------
bool fileList::addData(const infoFile& iF)
{
  infoFile* pw = new infoFile(iF);
  if(!Add(pw)) {
    delete pw;
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------
bool fileList::addData(infoFile* iF)
{
  if(!Add(iF)) {
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------
