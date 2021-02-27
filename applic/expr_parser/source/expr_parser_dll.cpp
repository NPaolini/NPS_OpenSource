//---------------- expr_parser_dll.cpp ----------------------
//-----------------------------------------------------------
#ifndef MAKE_PARSER_DLL
  #define MAKE_PARSER_DLL 1
#endif
//-----------------------------------------------------------
#include "precompHeader_expr.h"
//-----------------------------------------------------------
#include "expr_parser_dll.h"
#include "p_util.h"
#include "sv_manage_express.h"
//----------------------------------------------------------------------------
struct HandleParser
{
  svManExpression* pParser;
  InfoCallBack info;
  HandleParser() {   ZeroMemory(this, sizeof(*this)); }
};
//------------------------------------------------------------------
typedef HandleParser* pHandleParser;
//------------------------------------------------------------------
#ifdef UNICODE
//------------------------------------------------------------------
static bool parse(svManExpression* parser, BuffParserW& BuffParser)
{
  bool success = false;
  if(BuffParser.len)
    success = parser->makeExpressionByBuff(BuffParser.buff);
  else
    success = parser->makeExpression(BuffParser.buff);
  return success;
}
//------------------------------------------------------------------
static bool parse(svManExpression* parser, BuffParserA& BuffParser)
{
  bool success = false;
  if(BuffParser.len) {
    uint len = BuffParser.len;
    LPWSTR buff = makeWCHAR(len, BuffParser.buff);
    success = parser->makeExpressionByBuff(buff);
    }
  else {
    TCHAR buff[_MAX_PATH];
    copyStrZ(buff, BuffParser.buff);
    success = parser->makeExpression(buff);
    }
  return success;
}
//------------------------------------------------------------------
#else
//------------------------------------------------------------------
static bool parse(svManExpression* parser, BuffParserA& BuffParser)
{
  bool success = false;
  if(BuffParser.len)
    success = parser->makeExpressionByBuff(BuffParser.buff);
  else
    success = parser->makeExpression(BuffParser.buff);
  return success;
}
//------------------------------------------------------------------
static bool parse(svManExpression* parser, BuffParserW& BuffParser)
{
  bool success = false;
  if(BuffParser.len) {
    uint len = BuffParser.len;
    LPSTR buff = unmakeWCHAR(len, BuffParser.buff);
    success = parser->makeExpressionByBuff(buff);
    }
  else {
    char buff[_MAX_PATH];
    copyStrZ(buff, BuffParser.buff);
    success = parser->makeExpression(buff);
    }
  return success;
}
//------------------------------------------------------------------
#endif
//------------------------------------------------------------------
HANDLE_PARSER makeHandle(pInfoCallBack pFz)
{
  pHandleParser ph = new HandleParser;
  ph->info = *pFz;
  ph->pParser = new svManExpression(&ph->info);
  return (HANDLE_PARSER)ph;
}
//------------------------------------------------------------------
template <class T>
HANDLE_PARSER performParse(pHandleParser ph, T& BuffParser)
{
  return (HANDLE_PARSER) parse(ph->pParser, BuffParser);
}
//------------------------------------------------------------------
static LRESULT perform(pHandleParser hp)
{
  return (LRESULT)hp->pParser->performExpression();
}
//------------------------------------------------------------------
static LRESULT destroy(pHandleParser hp)
{
  uint32_t result = hp->info.fz_callBack(cDestroy, (HANDLE_PARSER)hp, hp->info.customData);
  delete hp->pParser;
  delete hp;
  return (LRESULT)result;
}
//------------------------------------------------------------------
static LRESULT getPrec(pInfoPrecision pip)
{
  pip->value = d_PRECISION;
  return 0;
}
//------------------------------------------------------------------
static LRESULT setPrec(pInfoPrecision pip)
{
  d_PRECISION = pip->value;
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PARSER_IMPORT_EXPORT
LRESULT ParserProcDLL(uint codeMsg, LPVOID p1, HANDLE_PARSER pParser)
{
  if(!pParser)
    return 0;
  switch(codeMsg) {
    case cMakeHandle:
      return (LRESULT)makeHandle((pInfoCallBack)p1);
    case cParseA:
      return (LRESULT)performParse((pHandleParser)pParser, *(pBuffParserA)p1);
    case cParseW:
      return (LRESULT)performParse((pHandleParser)pParser, *(pBuffParserW)p1);

    case cPerform:
      return perform((pHandleParser)pParser);

    case cPrecisionGet:
      return getPrec((pInfoPrecision)p1);
    case cPrecisionSet:
      return setPrec((pInfoPrecision)p1);
    case cDestroy:
      return destroy((pHandleParser)pParser);
    }
  return 0;
}
//------------------------------------------------------------------
