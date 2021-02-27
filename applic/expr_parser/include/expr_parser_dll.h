//-------- expr_parser_dll.h ---------------------------------------
#ifndef expr_parser_dll_H_
#define expr_parser_dll_H_
//------------------------------------------------------------------
#include "precompHeader_expr.h"
//------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------
#if defined(_MSC_VER)
  #pragma pack(push, 4)

#else
  #error It is necessary to define the 4-byte alignment for the following structures
#endif
//------------------------------------------------------------------
DECLARE_HANDLE(HANDLE_PARSER);
//------------------------------------------------------------------
enum infoTypeVar {  itv_real, itv_Int };
//------------------------------------------------------------------
typedef struct tag_InfoVar
{
  union {
    double d;
    __int64 i;
    };
  int16_t type; // 0 -> real, 1 -> integer

  // integer code assigned internally to the variable varName. It could serve to speed up search operations, for id instead of by name
  int16_t id;

  int32_t offset; //in the case in the formula is used as array, this is the relative index
  LPCWSTR varName;
} ParserInfo, *pParserInfo;
//------------------------------------------------------------------
typedef struct tag_BuffParserA
{
  LPCSTR buff;
  // if len == 0, buff is the name of the file, otherwise it is the buffer size already filled
  int32_t len;
} BuffParserA, *pBuffParserA;
//------------------------------------------------------------------
typedef struct tag_BuffParserW
{
  LPCWSTR buff;
  int32_t len;
} BuffParserW, *pBuffParserW;
//------------------------------------------------------------------
typedef struct tag_Error
{
  int16_t code;
  int16_t col;
  int16_t row;
  LPCWSTR varName;
  LPCWSTR functName;
} ParserError, *pParserError;
//----------------------------------------------------------------------------
// parameters for variables: code, pParserInfo, customData of InfoCallBack
// parameters for errors: code, pParserError, customData of InfoCallBack
// parameters for destroy: code, HANDLE_PARSER, customData of InfoCallBack
typedef LRESULT (*tCallBack)(uint32_t msg, LPVOID param, LPVOID custom);
//----------------------------------------------------------------------------
typedef struct tag_infoCallBack
{
  tCallBack fz_callBack;
  LPVOID customData;
}  InfoCallBack, *pInfoCallBack;
//----------------------------------------------------------------------------
//
typedef struct tag_precision
{
  double value; // set internally, by default, to 0.00001. It is a global variable, valid for all instances
}  InfoPrecision, *pInfoPrecision;
//------------------------------------------------------------------
#define ERR_EXPR_VAR   1
#define ERR_EXPR_PARAM 2
#define ERR_EXPR_VAR_NOT_FOUND 3
#define ERR_EXPR_DUP_LABEL 4
//------------------------------------------------------------------
#define cb_MSG_GET_VAR 1
#define cb_MSG_SET_VAR 2
#define cb_MSG_ERROR   3

// if CustomData of 'InfoCallBack' must be liberated you can do it in callBack who immediately called before deleting all
#define cb_MSG_DESTROY 10
//------------------------------------------------------------------
enum ParserCode {
  cNotCode,
  cMakeHandle,  // LRESULT = handle, p1 = pInfoCallBack, HANDLE_PARSER = 1
  cParseA,  // LRESULT = success, p1 = BuffParser, HANDLE_PARSER
  cPerform,      // LRESULT = operation result, p1 = 0,  HANDLE_PARSER
  cPrecisionGet, // LRESULT = always 0, p1 = pInfoPrecision,  HANDLE_PARSER
  cPrecisionSet,

  // destroys the object and destroys the handle is no longer valid
  cDestroy,      // LRESULT = operation result, p1 = 0, HANDLE_PARSER

// differentiated parts for unicode
  cParseW,

  };
//------------------------------------------------------------------
#ifndef UNICODE
  #define cParse cParseA
  #define BuffParser BuffParserA
  #define pBuffParser pBuffParserA
#else
  #define cParse cParseW
  #define BuffParser BuffParserW
  #define pBuffParser pBuffParserW
#endif
//------------------------------------------------------------------
#ifdef MAKE_PARSER_DLL
//------------------------------------------------------------------
PARSER_IMPORT_EXPORT
LRESULT ParserProcDLL(uint codeMsg, LPVOID p1, HANDLE_PARSER pParser);
extern double d_PRECISION;
//------------------------------------------------------------------
#else
//------------------------------------------------------------------
typedef LRESULT (*prtParserProcDLL)(uint codeMsg, LPVOID p1, HANDLE_PARSER pParser);
//------------------------------------------------------------------
#endif
//------------------------------------------------------------------
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//------------------------------------------------------------------
#ifdef __cplusplus
 }
#endif
//------------------------------------------------------------------
#endif
