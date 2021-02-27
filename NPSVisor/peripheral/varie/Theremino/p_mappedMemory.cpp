//-------------- p_MappedMemory.cpp --------------------------------------
//----------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------
#include <stdlib.h>
//----------------------------------------------------------------------
#include "p_MappedMemory.h"
#include "p_util.h"
//----------------------------------------------------------------------
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
p_MappedMemory::p_MappedMemory() :
  Addr(0), Len(0), HandleMapped(0)
{ }
//------------------------------------------------------------
p_MappedMemory::~p_MappedMemory()
{
  P_close();
}
//------------------------------------------------------------
extern void DisplayErrorString(DWORD dwErr);
//------------------------------------------------------------
bool p_MappedMemory::P_open(LPCTSTR name, __int64 desiredSize)
{
  while(true) {
    DWORD low;
    DWORD high;
    i64ToDword(desiredSize, low, high);
    HandleMapped = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, high, low, name);
    if(!HandleMapped) {
      DisplayErrorString(GetLastError());
      break;
      }
    Len = desiredSize;
    Addr = MapViewOfFile(HandleMapped, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    return toBool(Addr);
    }
  P_close();
  return false;
}
//------------------------------------------------------------
bool p_MappedMemory::P_close()
{
  if(Addr) {
    UnmapViewOfFile(Addr);
    Addr = 0;
    }
  if(HandleMapped) {
    CloseHandle(HandleMapped);
    HandleMapped = 0;
    }

  Len = 0;
  return true;
}
//------------------------------------------------------------
bool p_MappedMemory::remapAt(__int64 pos, DWORD len)
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
  Addr = MapViewOfFile(HandleMapped, FILE_MAP_ALL_ACCESS, high, low, len);
  return toBool(Addr);
}
//------------------------------------------------------------
__int64 p_MappedMemory::getDim()
{
  return Len;
}
