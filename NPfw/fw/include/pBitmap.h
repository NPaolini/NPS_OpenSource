//-------------------- pBitmap.h ---------------------------
//-----------------------------------------------------------
#ifndef PBITMAP_H_
#define PBITMAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#pragma once
#include "pAppl.h"
#include "pDef.h"
#include "PCrt_lck.h"
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
typedef Gdiplus::Bitmap* p_gdiplusBitmap;
//-----------------------------------------------------------
class CGdiPlusBitmap
{
  public:
    p_gdiplusBitmap m_pBitmap;

  public:
    CGdiPlusBitmap() : m_pBitmap(0) { }
    CGdiPlusBitmap(LPCWSTR pFile);
    CGdiPlusBitmap(LPCSTR pFile);
    virtual ~CGdiPlusBitmap() { Empty(); }

    virtual void Empty();

    virtual bool Load(LPCWSTR pFile);
    virtual bool Load(LPCTSTR pName, HMODULE hInst, LPCTSTR typeId = 0) { return false; }
    virtual bool Load(UINT id, HMODULE hInst, LPCTSTR typeId = 0) { return false; }

    operator Gdiplus::Bitmap*() const { return m_pBitmap; }
};
//-----------------------------------------------------------
class CGdiPlusBitmapResource : public CGdiPlusBitmap
{
  private:
    typedef CGdiPlusBitmap baseClass;

  public:
    CGdiPlusBitmapResource() : m_hBuffer(0) { }
    CGdiPlusBitmapResource(LPCTSTR pName, HMODULE hInst, LPCTSTR typeId = 0);
    CGdiPlusBitmapResource(UINT id, HMODULE hInst, LPCTSTR typeId = 0);
    virtual ~CGdiPlusBitmapResource() { Empty(); }

    void Empty();

    bool Load(LPCTSTR pName, HMODULE hInst, LPCTSTR typeId = 0);
    bool Load(UINT id, HMODULE hInst, LPCTSTR typeId = 0);
  protected:
    HGLOBAL m_hBuffer;
};
//-----------------------------------------------------------
class PBitmap;
//-----------------------------------------------------------
class manageGif
{
  public:
    manageGif(PBitmap* owner, HWND hwnd, PVect<uint> delay_time) :
        Owner(owner), hEvClose(0), hThread(0), curr(0), Hwnd(hwnd), byThread(false), delayTime(delay_time) {}
    ~manageGif();
    bool run();
    void evPaint(const PRect& r);
  private:
    PBitmap* Owner;
    HWND Hwnd;
    HANDLE hThread;
    HANDLE hEvClose;
    criticalSect CS;
    uint curr;
    PVect<uint> delayTime;
    PRect Rect;
    bool byThread;
    friend unsigned FAR PASCAL AnimProc(void*);
    friend class PBitmap;
};
//-----------------------------------------------------------
class IMPORT_EXPORT PBitmap
{
  public:
    PBitmap();
   // from resource
    PBitmap(int id, HINSTANCE hinst, LPCTSTR typeId = 0);
    PBitmap(LPCTSTR nameId, HINSTANCE hinst, LPCTSTR typeId = 0);

    // from file
    PBitmap(LPCWSTR filename);
    PBitmap(LPCSTR filename);

    // from device
    PBitmap(HDC hdc, const POINT& pt, const SIZE& sz);

    // from allocated
    PBitmap(HBITMAP hbmp, bool autodelete = false);
    PBitmap(p_gdiplusBitmap pbmp, bool owner = true);

    // esegue una copia dei dati
    PBitmap(const PBitmap& pbmp);

    virtual ~PBitmap();

    virtual bool draw(HDC hdc, const POINT& pt) const;
    virtual bool draw(HDC hdc, const POINT& pt, const SIZE& sz) const;
    virtual bool drawSlice(HDC hdc, const PRect& target, const PRect& source) const;
    virtual bool drawTiled(HDC hdc, const PRect& target) const;
    virtual bool drawTransp(HDC hdc, const PRect& target) const;

    virtual bool draw(Gdiplus::Graphics& graphics, const POINT& pt) const;
    virtual bool draw(Gdiplus::Graphics& graphics, const POINT& pt, const SIZE& sz) const;
    virtual bool drawSlice(Gdiplus::Graphics& graphics, const PRect& target, const PRect& source) const;
    virtual bool drawTiled(Gdiplus::Graphics& graphics, const PRect& target) const;
    virtual bool drawTransp(Gdiplus::Graphics& graphics, const PRect& target) const;

    HBITMAP clone(HDC hdc = 0) const;
    bool clone(p_gdiplusBitmap& target);

    operator HBITMAP() const;
    const p_gdiplusBitmap getManBmp() const { return manGdiPlusBitmap ? manGdiPlusBitmap->m_pBitmap : 0; }

    const PBitmap& operator = (const PBitmap& bmp);

    // non verifica se l'handle sia valido. In caso
    // di mancato caricamento SIZE vale { 0, 0 }
    SIZE getSize() const;

    bool isValid() { return toBool(manGdiPlusBitmap) || hBmp; }

    bool getQualityOnStretch() const { return qualityOnStretch; }
    bool setQualityOnStretch(bool set);
    bool setSmooth(bool set);
    bool getSmooth() const { return smooth; }

    void chgHandle(HBITMAP new_hBmp);

    bool addGifControl(PWin* owner);
    bool addGifControl(HWND owner);
    void removeGifControl();
    bool hasGifControl() const { return toBool(ManGif); }

    bool saveAs(LPCSTR filename, uint quality = 75);
    bool saveAs(LPCWSTR filename, uint quality = 75);

    static PBitmap* rotate(const PBitmap* bmp, Gdiplus::RotateFlipType type = Gdiplus::Rotate270FlipNone);
    static void rotateInside(PBitmap* bmp, Gdiplus::RotateFlipType type = Gdiplus::Rotate270FlipNone);
    static PBitmap* resizeBmp(const PBitmap* bmp, double perc);

  protected:
  private:
    mutable CGdiPlusBitmap* manGdiPlusBitmap;
    mutable HBITMAP hBmp;
    mutable bool autoDelete;
    bool qualityOnStretch;
    bool smooth;

    void InitializeVar(bool auto_delete = false);
    bool createFromHBitmap() const;
    friend class manageGif;
    manageGif* ManGif;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
