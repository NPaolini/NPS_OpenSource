//-------- sv_manage_skip.cpp -----------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "sv_manage_skip.h"
#include <stdio.h>
#include <stdlib.h>
//----------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------
#define MAX_LABEL 128
//----------------------------------------------------------
/*
  1) salva il blocco compreso tra l'inizio del buffer e la riga successiva a <calc>
  2) elabora le righe
    a) se trova un riferimento a label (#)
      a1) se è una label, inserisce nel set. Se ci sono elementi in notFound, elabora
          per verificare se può assegnare lo skip.
      a2) altrimenti ricerca se la label è già stata trovata, in tal caso assegna lo skip
          in caso contrario mette la riga nelle notFound.
          Prosegue poi nel caso (b)
    b) copia la riga nel set 'Rows'

    c) trova la fine della sezione 'calc'. Se ci sono ancora elementi in notFound torna
       con segnalazione di errore (oppure li imposta come 'abort').
       copia il restante buffer nell'ultima Rows e termina il ciclo.
  3) Calcola la dimensione totale degli elementi, alloca e copia le righe.

  ************
  modifiche da fare con l'aggiunta di funzioni custom:
  il blocco passato è depurato da inizio-fine blocco, quindi può essere sia il blocco di CALC
  che la singola funzione,
*/
//----------------------------------------------------------
void svManSkip::flush()
{
  delete []buffElab;
  buffElab = 0;

  uint nEl = notFound.getElem();
  for(uint i = 0; i < nEl; ++i)
    delete []notFound[i].skipLabel;
  notFound.reset();

  nEl = Label.getElem();
  for(uint i = 0; i < nEl; ++i)
    delete []Label[i].skipLabel;
  Label.reset();

  flushPAV(Rows);
}
//----------------------------------------------------------
static
LPTSTR getLine(LPTSTR buff)
{
  if(!*buff)
    return 0;
  LPTSTR p = buff;
  for(; *p; ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;

  if(!*p)
    return 0;

  *p++ = 0;
  if(!*p)
    return 0;

  for(; *p; ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(*p ? p : 0);
}
//------------------------------------------------------------------
static void extactLabel(LPTSTR l, LPCTSTR p)
{
  *l++ = *p++; // il primo è sicuramente il '#'

  while(*p && (_istalnum(*p) || _T('_') == *p))
    *l++ = *p++;
  *l = 0;
}
//------------------------------------------------------------------
void svManSkip::manageCalc(LPTSTR p)
{
  uint nEl = Rows.getElem();
  LPCTSTR pL = _tcschr(p, _T('#'));
  if(pL) {
    TCHAR tRow[1024];
    _tcscpy_s(tRow, p);
    uint offs = pL - p;
    do {
      TCHAR label[MAX_LABEL];
      extactLabel(label, pL);
      uint nLab = Label.getElem();
      bool found = false;
      for(uint i = 0; i < nLab; ++i) {
        if(!_tcsicmp(Label[i].skipLabel, label)) {
          wsprintf(tRow + offs, _T("%d"), (int)(Label[i].row - nEl));
          offs = _tcslen(tRow);
          _tcscpy_s(tRow + offs, SIZE_A(tRow) - offs, pL + _tcslen(label));
          found = true;
          break;
          }
        }
      if(!found) {
        infoCalc ic(nEl, str_newdup(label));
        notFound[notFound.getElem()] = ic;
        LPCTSTR pLt = _tcschr(pL + 1, _T('#'));
        if(pLt)
          offs += pLt - pL;
        pL = pLt;
        }
      else
        pL = _tcschr(pL + 1, _T('#'));
      } while(pL);
    Rows[nEl] = str_newdup(tRow);
    }
  else
    Rows[nEl] = str_newdup(p);
}
//------------------------------------------------------------------
void svManSkip::replaceLabel(uint ix, const infoCalc& iLabel)
{
  const infoCalc& check = notFound[ix];
  LPCTSTR p = Rows[check.row];
  LPCTSTR pL = _tcschr(p, _T('#'));
  if(!pL)  // ???
    return;

  TCHAR tRow[1024];
  _tcscpy_s(tRow, p);

  do {
    TCHAR label[MAX_LABEL];
    extactLabel(label, pL);
    uint offs = pL - p;
    if(!_tcsicmp(iLabel.skipLabel, label)) {
      wsprintf(tRow + offs, _T("%d"), iLabel.row - check.row - 1);
      offs = _tcslen(tRow);
      _tcscpy_s(tRow + offs, SIZE_A(tRow) - offs, pL + _tcslen(label));
      break;
      }
    pL = _tcschr(pL + 1, _T('#'));
    } while(pL);
  delete []Rows[check.row];
  Rows[check.row] = str_newdup(tRow);
  infoCalc& rem = notFound[ix];
  delete []rem.skipLabel;
  notFound.remove(ix);
}
//------------------------------------------------------------------
bool svManSkip::manageLabel(LPTSTR p)
{
  TCHAR label[MAX_LABEL];
  extactLabel(label, p);

  uint nLab = Label.getElem();
  for(uint i = 0; i < nLab; ++i) {
    if(!_tcsicmp(Label[i].skipLabel, label)) {
      manV->shoMsg(ID_EXPR_ERROR, label, -2, 0);
      onError = true;
      return false;
      }
    }
  Label[nLab].row = Rows.getElem();
  Label[nLab].skipLabel = str_newdup(label);
  int nEl = notFound.getElem();
  for(int i = nEl - 1; i >= 0; --i) {
    if(!_tcsicmp(notFound[i].skipLabel, label))
      replaceLabel(i, Label[nLab]);
    }
  return true;
}
//------------------------------------------------------------------
extern bool isReturn(LPCTSTR p, int& pos);
//------------------------------------------------------------------
bool svManSkip::manageLine(LPTSTR p)
{
  if(!p)
    return false;
  int pos;
  if(isReturn(p, pos)) {
    Rows[Rows.getElem()] = str_newdup(p);
    return true;
    }
  while(*p && (*p == _T(' ') || *p != VAR_CHAR && *p != _T('#') && *p != _T(';') && *p != _T('{') && *p != _T('}') && *p != VAR_FUNCT_CHAR))
    ++p;

  switch(*p) {
    case VAR_CHAR:
    case VAR_FUNCT_CHAR:
    case _T('{'):
    case _T('}'):
      manageCalc(p);
      break;
    case _T('#'):
      return manageLabel(p);
    }
  return true;
}
//------------------------------------------------------------------
static LPTSTR appendRow(LPTSTR buff, LPCTSTR s)
{
  while(*s)
    *buff++ = *s++;
  *buff++ = _T('\r');
  *buff++ = _T('\n');
  *buff = 0;
  return buff;
}
//------------------------------------------------------------------
LPTSTR svManSkip::finish(uint& len)
{
  uint tot = 4;
  uint nEl = Rows.getElem();
  for(uint i = 0; i < nEl; ++i) {
    tot += _tcslen(Rows[i]) + 2;
    }
  LPTSTR buff = new TCHAR[tot];
  buff[0] = 0;
  LPTSTR p = buff;
  for(uint i = 0; i < nEl; ++i)
    p = appendRow(p, Rows[i]);

  len = _tcslen(buff) + 1;
  return buff;
}
//------------------------------------------------------------------
LPTSTR svManSkip::run(LPTSTR buff, uint& len)
{
  flush();
  buffElab = str_newdup(buff);
  LPTSTR p = buffElab;
//  LPTSTR p = findInit(buffElab);
//  if(!p)
//    return 0;
  while(p && *p) {
    LPTSTR p2 = getLine(p);
    if(!manageLine(p))
      break;
    p = p2;
    }
  if(onError) {
    len = 0;
    return 0;
    }
  return finish(len);
}
//------------------------------------------------------------------
