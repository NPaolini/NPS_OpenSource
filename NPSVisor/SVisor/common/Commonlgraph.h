//-------- Commonlgraph.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMONLGRAPH_H_
#define COMMONLGRAPH_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_defbody.h"
#include "p_txt.h"
#include "ptextpanel.h"
#include "pstatic.h"
#include "pgraph.h"
#include "pedit.h"
#include "svDialog.h"
#include "svEdit.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// i dati sono memorizzati, nel vettore BuffY, nel range 0-maxval
// le variabili minVal e maxVal contengono i valori minimo e massimo reali
// Scale e Offset vengono calcolati come rapporto tra i dati reali e
// quelli presenti nei campi di edit
//#define MAX_H_DATA MAXSHORT
#define NBIT_TYPE(a) (8 * sizeof(a))
#define MAX_H_DATA ((DWORD)(((__int64)1 << NBIT_TYPE(lgCoord)) - 1) & ~(1 << (NBIT_TYPE(lgCoord) - 1)))

// per la simulazione di decimali
//#define MUL_W_DATA 20
// il tipo è stato portato da short a long, quindi possiamo aumentare il numero
// di decimali dispoanibili, (utile per lo zoom)
#define MUL_W_DATA 1000
//----------------------------------------------------------------------------
#define MAX_LINEAR_GRAPH 10
#define SIZE_STRUCT_RANGE 4096
#define MAX_SET ((SIZE_STRUCT_RANGE - sizeof(__int64) * 2 - sizeof(int) * MAX_LINEAR_GRAPH) / (sizeof(fREALDATA) * 2))
//----------------------------------------------------------------------------
DWORD findFirstPos(P_File& pFile, const FILETIME& ft, int lenRec, LPBYTE buff, int sizeHeader, bool prev, int offset=0);
//----------------------------------------------------------------------------
inline
DWORD findFirstPos(P_File& pFile, const SYSTEMTIME& st, int lenRec, LPBYTE buff, int sizeHeader, bool prev, int offset=0)
{
  FILETIME ft;
  SystemTimeToFileTime(&st, &ft);
  return findFirstPos(pFile, ft, lenRec, buff, sizeHeader, prev, offset);
}
//----------------------------------------------------------------------------
struct rangeSet
{
  __int64 from;
  __int64 to;
  fREALDATA rMin[MAX_SET];
  fREALDATA rMax[MAX_SET];
  int choose[MAX_LINEAR_GRAPH];
};
//----------------------------------------------------------------------------
//#if (sizeof(rangeSet) != SIZE_STRUCT_RANGE)
//  #error dimensione errata struct rangeSet
//#endif
//----------------------------------------------------------------------------
template <class T>
class statist
{
  public:
    // va inizializzata con il primo valore
    statist(T vMin, T vMax) : Min(vMin), Max(vMax), nElem(1)
#ifdef ALSO_MEDIA
      , Sum(v)
#endif
      { }
    void calc(T v);
    T getMin() const { return Min; }
    T getMax() const { return Max; }
#ifdef ALSO_MEDIA
    T getAverage() const { return Sum / nElem; }
#endif
  private:
    T Min;
    T Max;
#ifdef ALSO_MEDIA
    T Sum;
#endif
    int nElem;
};
//----------------------------------------------------------------------------
#define MAX_DOUBLE (DBL_MAX / 10)
template <class T>
void statist<T>::calc(T v)
{
  if(Min > v && v > 0.0)
    Min = v;
  if(Max < v && v < MAX_DOUBLE)
    Max = v;
  if(Min > Max)
    Min = Max;
#ifdef ALSO_MEDIA
  Sum += v;
#endif
  ++nElem;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class linear : public PLinearGraph
{
  public:
    linear(PRect rect, COLORREF bkg, bStyle type, COLORREF frg, bool axis);
    void rewind() { Curr = 0; }
    int getNPoints() { return PLinearGraph::getNPoints(); }
    void setBuff(const lgCoord* buffX, const lgCoord* buffY, int dim);
    void reset() {  setMaxPoint(0); }
    COLORREF getColor() const { return getLineColor(); }
  protected:
    virtual sPoint getNext();
    virtual void drawLines(HDC hdc);
    int Curr;
    const lgCoord* BuffY;
    const lgCoord* BuffX;
  private:
    double Scale;
};
//--------------------------------------------------------------------
inline void linear::setBuff(const lgCoord* buffX, const lgCoord* buffY, int dim)
{
  BuffX = buffX;
  BuffY = buffY;
  setMaxPoint(dim);
  rewind();
}
//----------------------------------------------------------------------------
#define USE_FILETIME_4_X -1
#define MAX_DESCR 26
//typedef const BYTE* LPCBYTE;
//----------------------------------------------------------------------------
#define OFFS_TRENDNAME  6
//----------------------------------------------------------------------------
uint getIdByNameTrend(LPCTSTR name);
uint getInitDataByNameTrend(LPCTSTR name, uint& nElem);
LPCTSTR getDescrDataTrend(uint id, bool& needDelete);
smartPointerConstString getDescrDataTrend(uint id);
bool getNameTrendById(LPTSTR name, uint sz, uint id);

LPCTSTR checkNewMode(const setOfString& set, LPCTSTR p);
LPCTSTR checkNewMode(LPCTSTR p);
//--------------------------------------------------------------------
class TD_Timer : public svModDialog
{
  public:
    TD_Timer(PWin* parent, LPCTSTR caption, LPCTSTR prompt, uint &time, uint resId = IDD_SET_TIME_FOR_TIMER);

    virtual bool create();

  protected:
    svEdit* _Time;
    PStatic* txtTime;
    LPCTSTR Capt;
    LPCTSTR Prompt;
    void CmOk();

  private:
    uint& Time;
    typedef svModDialog baseClass;
};
//----------------------------------------------------------------------------
void startSimulDialog(PWin& diag, PWin* owner);
//----------------------------------------------------------------------------
#endif
