//--------- log_alrm.h -----------------------------------------
#ifndef LOG_ALRM_H_
#define LOG_ALRM_H_
#include "setPack.h"
//--------------------------------------------------------------
class status_alarm
{
  public:
    status_alarm();
    void init_alarm(const FILETIME& old_tot);
    void reset_alarm();
    void startTime();
    virtual bool stopTime(); // torna false quando non ci sono più allarmi attivi
    void getTotal(SYSTEMTIME *buff) { FileTimeToSystemTime(&total, buff); }
    void getTotal(FILETIME *buff) { *buff = total; }
  protected:
    unsigned active;
  private:
    FILETIME init;
    FILETIME total;
};
//--------------------------------------------------------------
class status_time : public status_alarm
{
  public:
    virtual bool stopTime() {
      if(active > 0) // non importa quanti start ha ricevuto, deve terminare
        active = 1;
      return status_alarm::stopTime();
      }
};
//--------------------------------------------------------------
inline
status_alarm::status_alarm()
{
  reset_alarm();
}
//---------------------------------------------------------------
inline
void status_alarm::init_alarm(const FILETIME& old)
{
  total = old;
}
//---------------------------------------------------------------
inline
void status_alarm::reset_alarm()
{
  active = 0;
  (reinterpret_cast<LARGE_INTEGER*>(&init))->QuadPart = 0;
  (reinterpret_cast<LARGE_INTEGER*>(&total))->QuadPart = 0;
}
//--------------------------------------------------------------
#include "restorePack.h"
#endif