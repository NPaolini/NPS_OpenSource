//------ newNormal.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef NEWNORMAL_H_
#define NEWNORMAL_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "defin.h"
#include "setPack.h"
//----------------------------------------------------------------------------
fREALDATA getNorm(uint ix);
bool setupNormal(LPCTSTR file, PWin* parent);
//----------------------------------------------------------------------------
// torna true se il valore è compreso nel range o se non esiste un range
// il primo usa il valore normalizzato, il secondo il valore denormalizzato
bool isOnRange(fREALDATA val, uint ix);
bool isOnRange(DWDATA val, uint ix);
//----------------------------------------------------------------------------
void getMinMax(fREALDATA& vMin, fREALDATA& vMax, uint ix);
//----------------------------------------------------------------------------
// da richiamare dopo aver settato le directory di lavoro
void initNormal(LPCTSTR file);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
