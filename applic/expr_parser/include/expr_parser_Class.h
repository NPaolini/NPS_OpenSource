//---------- expr_parser_Class.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef expr_parser_Class_H_
#define expr_parser_Class_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "expr_parser_dll.h"
#include "p_util.h"
#include "p_file.h"
//----------------------------------------------------------------------------
//#define USE_DUAL_VECT
//----------------------------------------------------------------------------
class P_ExprMake
{
  public:
    P_ExprMake();
    ~P_ExprMake();

    bool addVar(const ParserInfo& v);
    bool remVar(LPCTSTR varname);

    bool addCalcRow(LPCTSTR row);
    bool deleteRow(uint ix);
    LPCTSTR getRow(uint ix);

    bool addFunctName(uint id, LPCTSTR name);
    bool addCalcRowFunct(uint id, LPCTSTR row);
    bool deleteRowFunct(uint id, uint ix);
    LPCTSTR getRowFunct(uint id, uint ix);

    void setCallBack(const InfoCallBack& i_c_b) { Icb = i_c_b; }
    bool make();
    bool make(LPCTSTR filename, bool canDelete);

    void flushAll();
#ifdef USE_DUAL_VECT
    struct offsetValues
    {
      PVect<__int64> Values;
      LPCWSTR varName;
      offsetValues(LPCWSTR varName = 0) : varName(varName) {}
    };
#endif
  protected:
    PVect<ParserInfo> Vars;
#ifdef USE_DUAL_VECT
    // to hold values with offset > 0
    PVect<offsetValues> Values;
#endif
    PVect<LPCTSTR> Calc;
    InfoCallBack Icb;
    LPCTSTR Prepared;
    uint DimBuff;
    bool canDeleteFile;

    void clearPrepared();

    struct funct
    {
      LPCTSTR name_parameter;
      uint id;
      PVect<LPCTSTR> Calc;
      funct() : name_parameter(0), id(0) {}
    };
    PVect<funct> Funct;

    funct* getFunct(uint id);

    void writeVars(P_File& pf);
    void writeFunct(P_File& pf);
    void writeCalc(P_File& pf);

    bool parse();
    bool perform();
  private:
    HANDLE_PARSER Parser;
    friend class P_ExprManageBase;

    NO_COPY_COSTR_OPER(P_ExprMake)
};
//----------------------------------------------------------------------------
class P_ExprManageBase
{
  public:
    P_ExprManageBase(P_ExprMake& emk);
    virtual ~P_ExprManageBase() {}

    virtual bool parse();
    virtual bool perform() { return EMK.perform(); }

  protected:
    friend LRESULT callBackBase(uint32_t msg, LPVOID param, LPVOID custom);
    P_ExprMake& EMK;
    InfoCallBack Icb;

    // need for derived classes to find the variables and read/write values
    // e.g. to set values before calling the perform ()
    PVect<ParserInfo>& getVars() { return EMK.Vars; }
#ifdef USE_DUAL_VECT
    PVect<P_ExprMake::offsetValues>& getValues() { return EMK.Values; }
#endif
    virtual LRESULT cbGetVar(pParserInfo pi) = 0;
    virtual LRESULT cbSetVar(const ParserInfo* pi) = 0;
    virtual LRESULT cbError(const ParserError* pe) { return 0; }
    virtual LRESULT cbDestroy() { return 0; }
};
//----------------------------------------------------------------------------
class P_ExprManageVar : public P_ExprManageBase
{
  private:
    typedef P_ExprManageBase baseClass;
  public:
    P_ExprManageVar(P_ExprMake& emk) : baseClass(emk) {}
    virtual bool perform() { return baseClass::perform(); }
    virtual void sendAllVars();

  protected:
    virtual LRESULT cbGetVar(pParserInfo pi);
    virtual LRESULT cbSetVar(const ParserInfo* pi);
    virtual void makeRowOut(LPTSTR buff, size_t dim, const ParserInfo& pi);
    virtual void sendOut(LPCTSTR varValue) {}
};
//----------------------------------------------------------------------------
#endif
