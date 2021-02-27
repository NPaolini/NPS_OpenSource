//----------------------- p_vect.cpp -----------------------------------
//----------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_vect.h"
//-----------------------------------------------------------
#ifdef USE_COMMON_BASE
//-----------------------------------------------------------
bool PVectBase::setDim(uint newdim)
{
  if(newdim <= Dim)
    return true;
  return resize(newdim - Dim);
}
//-----------------------------------------------------------
int PVectBase::getVerifiedPos(uint pos)
{
  if(pos >= Dim) {
    uint n = pos - Dim;
    n /= Incr;
    ++n;
    n *= Incr;
    if(!resize(n))
    	pos = Dim - 1;
    }
  if(nElem <= pos)
    nElem = pos + 1;

  return pos;
}
//-----------------------------------------------------------
void PVectBase::set(LPVOID val, uint pos)
{
	pos = getVerifiedPos(pos);
  memcpy(Vect + pos * vSize, val, vSize);
}
//-----------------------------------------------------------
bool PVectBase::resize(uint incr)
{
  LPBYTE tmp = new BYTE[(Dim + incr) * vSize];
  if(tmp) {
    if(Vect) {
      memcpy(tmp, Vect, Dim * vSize);
      delete [] Vect;
      }
    Vect = tmp;
    Dim += incr;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
LPVOID PVectBase::remove(uint pos)
{
  if(pos >= nElem)
  	return 0;

	if(nElem == Dim)
		resize(2);

	LPVOID tmp = Vect + (Dim - 1) * vSize;
	memcpy(tmp, get(pos), vSize);
	memmove(get(pos), get(pos + 1), (nElem - pos - 1) * vSize);
	--nElem;

	return tmp;
}
//-----------------------------------------------------------
bool PVectBase::remove(uint pos, LPVOID target)
{
	LPVOID tmp = remove(pos);
	if(tmp)
		memcpy(target, tmp, vSize);

	return toBool(tmp);
}
//-----------------------------------------------------------
bool PVectBase::insert(LPVOID val, uint pos)
{
  if(pos >= Dim) {
    if(!resize(Dim - pos))
      return false;
    }
  else if(nElem == Dim - 1)
    if(!resize(Incr))
      return false;

	memmove(get(pos + 1), get(pos), (nElem - pos) * vSize);
	memcpy(get(pos), val, vSize);

  ++nElem;
  return true;
}
//-----------------------------------------------------------
bool PVectBase::setElem(uint newElem)
{
  if(newElem < Dim) {
    nElem = newElem;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
#endif