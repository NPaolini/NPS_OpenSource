//----------- log_oper.h -------------------------------
#ifndef LOG_OPER__
#define LOG_OPER__
//--------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//--------------------------------------------------------------
#include "log_stat.h"
//--------------------------------------------------------------
class val_oper : public val_gen
{
  public:
    val_oper() : oper(0), turno(0), mt(0), kg(0) {}
    BYTE oper;
    BYTE turno;
    DWDATA mt;
    DWDATA kg;
    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);
    virtual bool exportData(P_File &f);
    virtual bool exportHeader(P_File &f);
    const val_oper& operator=(const val_gen& other) { clone(other); return *this; }
    virtual void clone(const val_gen& other)
    {
      val_gen::clone(other);
      const val_oper* vo = dynamic_cast<const val_oper*>(&other);
      if(vo) {
        oper = vo->oper;
        turno = vo->turno;
        mt = vo->mt;
        kg = vo->kg;
        }
    }
};
//--------------------------------------------------------------
class stat_oper_turn : public stat_gen
{
  public:
    virtual ~stat_oper_turn() {  }
    void set_data(const val_gen* val) { data = *val; }
    const val_oper& get_data() { return data; }
    virtual int save();
    virtual int init();
    virtual int load(const val_oper*) = 0;
    virtual bool exportData(P_File &f);
    void setFile(LPCTSTR newName) { _tcscpy_s(File, SIZE_A(File), newName); }
    void fillTotalTime(FILETIME *ft, int fromDay, int toDay);
    int getCod(LPCTSTR filename);
    void getCod(LPTSTR buff, LPCTSTR filename);
    virtual log_status::errEvent setEvent(const val_gen* = 0);
  protected:
    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);
    virtual bool makeFileName() = 0;

    val_oper data;
    TCHAR File[_MAX_PATH];
};
//--------------------------------------------------------------
//--------------------------------------------------------------
class stat_oper : public stat_oper_turn
{
  public:
    virtual int load(const val_oper*);
  protected:
    virtual bool makeFileName();
};
//--------------------------------------------------------------
//--------------------------------------------------------------
class stat_turn : public stat_oper_turn
{
  public:
    virtual int load(const val_oper*);
  protected:
    virtual bool makeFileName();
};
//--------------------------------------------------------------
//--------------------------------------------------------------
stat_gen *allocOper();
stat_gen *allocTurn();
//--------------------------------------------------------------
//--------------------------------------------------------------
#endif



