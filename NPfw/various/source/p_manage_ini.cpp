//------- p_manage_ini.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_manage_ini.h"

#define USE_UNICODE_4_WRITE_STRING
#include "p_file.h"
#include "p_param_v.h"
//----------------------------------------------------------------------------
void ini_param::destroy() { safeDeleteA(name); safeDeleteA(value); }
//----------------------------------------------------------------------------
void ini_block::destroy()
{
  safeDeleteA(name);
  uint n = items.getElem();
  for(uint i = 0; i < n; ++i)
    items[i].destroy();
  items.reset();
}
//----------------------------------------------------------------------------
ini_block* manageIni::findBlock(LPCTSTR name, bool canCreate)
{
  uint nElem = Ini.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_tcsicmp(name, Ini[i].name))
      return &(Ini[i]);
  if(canCreate) {
    Ini[nElem].name = str_newdup(name);
    return &(Ini[nElem]);
    }
  lastErr = le_NotFoundBlock;
  return 0;
}
//----------------------------------------------------------------------------
bool manageIni::existBlock(LPCTSTR blockName)
{
  return toBool(findBlock(blockName));
}
//----------------------------------------------------------------------------
ini_param* manageIni::findParam(LPCTSTR blockName, LPCTSTR paramName)
{
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return 0;
  uint nElem = blk->items.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_tcsicmp(paramName, blk->items[i].name))
      return &(blk->items[i]);
  lastErr = le_Not_FoundParam;
  return 0;
}
//----------------------------------------------------------------------------
void manageIni::getBlockList(PVect<LPCTSTR>& target)
{
  uint nElem = Ini.getElem();
  for(uint i = 0, j = target.getElem(); i < nElem; ++i, ++j)
    target[j] = str_newdup(Ini[i].name);
}
//----------------------------------------------------------------------------
void manageIni::getParamList(LPCTSTR blockName, PVect<ini_param>& target)
{
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return;

  uint nElem = blk->items.getElem();
  for(uint i = 0, j = target.getElem(); i < nElem; ++i, ++j) {
    target[j].name = str_newdup(blk->items[i].name);
    target[j].value = str_newdup(blk->items[i].value);
    }
}
//----------------------------------------------------------------------------
void manageIni::getParamList(LPCTSTR blockName, PVect<LPCTSTR>& target)
{
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return;

  uint nElem = blk->items.getElem();
  for(uint i = 0, j = target.getElem(); i < nElem; ++i, ++j)
    target[j] = str_newdup(blk->items[i].name);
}
//----------------------------------------------------------------------------
bool manageIni::createBlock(LPCTSTR name)
{
  if(findBlock(name)) {
    lastErr = le_Exists_Block;
    return false;
    }
  Ini[Ini.getElem()].name = str_newdup(name);
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::createParam(const ini_param& param, LPCTSTR blockName)
{
  if(findParam(blockName, param.name)) {
    lastErr = le_Exists_Param;
    return false;
    }
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return false;
  uint nElem = blk->items.getElem();
  blk->items[nElem].name = str_newdup(param.name);
  blk->items[nElem].value = str_newdup(param.value);
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::changeValue(const ini_param& param, LPCTSTR blockName)
{
  ini_param* p = findParam(blockName, param.name);
  if(!p)
    return false;
  delete []p->value;
  p->value = str_newdup(param.value);
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::addParam(const ini_param& param, LPCTSTR blockName)
{
  ini_block* blk = findBlock(blockName, true);
  if(!blk)
    return false;
  uint nElem = blk->items.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(param.name, blk->items[i].name)) {
      if(_tcsicmp(blk->items[i].value, param.value)) {
        delete []blk->items[i].value;
        blk->items[i].value = str_newdup(param.value);
        }
      return true;
      }
    }
  blk->items[nElem].name = str_newdup(param.name);
  blk->items[nElem].value = str_newdup(param.value);
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::renameParam(LPCTSTR blockName, LPCTSTR paramName, LPCTSTR oldParamName)
{
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return false;
  uint nElem = blk->items.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(oldParamName, blk->items[i].name)) {
      delete []blk->items[i].name;
      blk->items[i].name = str_newdup(paramName);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool manageIni::deleteParam(LPCTSTR paramName, LPCTSTR blockName)
{
  ini_block* blk = findBlock(blockName);
  if(!blk)
    return false;
  uint nElem = blk->items.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(paramName, blk->items[i].name)) {
      ini_param ip = blk->items.remove(i);
      ip.destroy();
      return true;
      }
    }
  lastErr = le_Not_FoundParam;
  return false;
}
//----------------------------------------------------------------------------
LPCTSTR manageIni::getValue(LPCTSTR paramName, LPCTSTR blockName)
{
  ini_param* p = findParam(blockName, paramName);
  if(!p)
    return 0;
  return p->value;
}
//----------------------------------------------------------------------------
bool manageIni::renameBlock(LPCTSTR blockName, LPCTSTR oldBlockName)
{
  ini_block* blk = findBlock(oldBlockName);
  if(!blk)
    return false;
  delete []blk->name;
  blk->name = str_newdup(blockName);
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::deleteBlock(LPCTSTR blockName)
{
  uint nElem = Ini.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(blockName, Ini[i].name)) {
      Ini[i].destroy();
      Ini.remove(i);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
#define MAX_DIM_NAME 512
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static LPTSTR get_line(LPTSTR buff, size_t len)
{
  if((int)len < 0)
    return 0;
  size_t i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
bool manageIni::parse_add_block(LPTSTR blockName, LPTSTR row)
{
  if(*row) {
    if(*row != _T('['))
      return false;
    ++row;
    for(uint i = 0; i < MAX_DIM_NAME; ++i) {
      // accettiamo anche la mancanza della parentesi di chiusura ...
      if(!row[i] || row[i] == _T(']')) {
        blockName[i] = 0;
        createBlock(blockName);
        return true;
        }
      blockName[i] = row[i];
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool manageIni::parse_add_row(LPCTSTR blockName, LPTSTR row)
{
  if(*row) {
    if(*row == _T('['))
      return false;
    ini_param ip;
    if(_T(';') == *row) { // è una riga di commento, la memorizziamo comunque
      ip.name = _T(";");
      ip.value = row + 1;
      }
    else {
      LPTSTR p = row;
      while(*p) {
        if(_T('=') == *p) {
          *p++ = 0;
          break;
          }
        ++p;
        }
      trim(row);
      ip.name = row;
      if(*p) {
        lTrim(p);
        ip.value = p;
        }
      }
    createParam(ip, blockName);
    }
  return true;
}
//----------------------------------------------------------------------------
bool manageIni::parse(LPCTSTR filename)
{
  if(!filename)
    filename = Filename;
  if(!filename)
    return false;
  uint dim = 0;
  LPTSTR buff = 0;
  do {
    P_File pf(filename, P_READ_ONLY);
    buff = openFile(filename, dim);
    if(!buff)
      return false;
  } while(false);

  LPTSTR p = buff;
  size_t offs = 0;
  bool enterBlock = false;
  TCHAR bn[MAX_DIM_NAME] = {0};;
  do {
    LPTSTR next = get_line(p, dim - offs);
    size_t len = next ? next - p : dim - offs;
    offs += len;
    trim(lTrim(p));
    if(enterBlock)
      enterBlock = parse_add_row(bn, p);
    if(!enterBlock)
      enterBlock = parse_add_block(bn, p);
    p = next;
  } while(p);
  delete []buff;
  return toBool(Ini.getElem());
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define WRITE_STRING P_writeToUnicode
#else
  #define WRITE_STRING P_writeToAnsi
#endif
//----------------------------------------------------------------------------
void manageIni::saveRows(P_File& pf, const PVect<ini_param>& rows)
{
  uint nElem = rows.getElem();
  for(uint i = 0; i < nElem; ++i) {
    pf.WRITE_STRING(rows[i].name);
    if(rows[i].name[0] != _T(';'))
      pf.WRITE_STRING(_T("="));
    pf.WRITE_STRING(rows[i].value);
    pf.WRITE_STRING(_T("\r\n"));
    }
}
//----------------------------------------------------------------------------
bool manageIni::save(LPCTSTR filename)
{
  if(!filename)
    filename = Filename;
  if(!filename)
    return false;
  P_File pf(filename, P_CREAT);
  if(!pf.P_open())
    return false;
  uint nElem = Ini.getElem();
  for(uint i = 0; i < nElem; ++i) {
    pf.WRITE_STRING(_T("["));
    pf.WRITE_STRING(Ini[i].name);
    pf.WRITE_STRING(_T("]\r\n"));
    saveRows(pf, Ini[i].items);
    }
  return true;
}
//----------------------------------------------------------------------------
void manageIni::flush()
{
  uint nElem = Ini.getElem();
  for(uint i = 0; i < nElem; ++i)
    Ini[i].destroy();
  Ini.reset();
}
