#ifndef VAL_ORD__
#define VAL_ORD__
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "gestdata.h"
//---------------------------------------------------------------------
#define SIZE_NAME_FILE 20
//---------------------------------------------------------------------
class val_order : public val_gen
{
  public:
    val_order();
    ordStat status;

    TCHAR name[SIZE_NAME_FILE];
    TCHAR cod_client[SIZE_NAME_FILE];

    FILETIME startDate;
    FILETIME endDate;

    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);
    virtual bool exportData(P_File &f);
    virtual bool exportHeader(P_File &f);
    const val_order& operator=(const val_gen& other) { clone(other); return *this; }
    virtual void clone(const val_gen& other)
    {
      val_gen::clone(other);
      const val_order* vo = dynamic_cast<const val_order*>(&other);
      if(vo) {
        status = vo->status;
        startDate = vo->startDate;
        endDate = vo->endDate;
        _tcscpy_s(name, vo->name);
        _tcscpy_s(cod_client, vo->cod_client);
        }
    }
};
#endif
