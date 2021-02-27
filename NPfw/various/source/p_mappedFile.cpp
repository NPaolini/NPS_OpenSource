//-------------- P_MappedFile.cpp --------------------------------------
//----------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------
#include <stdlib.h>
//----------------------------------------------------------------------
#include "p_mappedfile.h"
#include "p_util.h"
//----------------------------------------------------------------------
#define F_READ  GENERIC_READ
#define F_WRITE (GENERIC_WRITE | F_READ)
//----------------------------------------------------------------------
#define S_4_WRITE  FILE_SHARE_READ
#define S_4_READ  (FILE_SHARE_WRITE | S_4_WRITE)

#define SHARE_(a)  (FILE_SHARE_WRITE | FILE_SHARE_READ)

#define OPEN_(a,b) CreateFile((a), (b), SHARE_(b), 0,\
              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)

#define CREAT_(a) CreateFile((a), F_WRITE,\
              SHARE_(F_WRITE), 0,\
              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)

#define CREAT_TRUNCATE(a) CreateFile((a), GENERIC_WRITE,\
              SHARE_(GENERIC_WRITE), 0,\
              TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)
//----------------------------------------------------------------------
#define CLOSE_ CloseHandle

#define F_CREAT 0
//------------------------------------------------------------
extern bool checkReserved(LPCTSTR filename, bool message = true);
//------------------------------------------------------------
static void i64ToDword(__int64 v, DWORD& low, DWORD& high)
{
   LARGE_INTEGER li;
   li.QuadPart = v;

   low  = li.LowPart;
   high = li.HighPart;
}
//------------------------------------------------------------
static void DwordToi64(__int64& v, DWORD low, DWORD high)
{
   LARGE_INTEGER li;
   li.LowPart = low;
   li.HighPart = high;
   v = li.QuadPart;
}
//------------------------------------------------------------
p_MappedFile::p_MappedFile(LPCTSTR n, ACC_MAPPED_FILE mode) : Mode(mode),
  Name(str_newdup(n)), Handle(INVALID_HANDLE_VALUE), Addr(0), NamedView(0),
  Len(0), HandleMapped(0)
{ }
//------------------------------------------------------------
p_MappedFile::~p_MappedFile()
{
  P_close();
}
//------------------------------------------------------------
extern void DisplayErrorString(DWORD dwErr);
//------------------------------------------------------------
bool p_MappedFile::P_open(__int64 desiredSize)
{
  if(checkReserved(Name))
    return false;
  ACC_MAPPED_FILE mode = (ACC_MAPPED_FILE)Mode;
  int acc;
  WIN32_FIND_DATA FindFileData;
  HANDLE h = FindFirstFile(Name, &FindFileData);
  if(h == INVALID_HANDLE_VALUE)
    acc = 1;
  else {
    acc = 0;
    FindClose(h);
    DwordToi64(Len, FindFileData.nFileSizeLow, FindFileData.nFileSizeHigh);
    }
  if(Mode == mP_CREAT || (acc && Mode != mP_READ_ONLY && Mode != mP_MEM_ONLY)) {// se creazione o non esiste
    mode = mP_CREAT;
    }
  else {              // il file esiste
    if(Mode != mP_READ_ONLY)
      acc = F_WRITE;
    else
      acc = F_READ;
   }
  while(true) {
    DWORD mapAcc;
    LPCTSTR nv = get_namedView();
    // solo lettura
    if(mP_READ_ONLY == Mode) {
      HandleMapped = OpenFileMapping(FILE_MAP_READ, false, nv);
      if(!HandleMapped)
        break;
      mapAcc = FILE_MAP_READ;
      }

    else {
      mapAcc = FILE_MAP_ALL_ACCESS;
      // prima creazione
      if(mode == mP_CREAT) {
        Handle = CREAT_(Name);
        if(INVALID_HANDLE_VALUE == Handle && !acc) {
        // se qualche applicazione ha aperto il file con share solo in lettura
        // non si riesce comunque ad aprirlo ed azzerarlo
          Handle = CREAT_TRUNCATE(Name);
          }
        Mode = mP_RW;
        }
      else {
        HandleMapped = OpenFileMapping(FILE_MAP_WRITE, false, nv);
        if(!HandleMapped && mP_MEM_ONLY == mode) {
          HandleMapped = OpenFileMapping(FILE_MAP_READ, false, nv);
          if(HandleMapped)
            mapAcc = FILE_MAP_READ;
          }
        if(!HandleMapped && mP_MEM_ONLY != mode)
          Handle = OPEN_(Name, acc);
        }
      if(Handle == INVALID_HANDLE_VALUE && !HandleMapped && mP_MEM_ONLY != mode)
        break;
      if(Handle != INVALID_HANDLE_VALUE || (mP_MEM_ONLY == mode && !HandleMapped)) {
        if(mP_MEM_ONLY != mode && !resize(desiredSize) && desiredSize <= Len)
          break;
        DWORD low;
        DWORD high;
        i64ToDword(desiredSize, low, high);
        HandleMapped = CreateFileMapping(Handle, 0, PAGE_READWRITE | SEC_COMMIT, high, low, nv);
        if(!HandleMapped) {
          DisplayErrorString(GetLastError());
          break;
          }
        if(ERROR_ALREADY_EXISTS == GetLastError() && mP_CREAT == Mode)
          break;
        }
      }

    Addr = MapViewOfFile(HandleMapped, mapAcc, 0, 0, 0);
    Mode = mapAcc;
    return toBool(Addr);
    }
  P_close();
  return false;
}
//------------------------------------------------------------
bool p_MappedFile::P_close()
{
  flush();
  if(Addr) {
    UnmapViewOfFile(Addr);
    Addr = 0;
    }
  if(HandleMapped) {
    CloseHandle(HandleMapped);
    HandleMapped = 0;
    }
  if(INVALID_HANDLE_VALUE != Handle) {
    CLOSE_(Handle);
    Handle = INVALID_HANDLE_VALUE;
    }
  delete []NamedView;
  delete []Name;
  NamedView = 0;
  Name = 0;

  Len = 0;
  return true;
}
//------------------------------------------------------------
bool p_MappedFile::remapAt(__int64 pos, DWORD len)
{
  if(Addr) {
    UnmapViewOfFile(Addr);
    Addr = 0;
    }
  if(!HandleMapped)
    return false;
  DWORD low;
  DWORD high;
  i64ToDword(pos, low, high);
  if(Len - pos < (__int64)len)
    len = (DWORD)(Len - pos);
  Addr = MapViewOfFile(HandleMapped, Mode, high, low, len);
  return toBool(Addr);
}
//------------------------------------------------------------
bool p_MappedFile::resize(__int64 new_dim)
{
  bool ret = true;
  DWORD low;
  DWORD high;
  i64ToDword(new_dim, low, high);

  LONG lHigh = high;
  DWORD result = SetFilePointer(Handle, low, &lHigh, SEEK_SET_);
  if(INVALID_SET_FILE_POINTER != result || NO_ERROR == GetLastError())
    ret = SetEndOfFile(Handle) != 0;
  else
    ret = false;

  if(ret)
    Len = new_dim;
  return ret;
}
//------------------------------------------------------------
__int64 p_MappedFile::getDim()
{
  return Len;
}
//------------------------------------------------------------
bool p_MappedFile::finalResize(__int64 new_dim)
{
  if(mP_READ_ONLY == Mode)
    return false;

  if(Addr) {
    UnmapViewOfFile(Addr);
    Addr = 0;
    }
  if(HandleMapped) {
    CloseHandle(HandleMapped);
    HandleMapped = 0;
    }
  resize(new_dim);
  TCHAR name[_MAX_PATH];
  _tcscpy_s(name, Name);
  P_close();
  Name = str_newdup(name);
  Mode = mP_RW;
  return P_open(new_dim);
}
//------------------------------------------------------------
bool p_MappedFile::flush()
{
  if(Addr)
    return toBool(FlushViewOfFile(Addr, 0));
  return false;
}
//-------------------------------------------------------------
inline void replaceChar(LPTSTR c)
{
  if(*c <= _T(' '))
    *c = _T('_');
  else {
    switch(*c) {
      case _T('\\'):
      case _T('/'):
      case _T(':'):
      case _T('.'):
        *c = _T('_');
        break;
      default:
        if(_T('A') <= *c && *c <= _T('Z'))
          *c += _T('a') - _T('A');
      }
    }
}
//-------------------------------------------------------------
LPCTSTR p_MappedFile::get_namedView()
{
  if(!NamedView) {
    NamedView = str_newdup(Name);
    LPTSTR p = NamedView;
    for(; *p; ++p)
      replaceChar(p);
    }
  return NamedView;
}
