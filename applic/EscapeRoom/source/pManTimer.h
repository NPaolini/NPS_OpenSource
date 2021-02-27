//-------------------- pManTimer.h ---------------------------
//-----------------------------------------------------------
#ifndef pManTimer_H_
#define pManTimer_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
class manageTimer
{
  public:
    manageTimer() : currVal(0) {}
    ~manageTimer() {}
    int getValue() const { return currVal; }
  private:
    void addRefresh(PWin* w);
    void calcNow();
    PVect<PWin*> setRefresh;
    void refreshAll();
    void refresh(uint ix);
    void reset();
    int currVal;
    friend class mainWin;
};
//-----------------------------------------------------------
#endif
