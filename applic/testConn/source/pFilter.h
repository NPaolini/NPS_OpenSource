//------------------ PFilter.h -------------------------------
//------------------------------------------------------------
#ifndef PFILTER_H_
#define PFILTER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class myBinaryFilter : public PBinaryFilter
{
	public:
		virtual void initCtrl(HWND ctrl);
	private:
		typedef PBinaryFilter baseClass;
};
//----------------------------------------------------------------------------
class myHexFilter : public PHexFilter
{
	public:
		virtual void initCtrl(HWND ctrl);
	private:
		typedef PHexFilter baseClass;
};
//----------------------------------------------------------------------------
class myNumbFilter : public PNumbFilter
{
	public:
		virtual void initCtrl(HWND ctrl);
    bool accept(WPARAM& key, HWND ctrl);
	private:
		typedef PNumbFilter baseClass;
};
//----------------------------------------------------------------------------
#endif
