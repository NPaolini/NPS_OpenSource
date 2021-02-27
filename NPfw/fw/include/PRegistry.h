//------ PRegistry.h ---------------------------------------------------------
#ifndef PREGISTRY_H_
#define PREGISTRY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <regstr.h>
#include "p_util.h"
//#define LPCBYTE const LPBYTE
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PRegKey
{
  public:
    PRegKey(DWORD type);
    PRegKey(LPCTSTR name, DWORD type);
    PRegKey(LPCTSTR name, LPCBYTE value, DWORD type, DWORD size);

    PRegKey(const PRegKey&);
    PRegKey& operator=(const PRegKey&);

    virtual ~PRegKey();

    virtual LPCTSTR getName() const;

    virtual LPBYTE getValue();
    virtual DWORD& getType();
    virtual DWORD& getSize();

    virtual LPCBYTE getValue() const;
    virtual DWORD getType() const;
    virtual DWORD getSize() const;

    virtual void changeName(LPCTSTR newName);
    virtual void changeValue(LPCBYTE newValue);
    virtual void changeValue(LPCBYTE newValue, DWORD size);
    virtual void changeType(DWORD newType);

    virtual void allocSize(DWORD newSize);

  protected:
  private:
    LPTSTR Name;
    LPBYTE Value;
    DWORD Type;
    DWORD Size;

};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PRegistry
{
  public:
    PRegistry(HKEY baseKey = HKEY_CURRENT_USER);
    ~PRegistry();

    enum error { SUCCESS, NOT_OPEN, NOT_WRITE, NOT_READ, NOT_DELETED };

    bool setBaseKey(HKEY newBase, bool close = false);

    error writeKey(const PRegKey& key, LPCTSTR path);
    error readKey(PRegKey& key, LPCTSTR path);

    error deleteKey(const PRegKey& key, LPCTSTR path);
    error deleteValue(const PRegKey& key, LPCTSTR path);

    error openKey(HKEY& result, LPCTSTR path, REGSAM access = KEY_ALL_ACCESS);
    error openExistingKey(HKEY& result, LPCTSTR path, REGSAM access = KEY_ALL_ACCESS);

    void closeKey(HKEY key) {   RegCloseKey(key); }
  private:
    HKEY Base;
    bool autoClose;
    NO_COPY_COSTR_OPER(PRegistry)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PRegKeyStr : public PRegKey
{
  public:
    PRegKeyStr() : PRegKey(REG_SZ) {}
    PRegKeyStr(LPCTSTR name) : PRegKey(name, REG_SZ) {}
    PRegKeyStr(LPCTSTR name, LPCTSTR value);
    LPCTSTR getString();
    void setString(LPCTSTR str);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PRegKeyDWORD : public PRegKey
{
  public:
    PRegKeyDWORD() : PRegKey(REG_DWORD) {}
    PRegKeyDWORD(LPCTSTR name) : PRegKey(name, REG_DWORD) {}
    PRegKeyDWORD(LPCTSTR name, DWORD value);
    DWORD getDWORD();
    void setDWORD(DWORD value);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PRegKeyBinary : public PRegKey
{
  public:
    PRegKeyBinary() : PRegKey(REG_BINARY) {}
    PRegKeyBinary(LPCTSTR name) : PRegKey(name, REG_BINARY) {}
    PRegKeyBinary(LPCTSTR name, LPCBYTE value, DWORD len);
    LPCBYTE getBinary();
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline PRegKey::PRegKey(DWORD type) : Name(0), Value(0), Type(type), Size(0)
{ }
//----------------------------------------------------------------------------
inline PRegKey::PRegKey(LPCTSTR name, DWORD type) :
    Name(str_newdup(name)), Value(0), Type(type), Size(0)
{ }
//----------------------------------------------------------------------------
inline PRegKey::PRegKey(LPCTSTR name, LPCBYTE value, DWORD type, DWORD size) :
    Name(str_newdup(name)), Value((LPBYTE)memcpy(new BYTE[size], value, size)),
    Type(type), Size(size)
{ }
//----------------------------------------------------------------------------
inline PRegKey::PRegKey(const PRegKey& other) :
    Name(str_newdup(other.Name)),
    Value((LPBYTE)memcpy(new BYTE[other.Size], other.Value, other.Size)),
    Type(other.Type), Size(other.Size)
{ }
//----------------------------------------------------------------------------
inline PRegKey::~PRegKey()
{
  delete []Name;
  delete []Value;
}
//----------------------------------------------------------------------------
inline
LPCTSTR PRegKey::getName() const
{
  return Name;
}
//----------------------------------------------------------------------------
inline
LPBYTE PRegKey::getValue()
{
  return Value;
}
//----------------------------------------------------------------------------
inline
DWORD& PRegKey::getType()
{
  return Type;
}
//----------------------------------------------------------------------------
inline
DWORD& PRegKey::getSize()
{
  return Size;
}
//----------------------------------------------------------------------------
inline
LPCBYTE PRegKey::getValue() const
{
  return Value;
}
//----------------------------------------------------------------------------
inline
DWORD PRegKey::getType() const
{
  return Type;
}
//----------------------------------------------------------------------------
inline
DWORD PRegKey::getSize() const
{
  return Size;
}
//----------------------------------------------------------------------------
inline
void PRegKey::changeName(LPCTSTR newName)
{
  delete []Name;
  Name = str_newdup(newName);
}
//----------------------------------------------------------------------------
inline
void PRegKey::changeValue(LPCBYTE newValue)
{
  memcpy(Value, newValue, Size);
}
//----------------------------------------------------------------------------
inline
void PRegKey::changeType(DWORD newType)
{
  Type = newType;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
PRegistry::PRegistry(HKEY baseKey) : Base(baseKey), autoClose(false) { }
//----------------------------------------------------------------------------
inline PRegistry::~PRegistry()
{
  if(autoClose)
    closeKey(Base);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline PRegKeyStr::PRegKeyStr(LPCTSTR name, LPCTSTR value) :
      PRegKey(name, (LPCBYTE)value, REG_SZ,(DWORD)((_tcslen(value) + 1) * sizeof(*value)))
{ }
//----------------------------------------------------------------------------
inline LPCTSTR PRegKeyStr::getString()
{
  return (LPCTSTR)getValue();
}
//----------------------------------------------------------------------------
inline void PRegKeyStr::setString(LPCTSTR str)
{
  changeValue((LPCBYTE)str, (DWORD)((_tcslen(str) + 1) * sizeof(*str)));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline PRegKeyDWORD::PRegKeyDWORD(LPCTSTR name, DWORD value) :
      PRegKey(name, (LPCBYTE)&value, REG_DWORD, sizeof(value))
{
}
//----------------------------------------------------------------------------
inline DWORD PRegKeyDWORD::getDWORD()
{
  return *(DWORD*)getValue();
}
//----------------------------------------------------------------------------
inline void PRegKeyDWORD::setDWORD(DWORD value)
{
  changeValue((LPCBYTE)&value, sizeof(value));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline PRegKeyBinary::PRegKeyBinary(LPCTSTR name, LPCBYTE value, DWORD len) :
      PRegKey(name, value, REG_BINARY, len)
{
}
//----------------------------------------------------------------------------
inline LPCBYTE PRegKeyBinary::getBinary()
{
  return (LPCBYTE)getValue();
}
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
