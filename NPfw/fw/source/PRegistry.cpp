//------ PRegistry.cpp -------------------------------------------------------
#include "precHeader.h"
#include "pregistry.h"
//----------------------------------------------------------------------------
void PRegKey::changeValue(LPCBYTE newValue, DWORD newSize)
{
  delete []Value;
  Size = newSize;
  Value = new BYTE[Size];
  memcpy(Value, newValue, Size);
}
//----------------------------------------------------------------------------
void PRegKey::allocSize(DWORD newSize)
{
  delete []Value;
  Size = newSize;
  Value = new BYTE[Size];
  ZeroMemory(Value, Size);
}
//----------------------------------------------------------------------------
PRegKey& PRegKey::operator=(const PRegKey& other)
{
  if(this != &other) {
    changeName(other.Name);
    changeValue(other.Value, other.Size);
    changeType(other.Type);
    }
  return *this;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRegistry::error PRegistry::openKey(HKEY& result, LPCTSTR path, REGSAM access)
{
  DWORD dwDisposition;
  // inutile aprire con accesso pieno se non serve e non richiesto
#if 1
  if(ERROR_SUCCESS == RegCreateKeyEx(Base, path, 0, 0,
          REG_OPTION_NON_VOLATILE, access, 0, &result,
          &dwDisposition))
    return SUCCESS;
  return NOT_OPEN;
#else
  if(ERROR_SUCCESS != RegCreateKeyEx(Base, path, 0, 0,
              REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &result,
              &dwDisposition)) {
    // se non riesce ad aprire usando l'accesso pieno
    // verifica se si richiede un accesso mirato
    if(KEY_ALL_ACCESS != access)
      if(ERROR_SUCCESS == RegCreateKeyEx(Base, path, 0, 0,
              REG_OPTION_NON_VOLATILE, access, 0, &result,
              &dwDisposition))
        return SUCCESS;

    return NOT_OPEN;
    }

  return SUCCESS;
#endif
 }
//----------------------------------------------------------------------------
PRegistry::error PRegistry::openExistingKey(HKEY& result, LPCTSTR path, REGSAM access)
{
  // idem come sopra
#if 1
  if(ERROR_SUCCESS == RegOpenKeyEx(Base, path, 0, access, &result))
    return SUCCESS;
  return NOT_OPEN;
#else
  if(ERROR_SUCCESS != RegOpenKeyEx(Base, path, 0, KEY_ALL_ACCESS, &result)) {
    // se non riesce ad aprire usando l'accesso pieno
    // verifica se si richiede un accesso mirato
    if(KEY_ALL_ACCESS != access)
      if(ERROR_SUCCESS == RegOpenKeyEx(Base, path, 0, access, &result))
        return SUCCESS;

    return NOT_OPEN;
    }

  return SUCCESS;
#endif
 }
//----------------------------------------------------------------------------
PRegistry::error PRegistry::writeKey(const PRegKey& key, LPCTSTR path)
{
  HKEY subKey;
  if(SUCCESS != openKey(subKey, path, KEY_WRITE))
    return NOT_OPEN;

  error result = SUCCESS;
  if(ERROR_SUCCESS != RegSetValueEx(subKey, key.getName(), 0, key.getType(),
                      key.getValue(), key.getSize()))
    result = NOT_WRITE;

  closeKey(subKey);

  return result;
}
//----------------------------------------------------------------------------
PRegistry::error PRegistry::readKey(PRegKey& key, LPCTSTR path)
{
  HKEY subKey;
  if(SUCCESS != openExistingKey(subKey, path, KEY_READ))
    return NOT_OPEN;

  DWORD type;
  error result = SUCCESS;

  if(!key.getValue()) {
    DWORD needed;
    if(ERROR_SUCCESS != RegQueryValueEx(subKey, key.getName(), 0, &type,
                      0, &needed))
      result = NOT_READ;

    else
      key.allocSize(needed);
    }

  if(SUCCESS == result)
    if(ERROR_SUCCESS != RegQueryValueEx(subKey, key.getName(), 0, &type,
                      key.getValue(), &key.getSize()))
      result = NOT_READ;

  if(SUCCESS == result)
    key.getType() = type;

  closeKey(subKey);

  return result;
}
//----------------------------------------------------------------------------
bool PRegistry::setBaseKey(HKEY newBase, bool close)
{
  if(autoClose)
    closeKey(Base);
   Base = newBase;
   autoClose = close;
   return true;
}
//----------------------------------------------------------------------------
PRegistry::error PRegistry::deleteKey(const PRegKey& key, LPCTSTR path)
{
  HKEY subKey;
  if(SUCCESS != openKey(subKey, path, KEY_WRITE))
    return NOT_OPEN;

  error result = SUCCESS;
  if(ERROR_SUCCESS != RegDeleteKey(subKey, key.getName()))
    result = NOT_DELETED;

  closeKey(subKey);

  return result;
}
//----------------------------------------------------------------------------
PRegistry::error PRegistry::deleteValue(const PRegKey& key, LPCTSTR path)
{
  HKEY subKey;
  if(SUCCESS != openKey(subKey, path, KEY_WRITE))
    return NOT_OPEN;

  error result = SUCCESS;
  if(ERROR_SUCCESS != RegDeleteValue(subKey, key.getName()))
    result = NOT_DELETED;

  closeKey(subKey);

  return result;
}
//----------------------------------------------------------------------------
