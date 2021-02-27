//---------- expr_parser_Class.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef expr_parser_Class_H_
#define expr_parser_Class_H_
//----------------------------------------------------------------------------
#include "expr_parser_dll.h"
//----------------------------------------------------------------------------
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std::string_literals;
//------------------------------------------------------------------
#define NO_COPY_COSTR_OPER_(a) \
  private:\
    a(const a&);\
    const a& operator=(const a&);
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define cout_t      std::wcout
  #define string_t    std::wstring
  #define ofstream_t  std::wofstream
  #define to_string_t std::to_wstring
  #define LPCWSTR_TO_STRING(buff, txt) { buff = string_t(txt); }
#else
  #define cout_t      std::cout
  #define string_t    std::string
  #define ofstream_t  std::ofstream
  #define to_string_t std::to_string
  #define LPCWSTR_TO_STRING(buff, txt) { std::wstring t(txt); buff = string_t(t.begin(), t.end()); }
#endif
//----------------------------------------------------------------------------
class P_ExprMake
{
  public:
    P_ExprMake();
    ~P_ExprMake();

    bool addVar(const ParserInfo& v);

    bool addCalcRow(const string_t& row);

    bool addFunctName(uint id, const string_t& name);
    bool addCalcRowFunct(uint id, const string_t& row);

    void setCallBack(const InfoCallBack& i_c_b) { Icb = i_c_b; }
    bool make();
    bool make(LPCTSTR filename, bool canDelete);

    void flushAll();
    struct funct
    {
      string_t name_parameter;
      uint id;
      mutable std::vector<string_t> Calc;
      funct(uint id, string_t name) : name_parameter(name), id(id) {}
      funct() : name_parameter(0), id(0) {}
    };
  protected:
    std::set<ParserInfo> Vars;
    std::vector<string_t> Calc;
    std::set<funct> Funct;
    InfoCallBack Icb;
    LPTSTR Prepared;
    uint DimBuff;
    bool canDeleteFile;
    void clearPrepared();
    void writeVars(ofstream_t& pf);
    void writeFunct(ofstream_t& pf);
    void writeCalc(ofstream_t& pf);

    bool parse();
    bool perform();
  private:
    HANDLE_PARSER Parser;
    friend class P_ExprManageBase;

    NO_COPY_COSTR_OPER_(P_ExprMake)

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
    std::set<ParserInfo>& getVars() { return EMK.Vars; }

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

    virtual void makeRowOut(string_t& buff, const ParserInfo& pi);
    virtual void sendOut(string_t varValue) {}
};
//----------------------------------------------------------------------------
#endif
