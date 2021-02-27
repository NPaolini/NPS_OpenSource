//-------------- P_BITS.CPP ---------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "P_Bits.h"
//-----------------------------------------------------------------------------
static const DWORD bitOfDWord = (sizeof(DWORD) * 8);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define USE_SHIFT

#ifdef USE_SHIFT
  //---------------------------------------------------------------------------
  template <DWORD size>
  DWORD calcExponentOfBase2()
  {
    if(size > 1)
      return calcExponentOfBase2<(size >> 1)>() + 1;
    return 0;
  }
  //---------------------------------------------------------------------------
  static const DWORD bitCount = calcExponentOfBase2<bitOfDWord>();
  //---------------------------------------------------------------------------
  #define MASK_BIT_BASE ((1 << bitCount) - 1)
  //---------------------------------------------------------------------------
  #define CALC_TOT_DWORD(bits) (((bits)  + bitOfDWord - 1) >> bitCount)
  #define CALC_DWORD(bits) ((bits) >> bitCount)
  #define CALC_BITS(bits) bits = 1 << (bits & MASK_BIT_BASE)
  //---------------------------------------------------------------------------
#else
  //---------------------------------------------------------------------------
  #define CALC_TOT_DWORD(bits) (((bits)  + bitOfDWord - 1) / bitOfDWord)
  #define CALC_DWORD(bits) ((bits) / bitOfDWord)
  #define CALC_BITS(bits) bits = 1 << (bits % bitOfDWord)
  //---------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
P_Bits::P_Bits(DWORD nBits) : nBits(nBits), nDWord(0), Bits(0), numOfSet(0)
{
  if(nBits) {
    if(!bitCount) {
      DWORD bitCount = calcExponentOfBase2<bitOfDWord>();
      nDWord = CALC_TOT_DWORD(nBits);
      }
    else
      nDWord = CALC_TOT_DWORD(nBits);
    Bits = new DWORD[nDWord];
    memset(Bits, 0, sizeof(DWORD) * nDWord);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
P_Bits::~P_Bits()
{
  delete []Bits;
}
//-----------------------------------------------------------------------------
void P_Bits::clone(const P_Bits& other)
{
  if(&other != this) {
    nBits = other.nBits;
    numOfSet = other.numOfSet;
    if(nBits) {
      if(nDWord != other.nDWord) {
        nDWord = other.nDWord;
        delete []Bits;
        Bits = new DWORD[nDWord];
        }
      memcpy(Bits, other.Bits, sizeof(DWORD) * nDWord);
      }
    else
      nDWord = other.nDWord;
    }
}
//-----------------------------------------------------------------------------
bool P_Bits::isSet(DWORD bit)
{
  if(bit >= nBits)
    return false;
  DWORD DWord = CALC_DWORD(bit);
  CALC_BITS(bit);
  return toBool(Bits[DWord] & bit);
}
//-----------------------------------------------------------------------------
DWORD P_Bits::getDWordAt(uint offset) const
{
  if(offset >= nDWord)
    return 0;
  return Bits[offset];
}
//-----------------------------------------------------------------------------
static uint getActiveBit(DWORD val)
{
  uint act = 0;
  for(uint i = 0; i < bitOfDWord; ++i, val >>= 1)
    act += val & 1;
  return act;
}
//-----------------------------------------------------------------------------
void P_Bits::setDWordAt(uint offset, DWORD val)
{
  if(offset >= nDWord)
    return;
  numOfSet += getActiveBit(val);
  Bits[offset] = val;
}
//-----------------------------------------------------------------------------
bool P_Bits::isDWordSet(DWORD dword)
{
  if(dword >= nDWord)
    return false;
  return toBool(Bits[dword]);
}
//-----------------------------------------------------------------------------
void P_Bits::set(DWORD bit)
{
  if(bit >= nBits)
    return;
  DWORD DWord = CALC_DWORD(bit);
  CALC_BITS(bit);
  if(!(Bits[DWord] & bit)) {
    ++numOfSet;
    Bits[DWord] |= bit;
    }
}
//-----------------------------------------------------------------------------
void P_Bits::clear(DWORD bit)
{
  if(bit >= nBits)
    return;
  DWORD DWord = CALC_DWORD(bit);
  CALC_BITS(bit);
  if(Bits[DWord] & bit) {
    --numOfSet;
    Bits[DWord] &= ~bit;
    }
}
//-----------------------------------------------------------------------------
void P_Bits::toggle(DWORD bit)
{
  if(bit >= nBits)
    return;
  DWORD DWord = CALC_DWORD(bit);
  CALC_BITS(bit);
  if(Bits[DWord] & bit)
    --numOfSet;
  else
    ++numOfSet;
  Bits[DWord] ^= bit;
}
//-----------------------------------------------------------------------------
void P_Bits::setAll()
{
  memset(Bits, (DWORD)-1, sizeof(DWORD) * nDWord);
  numOfSet = nBits;
}
//-----------------------------------------------------------------------------
void P_Bits::clearAll()
{
  memset(Bits, 0, sizeof(DWORD) * nDWord);
  numOfSet = 0;
}
//-----------------------------------------------------------------------------
void P_Bits::toggleAll()
{
  for(DWORD i = 0; i < nDWord; ++i)
    Bits[i] ^= (DWORD)-1;
  numOfSet = nBits - numOfSet;
}
//-----------------------------------------------------------------------------
bool P_Bits::newDef(DWORD n_bits, bool copy)
{
  if(!n_bits)
    return false;
  if(n_bits <= nDWord * bitOfDWord) {
    nBits = n_bits;
    if(!copy)
      clearAll();
    }
  else {
    DWORD newDWord = CALC_TOT_DWORD(n_bits);
    DWORD* p;
    if(copy) {
      p = new DWORD[newDWord];
      memset(p, 0, sizeof(DWORD) * newDWord);
      memcpy(p, Bits, nDWord * sizeof(DWORD));
      delete []Bits;
      }
    else {
      delete []Bits;
      p = new DWORD[newDWord];
      memset(p, 0, sizeof(DWORD) * newDWord);
      numOfSet = 0;
      }
    Bits = p;
    nBits = n_bits;
    nDWord = newDWord;
    }
  return true;
}
