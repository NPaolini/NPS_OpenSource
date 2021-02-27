//------------ CFireRoutine.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef CFireRoutine_H_
#define CFireRoutine_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
class CFireRoutine
{
public:
  CFireRoutine();
  virtual ~CFireRoutine();

  // Functs (public)

  void    InitFire();
  void    ClrHotSpots();
  void    InitPallette();
  void    SetHotSpots();
  void    MakeLines();
  void    Render(DWORD* pVideoMemory,
                     int iwidth,
             int iheight);


  unsigned char Average(int x, int y);

  // props
  int     m_iFlameHeight;
  int     m_iWidth;
  int     m_iHeight;
  int     m_iFireSource;//The y position for the lit spots
  int     m_iFireChance;
  int     m_iAvgFlameWidth;
  int     m_iAlpha;

  COLORREF  m_FireColors[4];

  BYTE* m_pFireBits;
  DWORD   m_pPalletteBuffer[256];
  long* m_pYIndexes;
};
//----------------------------------------------------------------------------
#endif
