//-------------- P_FILE.CPP -------------------------------------------
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include <stdlib.h>
//---------------------------------------------------------------------
#include "p_file.h"
#include "p_util.h"
//---------------------------------------------------------------------
#define F_READ  GENERIC_READ
#define F_WRITE (GENERIC_WRITE | F_READ)

#define S_4_WRITE  FILE_SHARE_READ
#define S_4_READ  (FILE_SHARE_WRITE | S_4_WRITE)

#define SHARE_(a)  ((a) & GENERIC_WRITE ? S_4_WRITE : (a) & GENERIC_READ ? S_4_READ : 0)

#if 0
  #define OPEN_(a,b) CreateFile((a), (b), SHARE_(b), 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)
  #define CREAT_(a) CreateFile((a), F_WRITE, SHARE_(F_WRITE), 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)
  #define CREAT_TRUNCATE(a) CreateFile((a), GENERIC_WRITE, SHARE_(GENERIC_WRITE), 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)
#else
  #define OPEN_(a,b) CreateFile((a), (b), SHARE_(b), 0, OPEN_EXISTING, Attr, 0)
  #define CREAT_(a) CreateFile((a), F_WRITE, SHARE_(F_WRITE), 0, CREATE_ALWAYS, Attr, 0)
  #define CREAT_TRUNCATE(a) CreateFile((a), GENERIC_WRITE, SHARE_(GENERIC_WRITE), 0, TRUNCATE_EXISTING, Attr, 0)
#endif

//---------------------------------------------------------------------
#define CLOSE_ CloseHandle
//---------------------------------------------------------------------
// occorre inserire un long TNum prima di chiamare le read e write
#define WRITE_(a,b,c) WriteFile(a,b,c,&TNum,0)
#define READ_(a,b,c) ReadFile(a,b,c,&TNum,0)
#define SEEK_(a,b,c) SetFilePointer(a,b,0,c)
//---------------------------------------------------------------------
#define F_CREAT 0
//---------------------------------------------------------------------
static TCHAR *Msg_Err[MAX_MSG + 1] = {
  _T("Nessun Errore"),
  _T("Gestore non allocato"),
  _T("File non aperto"),
  _T("Errore numero di user"),
  _T("Errore in lettura"),
  _T("Errore in scrittura"),
  _T("Errore di posizionamento"),
  _T("Errore indefinito")
  };
//------------------------------------------------------------
static void message_warn(LPCTSTR title, LPCTSTR msg)
{
  ::MessageBox(0, msg, title, MB_ICONWARNING);
}
//------------------------------------------------------------
bool checkReserved(LPCTSTR filename, bool message = true)
{
  TCHAR file[_MAX_FNAME];
  _tsplitpath_s(filename, 0, 0, 0, 0, file, SIZE_A(file), 0, 0);
  LPCTSTR reserved[] = { _T("COMx"), _T("LPTx"), _T("PRN"), _T("CON") };
  uint i;
  bool found = false;
  for(i = 0; i < 2; ++i) {
    TCHAR t[8];
    _tcscpy_s(t, reserved[i]);
    for(int j = 0; j < 9; ++j) {
      t[3] = _T('1') + j;
      if(!_tcsicmp(file, t)) {
        found = true;
        break;
        }
      }
    }
  if(!found)
    for(; i < SIZE_A(reserved); ++i)
      if(!_tcsicmp(file, reserved[i])) {
        found = true;
        break;
        }
  if(found) {
    if(message) {
      TCHAR msg[100];
      wsprintf(msg, _T("The filename [%s] is reserved for peripherals!"), file);
      message_warn(_T("Error in filename"), msg);
      }
    return true;
    }
  return false;
}
//------------------------------------------------------------
P_File::P_File(LPCTSTR n, ACC_FILE mode) : Mode(mode), Attr(FILE_ATTRIBUTE_NORMAL),
  Name(str_newdup(n)), Handle(INVALID_HANDLE_VALUE),
    Pos(0), Len(0), tError(0) { }
//------------------------------------------------------------
P_File::~P_File(void)
{
  delete []Name;
  if(INVALID_HANDLE_VALUE != Handle)
    CLOSE_(Handle);
}
//------------------------------------------------------------
int P_File::Error(int type)
{
  tError = type;
  message_warn(Msg_Err[type],Name);
  return false;
}
//------------------------------------------------------------
bool P_File::P_exist()
{
  return P_exist(Name);
}
//------------------------------------------------------------
bool P_File::P_exist(LPCTSTR filename)
{
  if(checkReserved(filename, false))
    return false;
  WIN32_FIND_DATA FindFileData;
  HANDLE h = FindFirstFile(filename, &FindFileData);
  if(h == INVALID_HANDLE_VALUE)
    return false;
  FindClose(h);
  return true;

}
//------------------------------------------------------------
#if 1
//------------------------------------------------------------
bool P_File::P_open(HANDLE opened, bool append)
{
  if(!opened && checkReserved(Name))
    return false;
  ACC_FILE mode = Mode;
  int acc = 0;
  if(Mode != P_CREAT) { // il file deve esistere
    if(Mode == P_RW)
      acc = F_WRITE;
    else
      acc = F_READ;
    }
  Pos = 0;
  if(mode == P_CREAT) {
    if(opened)
      Handle = opened;
    else
      Handle = CREAT_(Name);
    if(INVALID_HANDLE_VALUE == Handle) {
      // se qualche applicazione ha aperto il file con share solo in lettura
      // non si riesce comunque ad aprirlo ed azzerarlo
      Handle = CREAT_TRUNCATE(Name);
      }
    }
  else {
    if(opened)
      Handle = opened;
    else {
      Handle = OPEN_(Name, acc);
      // se non riesce ad aprirlo forse non esiste, e se non in sola lettura lo crea
      if(INVALID_HANDLE_VALUE == Handle && Mode != P_READ_ONLY)
        Handle = CREAT_(Name);
      }
    }
  if(Handle != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER li;
    if(GetFileSizeEx(Handle, &li))
      Len = li.QuadPart;
    }
  if(opened)
    if(append)
      P_seek(0, SEEK_END_);
    else
      P_seek(0);
  return Handle != INVALID_HANDLE_VALUE;
}
//------------------------------------------------------------
#else
//------------------------------------------------------------
bool P_File::P_open(HANDLE opened)
{
  if(!opened && checkReserved(Name))
    return false;
  ACC_FILE mode = Mode;
  int acc;
  WIN32_FIND_DATA FindFileData;
  HANDLE h = FindFirstFile(Name, &FindFileData);
  bool found = false;
  if(h == INVALID_HANDLE_VALUE)
    acc = 1;
  else {
    acc = 0;
    found = true;
    FindClose(h);
    }
  if(Mode == P_CREAT || (acc && Mode != P_READ_ONLY)) {// se creazione o non esiste
    mode = P_CREAT;
    }
  else {              // il file esiste
    if(Mode == P_RW)
      acc = F_WRITE;
    else
      acc = F_READ;
    if(found) {
      LARGE_INTEGER li;
      li.LowPart = FindFileData.nFileSizeLow;
      li.HighPart = FindFileData.nFileSizeHigh;
      Len = li.QuadPart;
      }
    }
  Pos=0;
  if(mode == P_CREAT) {
    if(opened)
      Handle = opened;
    else
      Handle = CREAT_(Name);
    if(INVALID_HANDLE_VALUE == Handle && !acc) {
      // se qualche applicazione ha aperto il file con share solo in lettura
      // non si riesce comunque ad aprirlo ed azzerarlo
      Handle = CREAT_TRUNCATE(Name);
      }
//    Mode = P_RW;
    }
  else
    if(opened)
      Handle = opened;
    else
      Handle = OPEN_(Name, acc);

  return Handle != INVALID_HANDLE_VALUE;
}
#endif
//------------------------------------------------------------
bool P_File::P_close()
{
  if(Handle != INVALID_HANDLE_VALUE)
    CLOSE_(Handle);
  Handle = INVALID_HANDLE_VALUE;
  return true;
}
//------------------------------------------------------------
lUDimF P_File::P_read(tFILE *buff, lUDimF len)
{
  if(Handle == INVALID_HANDLE_VALUE)
    return Error(NOT_OPEN);
  lUDimF TNum;
  if(READ_(Handle, buff, len)) {
    Pos += TNum;
    if(Pos > Len)
      Len = Pos;
    return TNum;
    }
  return 0;
}
//------------------------------------------------------------
lUDimF P_File::P_write(const tFILE *buff, lUDimF len)
{
  if(Handle == INVALID_HANDLE_VALUE)
    return Error(NOT_OPEN);
  lUDimF TNum;
  if(WRITE_(Handle, buff, len)) {
    Pos += TNum;
    if(Pos > Len)
      Len = Pos;
    return TNum;
    }
  return 0;
}
//------------------------------------------------------------
bool P_File::P_seek(DimF offset, int fromwhere)
{
  if(Handle == INVALID_HANDLE_VALUE)
    return toBool(Error(NOT_OPEN));

  switch(fromwhere) {
    case SEEK_SET_:
      if(offset > static_cast<DimF>(Len)) {
        if(0 == chgsize(offset))
          Len = offset;
        else
          offset = Len;
        }
      break;
    case SEEK_CUR_:
      if(offset < 0 && -offset > static_cast<DimF>(Pos))
        offset = -static_cast<DimF>(Pos);
      else if(offset > static_cast<DimF>(Len - Pos))
        offset = Len - Pos;
      break;
    case SEEK_END_:
      if(-offset > static_cast<DimF>(Len))
        offset = -static_cast<DimF>(Len);
      break;
    }

  LARGE_INTEGER li;
  li.QuadPart = offset;

  li.LowPart = SetFilePointer( Handle, li.LowPart, &li.HighPart, fromwhere);
  offset = li.QuadPart;

  if(offset >= 0) {
    Pos = offset;
    if(Pos > Len)
      Len = Pos;
    return true;
    }
  return toBool(Error(NOT_SEEK));
}
//------------------------------------------------------------
#define MAX_SIZE_TRANSFER (4096L * 16)
bool P_File::chgsize(UDimF new_dim, LPCTSTR back_ext)
{
  bool ret = true;
  if(back_ext) {
    TCHAR tmp[_MAX_PATH];
    _tcscpy_s(tmp, Name);
    reback(back_ext);           // rinomina
    HANDLE p = CREAT_(tmp); // ricrea il file con nome originario
    if(p != INVALID_HANDLE_VALUE) {
      if(new_dim) {
        LPBYTE t = new BYTE[MAX_SIZE_TRANSFER];     // alloca spazio
        SEEK_(Handle, 0L, SEEK_SET_);      // resetta puntatore
        lUDimF TNum;
        UDimF tmpDim = new_dim;
        while(tmpDim) {
          lUDimF dim = (lUDimF)(tmpDim < MAX_SIZE_TRANSFER ? tmpDim : MAX_SIZE_TRANSFER);
          READ_(Handle, t, dim);          // legge dal back
          WRITE_(p, t, dim);              // scrive nel nuovo
          tmpDim -= dim;
          }
        delete []t;
        }
      CLOSE_(Handle);
      Handle = p;
      }
    delete []Name;
    Name = str_newdup(tmp);
    }
  else {
    LARGE_INTEGER li;
    li.QuadPart = new_dim;

    li.LowPart = SetFilePointer( Handle, li.LowPart, &li.HighPart, SEEK_SET_);

    if(li.QuadPart == new_dim)
      ret = SetEndOfFile(Handle) != 0;
    else
      ret = false;
    }
  if(ret) {
    Len = new_dim;
    if(Pos > Len)
      Pos = Len;
    }
  return ret;
}
//------------------------------------------------------------
void P_File::chgExt(LPTSTR filename, LPCTSTR newExt)
{
  TCHAR disk[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR file[_MAX_FNAME];
  _tsplitpath_s(filename, disk, SIZE_A(disk), dir, SIZE_A(dir), file, SIZE_A(file), 0, 0);
  _tmakepath_s(filename, _MAX_PATH, disk, dir, file, newExt);
}
//------------------------------------------------------------
bool P_File::reback(LPCTSTR ext)
{
  if(!ext)
    ext = _T(".BAC");
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, Name);
  chgExt(tmp, ext);
  return P_rename(tmp);
}
//------------------------------------------------------------
bool P_File::unback(LPCTSTR ext)
{
  if(!ext)
    ext = _T(".BAC");
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, Name);
  TCHAR tName[_MAX_PATH];
  _tcscpy_s(tName, Name);
  chgExt(tName, ext);
  if(_tcslen(tName) > _tcslen(Name)) {
    delete []Name;
    Name = str_newdup(tName);
    }
  else
    _tcscpy_s(Name, _tcslen(Name) + 1, tName);
  return P_rename(tmp);
}
//------------------------------------------------------------
bool P_File::appendback()
{
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, Name);
  _tcscat_s(tmp, _T(".bak"));
  return P_rename(tmp);
}
//------------------------------------------------------------
bool P_File::unappendback()
{
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, Name);
  size_t len = _tcslen(tmp);
  if(len > 4 && !_tcsicmp(tmp + len - 4, _T(".bak"))) {
    tmp[len - 4] = 0;
    return P_rename(tmp);
    }
  return false;
}
//------------------------------------------------------------
bool P_File::P_chgName(LPCTSTR new_name)
{
  delete []Name;
  Name = str_newdup(new_name);
  if(INVALID_HANDLE_VALUE == Handle || !Handle)
    return true;

  P_close();
  ACC_FILE oldMode = Mode;
  if(P_CREAT == Mode)
    Mode = P_RW;
  bool result = P_open();
  Mode = oldMode;
  return result;
}
//------------------------------------------------------------
bool P_File::P_performRename(LPCTSTR new_name)
{
  SetFileAttributes(new_name, FILE_ATTRIBUTE_NORMAL);
  DeleteFile(new_name);          // cancella l'eventuale file esistente
  bool result = toBool(MoveFile(Name, new_name));                // rinomina
  size_t len = _tcslen(Name);
  if(len < _tcslen(new_name)) {
    delete []Name;
    len = _tcslen(new_name);
    Name = new TCHAR[len + 1];
    }
  _tcscpy_s(Name, len + 1, new_name);
  return result;
}
//------------------------------------------------------------
bool P_File::P_rename(LPCTSTR new_name)
{
  if(INVALID_HANDLE_VALUE == Handle || !Handle)
    return P_performRename(new_name);

  P_close();
  bool result = P_performRename(new_name);
  ACC_FILE oldMode = Mode;
  if(P_CREAT == Mode)
    Mode = P_RW;
  P_open();
  Mode = oldMode;
  return result;
}
//------------------------------------------------------------
bool P_File::flush()
{
  if(Handle != INVALID_HANDLE_VALUE)
    return toBool(FlushFileBuffers(Handle));
  return false;
}
//-------------------------------------------------------------
lUDimF P_File::P_writeToAnsi(LPCSTR b, int len)
{
  if(!b)
    return 0;
  int curr = (int)strlen(b);
  if(-1 == len)
    len = curr;
  else
    len = min(len, curr);
  return P_write(b, len);
}
//-------------------------------------------------------------
lUDimF P_File::P_writeToAnsi(LPCWSTR b, int len)
{
  if(!b)
    return 0;
  int currlen = WideCharToMultiByte(CP_ACP, 0, b, len, 0, 0, 0, 0);
  LPSTR p = new char[currlen];
  WideCharToMultiByte(CP_ACP, 0, b, len, p, currlen, 0, 0);
  lUDimF result = P_write(p, (lUDimF)strlen(p));
  delete []p;
  return result;
}
//-------------------------------------------------------------
lUDimF P_File::P_writeToUnicode(LPCSTR b, int len)
{
  if(!b)
    return 0;
  int currlen = MultiByteToWideChar(CP_ACP, 0, b, len, 0, 0);
  LPWSTR p = new wchar_t[currlen];
  MultiByteToWideChar(CP_ACP, 0, b, len, p, currlen);
  lUDimF result = P_writeToUnicode(p, currlen);
  delete []p;
  return result;
}
//-------------------------------------------------------------
#define BOM_UTF_16_LE 0xfeff
//-------------------------------------------------------------
lUDimF P_File::P_writeToUnicode(LPCWSTR b, int len)
{
  if(!b)
    return 0;
  int curr = (int)wcslen(b);
  if(-1 == len)
    len = curr;
  else
    len = min(len, curr);
  if(!get_len()) {
    WORD v = BOM_UTF_16_LE;
    P_write(&v, sizeof(v));
    }
  lUDimF result = P_write(b, len * sizeof(*b));
  return result;
}
//------------------------------------------------------------
bool P_File::getData(FILETIME& lasttime)
{
  if(Handle != INVALID_HANDLE_VALUE) {
    FILETIME A;
    FILETIME C;
    return toBool(GetFileTime(Handle, &A, &C, &lasttime));
    }
  return false;
}
//------------------------------------------------------------
bool P_File::setData(const FILETIME& newtime)
{
  if(Handle != INVALID_HANDLE_VALUE)
    return toBool(SetFileTime(Handle, &newtime, &newtime, &newtime));
  return false;
}
//--------------------------------------------------------------------
void GetModuleDirName(size_t lenBuff, LPTSTR target)
{
  target[0] = 0;
  GetModuleFileName(0, target, (DWORD)lenBuff);
  GetLongPathName(target, target, (DWORD)lenBuff);
  int len = (int)_tcslen(target);
  for(--len; len > 0; --len) {
    if(_T('\\') == target[len]) {
      target[len] = 0;
      break;
      }
    }
}
//----------------------------------------------------------------------------
LPWSTR dirName(LPWSTR path)
{
  size_t l = wcslen(path);
  for(int i = l - 1; i > 0; --i)
    if(L'\\' == path[i]) {
      path[i] = 0;
      break;
      }
  return path;
}
//----------------------------------------------------------------------------
LPSTR dirName(LPSTR path)
{
  size_t l = strlen(path);
  for(int i = l - 1; i > 0; --i)
    if('\\' == path[i]) {
      path[i] = 0;
      break;
      }
  return path;
}
//----------------------------------------------------------------------------
