//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <float.h>
//----------------------------------------------------------------------------
#include "mainwin.h"
//----------------------------------------------------------------------------
#include "p_checkKey.h"
//----------------------------------------------------------------------------
/*
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
*/
//----------------------------------------------------------------------------
class genApp : public PAppl
{
  public:
    genApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new mainWin(hInstance);
    }
};
//----------------------------------------------------------------------------
extern bool needMaximized();
//----------------------------------------------------------------------------
//#define USE_SHOW_TRACE
#ifdef USE_SHOW_TRACE
  #define SHOW_TRACE(a, b) show_trace(a, b)
  bool show_trace(LPCTSTR msg1, LPCTSTR msg2)
  {
    TCHAR t[4096];
    wsprintf(t, _T("%s - %s"), msg1, msg2);
    MessageBox(0, t, _T("Error"), MB_OK | MB_ICONSTOP);
    return false;
  }
#else
  #define SHOW_TRACE(a, b) false
#endif
//----------------------------------------------------------------------------
static HMODULE hsvBase;
//----------------------------------------------------------------------------
#define CUR_SV_BASE_VER 0x01000001
#define DIM_FAKE_JMP 8
#define DIM_KEY_CRYPT DIM_svKEY
//----------------------------------------------------------------------------
#define OFFS_SV_CHKSUM_VER 8
#define OFFS_SV_CHKSUM (OFFS_SV_CHKSUM_VER + 4)
#define V_7001 0x01000001
#define V_7002 0x01000002
//----------------------------------------------------------------------------
#define KEY_BASE_CRYPT 11235813
#define DIM_KEY_CRYPT  DIM_svKEY
//----------------------------------------------------------------------------
static BYTE decrypted_buff[DIM_KEY_CRYPT * 2 + 2];
static bool Crypted;
bool isCrypted() { return Crypted; }
LPCBYTE getDecryptCode() { return decrypted_buff; }
//----------------------------------------------------------------------------
//#define MATCH "@#@#§§#@"
static BYTE MATCH[] = { '@', '#' + 1, '@' + 2, '#' + 3, '§' + 4, '§' + 5, '#' + 6, '@' + 7 };
//----------------------------------------------------------------------------
static bool verifyMatch(LPBYTE buff)
{
  for(uint i = 1; i < sizeof(MATCH); ++i)
    if(buff[i] != MATCH[i] - i)
      return false;
  return true;
}
//----------------------------------------------------------------------------
static bool findMatch(uint& pos, LPBYTE buff, uint len)
{
  for(uint i = 0; i < len; ++i) {
    if(MATCH[0] == buff[i]) {
      if(verifyMatch(buff + i)) {
        pos = i + sizeof(MATCH);
        return true;
        }
      }
    }
  return false;
}
//----------------------------------------------------------------------------
static DWORD makeChk(LPDWORD buff, uint pos)
{
  #define DIM_4_CRC pos
  #define DIM_STEP_CRC 1024
  #define DIM_DW_CRC (DIM_4_CRC / sizeof(DWORD))
  #define MAX_STEP (DIM_DW_CRC / DIM_STEP_CRC + 2)

  uint i = 0;
  LPDWORD step = new DWORD[MAX_STEP];
  for(uint j = 0; j < MAX_STEP; ++j)
    step[j] = 0;
  for(uint j = 0; j < MAX_STEP; ++j) {
    for(uint k = 0; k < DIM_STEP_CRC && i < DIM_DW_CRC; ++i, ++k)
      step[j] += buff[i];
    }
  DWORD k = 0;
  for(uint j = 0; j < MAX_STEP; ++j)
    k += step[j];
  delete []step;
  return k;
}
//----------------------------------------------------------------------------
static DWORD getChecksum(LPCTSTR path)
{
  P_File pf(path, P_READ_ONLY);
  DWORD calc = 0;
  if(pf.P_open()) {
    uint len = (uint)pf.get_len();
    LPBYTE buff = new BYTE[len];
    pf.P_read(buff, len);
    uint pos;
    bool found = findMatch(pos, buff, len);
    if(found) {
      pos += OFFS_SV_CHKSUM;
      calc = makeChk((LPDWORD)buff, pos);
      }
    delete []buff;
    }
  return calc;
}
//----------------------------------------------------------------------------
static FARPROC getProc(HINSTANCE hI, LPCSTR name)
{
  // prima prova con l'underscore
  FARPROC f = GetProcAddress(hI, name);
  if(!f)
    // prova senza underscore
    f = GetProcAddress(hI, name + 1);
  return f;
}
//----------------------------------------------------------------------------
bool hasKey();
//----------------------------------------------------------------------------
static void fakeFz(bool& success)
{
  success = hasKey();
}
//----------------------------------------------------------------------------
static bool performCheckCode(HMODULE hMod)
{
  Crypted = false;
  bool success = false;
  while(!success) {
    int offs = (GetTickCount() & 0xfc) | 8;
    typedef void (*SV_Fake)(bool&);
    BYTE cryptCode[DIM_KEY_CRYPT + 2 + DIM_FAKE_JMP];
    DWORD k = GetTickCount();
    LPBYTE jmpFake = (LPBYTE)fakeFz;
    *(LPDWORD)cryptCode = k;


    typedef void (*SV_Base)(LPBYTE buff);
    SV_Base bs = (SV_Base) getProc(hMod, "_SV_Ver");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Ver)"), _T("failed"));
      break;
      }
    bs(cryptCode);
    *(LPDWORD)cryptCode ^= k;
    if(CUR_SV_BASE_VER > *(LPDWORD)cryptCode) {
      SHOW_TRACE(_T("cryptCode Ver"), _T("failed"));
      break;
      }
    DWORD ver = *(LPDWORD)cryptCode;

    bs = (SV_Base) getProc(hMod, "_SV_Base");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Base)"), _T("failed"));
      break;
      }

    jmpFake += offs;

    LPDWORD pdw = (LPDWORD)cryptCode;
    k = GetTickCount();
    pdw[0] = k;
    pdw[2] = offs ^ ver;
    pdw[3] = k;
    bs(cryptCode);
    pdw[0] ^= k;
    pdw[1] ^= ~k;

#if 0
// per creare la chiave vuota
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i)
      cryptCode[i] = 0x20;
#endif
    k += getChecksum(NAME_SVBASE);
    crypt(cryptCode, decrypted_buff, KEY_BASE_CRYPT, DIM_KEY_CRYPT);
    ((SV_Fake)(jmpFake - pdw[3] + k))(success);
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i) {
      if(decrypted_buff[i] != 0x20) {
        Crypted = true;
        break;
        }
      }
    break;
    }
  return success;
}
//----------------------------------------------------------------------------
bool hasKey()
{
#ifdef NO_USE_KEY
  return true;
#else  
  if(hsvBase)
    return true;
  bool success = false;
  while(!success) {
    hsvBase = LoadLibrary(NAME_SVBASE);
    if(!hsvBase) {
      SHOW_TRACE(_T("loadlibrary"), _T("failed"));
      break;
      }
    success = performCheckCode(hsvBase);
    break;
    }
  return success;
#endif
}
//---------------------------------------------------------------------
void releaseSvBase()
{
  if(!hsvBase)
    return;
  FreeLibrary(hsvBase);
  hsvBase = 0;
}
//---------------------------------------------------------------------
bool getKeyCode(LPBYTE key)
{
  ZeroMemory(key, DIM_svKEY);
  if(Crypted)
    memcpy(key, getDecryptCode(), DIM_svKEY);
  return true;
}
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
  if(!hasKey()) {
    releaseSvBase();
    return EXIT_FAILURE;
    }
  _control87(MCW_EM,MCW_EM);
  InitCommonControls();
  if(needMaximized())
    nCmdShow = SW_SHOWMAXIMIZED;
  int result= genApp(hInstance, nCmdShow).run(_T("Appl"));
  return result;
}
