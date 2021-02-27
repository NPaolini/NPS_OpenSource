//--------------- simpleXmlParser.cpp ----------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "p_simpleXmlParser.h"
#include "p_util.h"
#include "p_txt.h"
//----------------------------------------------------------
const xmlNode* xmlNode::getFirstChild() const
{
  if(Childs.getElem())
    return Childs[0];
  return 0;
}
//----------------------------------------------------------
const xmlNode* xmlNode::getNext() const
{
  if(!Parent)
    return 0;
  uint nElem = Parent->Childs.getElem();
  if(Index + 1 >= nElem)
    return 0;
  return Parent->Childs[Index + 1];
}
//----------------------------------------------------------
const xmlNode* xmlNode::getPrev() const
{
  if(!Parent || !Index)
    return 0;
  return Parent->Childs[Index - 1];
}
//----------------------------------------------------------
const xmlNode* xmlNode::getChild(LPCWSTR name) const
{
  uint nElem = Childs.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_wcsicmp(name, Childs[i]->getName()))
      return Childs[i];
  return 0;
}
//---------------------------------------------------------
LPCWSTR xmlNode::getValue(LPCWSTR tagAttr) const
{
  uint nElem = Attribute.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_wcsicmp(tagAttr, Attribute[i].tagName))
      return Attribute[i].Value;
  return 0;
}
//---------------------------------------------------------
bool xmlNode::isEqu(LPCWSTR tagAttr, LPCWSTR match, bool& result) const
{
  uint nElem = Attribute.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_wcsicmp(tagAttr, Attribute[i].tagName)) {
      result = !_wcsicmp(Attribute[i].Value, match);
      return true;
      }
  result = false;
  return false;
}
//---------------------------------------------------------
bool xmlNode::isEqu(LPCWSTR match, bool& result) const
{
  if(mySelf.Value) {
    result = !_wcsicmp(mySelf.Value, match);
    return true;
    }
  result = false;
  return false;
}
//---------------------------------------------------------
bool xmlNode::getValue(LPCWSTR tagAttr, bool& result) const
{
  return isEqu(tagAttr, L"true", result);
}
//---------------------------------------------------------
#define MAX_LEN_CONVERT 128
//---------------------------------------------------------
static void convertPoint(LPWSTR t, LPCWSTR s)
{
  uint len = 0;
  while(*s) {
    if(++len >= MAX_LEN_CONVERT)
      break;
    switch(*s) {
      case L',':
        *t++ = L'.';
      case L'.':
        ++s;
        break;
      default:
        *t++ = *s++;
        break;
      }
    }
  *t = 0;
}
//---------------------------------------------------------
bool xmlNode::getValue(LPCWSTR tagAttr, double& result) const
{
  uint nElem = Attribute.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_wcsicmp(tagAttr, Attribute[i].tagName)) {
      if(simpleXmlParser::needConvertPoint()) {
        wchar_t tmp[MAX_LEN_CONVERT];
        convertPoint(tmp, Attribute[i].Value);
        result = _wtof(tmp);
        }
      else
        result = _wtof(Attribute[i].Value);
      return true;
      }
  result = 0;
  return false;
}
//---------------------------------------------------------
bool xmlNode::getValue(LPCWSTR tagAttr, int& result) const
{
  uint nElem = Attribute.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_wcsicmp(tagAttr, Attribute[i].tagName)) {
      // non dovrebbe essere necessario, ma potrebbe esserci un intero con il punto delle migliaia
      if(simpleXmlParser::needConvertPoint()) {
        wchar_t tmp[MAX_LEN_CONVERT];
        convertPoint(tmp, Attribute[i].Value);
        result = _wtoi(tmp);
        }
      else
        result = _wtoi(Attribute[i].Value);
      return true;
      }
  result = 0;
  return false;
}
//---------------------------------------------------------
bool xmlNode::getValue(bool& result) const
{
  return isEqu(L"true", result);
}
//---------------------------------------------------------
bool xmlNode::getValue(double& result) const
{
  if(mySelf.Value) {
    if(simpleXmlParser::needConvertPoint()) {
      wchar_t tmp[MAX_LEN_CONVERT];
      convertPoint(tmp, mySelf.Value);
      result = _wtof(tmp);
      }
    else
      result = _wtof(mySelf.Value);
    return true;
    }
  result = 0;
  return false;
}
//---------------------------------------------------------
bool xmlNode::getValue(int& result) const
{
  if(mySelf.Value) {
    if(simpleXmlParser::needConvertPoint()) {
      wchar_t tmp[MAX_LEN_CONVERT];
      convertPoint(tmp, mySelf.Value);
      result = _wtoi(tmp);
      }
    else
      result = _wtoi(mySelf.Value);
    return true;
    }
  result = 0;
  return false;
}
//---------------------------------------------------------
bool simpleXmlParser::ConvertPoint = true;
static wchar_t gRoot[] = L"root";
//---------------------------------------------------------
bool simpleXmlParser::parse(LPCSTR text)
{
  if(needDelete)
    delete []Buff;
  uint len = 0;
  Buff = makeWCHAR(len, text, CP_UTF8);
  needDelete = true;
  return startParse();
}
//---------------------------------------------------------
bool simpleXmlParser::parse(LPWSTR text, bool canUse)
{
  if(needDelete)
    delete []Buff;
  if(canUse) {
    needDelete = false;
    Buff = text;
    }
  else {
    needDelete = true;
    Buff = str_newdup(text);
    }
  return startParse();
}
//---------------------------------------------------------
bool simpleXmlParser::startParse()
{
  currPos = 0;
  level = 0;
  Root = new xmlNode(0, gRoot);
  uint nElem = 0;
  while(true) {
    xmlNode* child = performParse(Root);
    if(!child)
      break;
    child->setIndex(nElem);
    Root->setChild(child, nElem);
    ++nElem;
    }
  return toBool(Root->getFirstChild());
}
//---------------------------------------------------------
xmlNode* simpleXmlParser::performParse(xmlNode* parent)
{
  while(Buff[currPos] && Buff[currPos] != L'<')
    ++currPos;
  if(!Buff[currPos])
    return 0;

  ++currPos;
  // pulisce eventuali spazi (non dovrebbero esserci)
  while(Buff[currPos] && Buff[currPos] <= L' ')
    ++currPos;
  // è la chiusura di un nodo con figli
  if(Buff[currPos] == L'/') {
    while(Buff[currPos] && Buff[currPos] != L'>')
      ++currPos;
    return 0;
    }

  xmlNode* node = new xmlNode(parent, Buff + currPos);

  while(Buff[currPos] && Buff[currPos] != L' ' && Buff[currPos] != L'>' && Buff[currPos] != L'/')
    ++currPos;
  // memorizziamo il punto di termine stringa
  uint pos = currPos;
  // eliminiamo gli eventuali blank
  while(Buff[currPos] && Buff[currPos] <= L' ')
    ++currPos;

  bool hasParameter = true;
  bool finished = false;
  if(Buff[currPos] == L'>') {// è un blocco senza attributi
    hasParameter = false;
    ++currPos;
    while(Buff[currPos] && Buff[currPos] <= L' ')
      ++currPos;
    }
  else if(Buff[currPos] == L'/') {
    ++currPos;
    finished = true;
    }
  else if(!Buff[currPos]) { // ?? errore, cancelliamo il nodo e si torna zero
    delete node;
    return 0;
    }
  Buff[pos] = 0; // terminiamo il nome del nodo
  if(finished) {
    while(Buff[currPos] && Buff[currPos] != L'>')
      ++currPos;
    return node;
    }
  if(hasParameter) {
// cerchiamo i parametri
    PVect<xmlAttribute>& Attrib = node->getAttribute();
    while(Buff[currPos] && Buff[currPos] != L'>' && Buff[currPos] != L'/') {
      xmlAttribute attr(Buff + currPos);
      while(Buff[currPos] && Buff[currPos] != L' ' && Buff[currPos] != L'=')
        ++currPos;
      pos = currPos;
      while(Buff[currPos] && Buff[currPos] != L'=')
        ++currPos;
      ++currPos;
      Buff[pos] = 0;
      while(Buff[currPos] && Buff[currPos] != L'\"')
        ++currPos;
      attr.Value = Buff + currPos + 1;
        ++currPos;
      while(Buff[currPos] && Buff[currPos] != L'\"')
        ++currPos;
      Buff[currPos] = 0;
      ++currPos;
      if(L'>' == Buff[currPos])
        finished = true;
      Attrib[Attrib.getElem()] = attr;
      ++currPos;
//      if(finished)
//        break;
      while(Buff[currPos] && Buff[currPos] <= L' ')
        ++currPos;
      if(finished)
        break;
      }
    // finiti gli argomenti, o è finita la stringa (errore) o c'è uno dei due caratteri '>', '/'
    if(Buff[currPos] == L'/') {
      while(Buff[currPos] && Buff[currPos] != L'>')
        ++currPos;
      return node;
      }
    else if(!Buff[currPos]) { // ?? errore, cancelliamo il nodo e si torna zero
      delete node;
      return 0;
      }
    }
// ci sono nodi figlio o il valore

// è il valore, lo salviamo e scorriamo fino al termine del nodo
  if(L'<' !=  Buff[currPos]) {
    node->setValue(Buff + currPos);
    while(L'<' != Buff[currPos])
      ++currPos;
    Buff[currPos] = 0;
    ++currPos;
    while(L'>' != Buff[currPos])
      ++currPos;
    return node;
    }

  ++level;
  uint nElem = 0;
  while(true) {
    xmlNode* child = performParse(node);
    if(!child)
      break;
    child->setIndex(nElem);
    node->setChild(child, nElem);
    ++nElem;
    }
  --level;
  return node;
}
//---------------------------------------------------------
