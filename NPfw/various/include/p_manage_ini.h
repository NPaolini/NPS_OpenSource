//------- p_manage_ini.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef manageIni_H_
#define manageIni_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_vect.h"
#include "p_file.h"
#include "p_util.h"
//----------------------------------------------------------------------------
struct ini_param {
  LPCTSTR name;
  LPCTSTR value;
  ini_param(LPCTSTR name = 0, LPCTSTR value = 0) : name(name), value(value) {}
  ~ini_param() { }

  void destroy();
};
//----------------------------------------------------------------------------
struct ini_block {
  LPCTSTR name;
  PVect<ini_param> items;
  ini_block() : name(0) {}
  ~ini_block() { }

  void destroy();
};
//----------------------------------------------------------------------------
class manageIni
{
  public:
    manageIni() : Filename(0), lastErr(0) {}
    manageIni(LPCTSTR filename) : Filename(str_newdup(filename)), lastErr(0) {}
    virtual ~manageIni() { flush(); delete Filename; }

    void getBlockList(PVect<LPCTSTR>& target);
    void getParamList(LPCTSTR blockName, PVect<ini_param>& target);
    void getParamList(LPCTSTR blockName, PVect<LPCTSTR>& target);

    bool existBlock(LPCTSTR blockName);
    bool parse(LPCTSTR filename = 0);

    bool createBlock(LPCTSTR name);
    bool createParam(const ini_param& param, LPCTSTR blockName);

    bool changeValue(const ini_param& param, LPCTSTR blockName);
    bool deleteParam(LPCTSTR paramName, LPCTSTR blockName);
    LPCTSTR getValue(LPCTSTR paramName, LPCTSTR blockName);
    bool renameParam(LPCTSTR blockName, LPCTSTR paramName, LPCTSTR oldParamName);

    // se non esiste il blocco lo crea, se esiste il param ne cambia il value, altrimenti lo aggiunge
    bool addParam(const ini_param& param, LPCTSTR blockName);

    bool renameBlock(LPCTSTR blockName, LPCTSTR oldBlockName);
    bool deleteBlock(LPCTSTR blockName);

    bool save(LPCTSTR filename = 0);

    void flush();

    DWORD getLastErr() { return lastErr; }
    enum lastE { NoErr, le_NotFoundBlock, le_Not_FoundParam, le_Exists_Block, le_Exists_Param };
  protected:
    PVect<ini_block> Ini;
    LPCTSTR Filename;

    ini_block* findBlock(LPCTSTR name, bool canCreate = false);
    ini_param* findParam(LPCTSTR blockName, LPCTSTR paramName);
    bool parse_add_block(LPTSTR blockName, LPTSTR row);
    bool parse_add_row(LPCTSTR blockName, LPTSTR row);

    void saveRows(P_File& pf, const PVect<ini_param>& rows);
    DWORD lastErr;

  NO_COPY_COSTR_OPER(manageIni)
};
#endif
