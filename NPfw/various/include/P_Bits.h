//-------------- P_BITS.H -----------------------------------------------------
#ifndef P_BITS_H_
#define P_BITS_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
#include "setPack.h"
//-----------------------------------------------------------------------------
class P_Bits
{
  public:
    P_Bits(DWORD nBits = 0);
    ~P_Bits();

    P_Bits(const P_Bits& other) : Bits(0), nDWord(0) { clone(other); }
    const P_Bits& operator=(const P_Bits& other) { clone(other); return *this; }

    bool newDef(DWORD n_bits, bool copy = false);

    bool isSet(DWORD bit);
    bool isDWordSet(DWORD dword);

    bool hasSet() const { return toBool(numOfSet); }
    DWORD totSet() const { return numOfSet; }

    void set(DWORD bit);
    void clear(DWORD bit);
    void toggle(DWORD bit);
    void setAll();
    void clearAll();
    void toggleAll();

    DWORD getNumDWORD() const { return nDWord; }
    DWORD getNumBit() const { return nBits; }

    DWORD getDWordAt(uint offset) const;
    void setDWordAt(uint offset, DWORD val);
  private:
    DWORD* Bits;
    DWORD nBits;
    DWORD nDWord;
    DWORD numOfSet;

    void clone(const P_Bits& other);
};
//-----------------------------------------------------------------------------
#include "restorePack.h"
#endif
