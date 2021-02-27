//--------------- p_simpleXmlParser.h ---------------------
#ifndef p_simpleXmlParser_H_
#define p_simpleXmlParser_H_
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
struct xmlAttribute
{
  xmlAttribute(LPCWSTR tag = 0, LPCWSTR value = 0) : tagName(tag), Value(value) {}
  LPCWSTR tagName;
  LPCWSTR Value;
};
//---------------------------------------------------------
class xmlNode
{
  public:
    xmlNode(xmlNode* parent, LPCWSTR name) : Parent(parent), mySelf(name), Index(0) {}
    ~xmlNode() { flushPV(Childs); }

    LPCWSTR getName() const { return mySelf.tagName; }
    uint getIndex() const { return Index; }
    const PVect<xmlAttribute>& getAttribute() const { return Attribute; }
    LPCWSTR getValue()  const { return mySelf.Value; }
    LPCWSTR getValue(LPCWSTR tagAttr) const;

    bool getValue(LPCWSTR tagAttr, bool& result) const;
    bool getValue(LPCWSTR tagAttr, double& result) const;
    bool getValue(LPCWSTR tagAttr, int& result) const;

    bool getValue(bool& result) const;
    bool getValue(double& result) const;
    bool getValue(int& result) const;

    bool isEqu(LPCWSTR tagAttr, LPCWSTR match, bool& result) const;
    bool isEqu(LPCWSTR match, bool& result) const;

    const xmlNode* getParent() const { return Parent; }
    const xmlNode* getFirstChild() const;
    const xmlNode* getNext() const;
    const xmlNode* getPrev() const;
    const xmlNode* getChild(LPCWSTR name) const;

  private:
    xmlNode* Parent;
    PVect<xmlNode*> Childs;
    PVect<xmlAttribute> Attribute;
    xmlAttribute mySelf;
    uint Index;

    void setName(LPCWSTR name) { mySelf.tagName = name; }
    void setIndex(uint ix) { Index = ix; }
    void setValue(LPWSTR v) { mySelf.Value = v; }
    void setChild(xmlNode* node, uint ix) { Childs[ix] = node; }
    PVect<xmlAttribute>& getAttribute() { return Attribute; }

    friend class simpleXmlParser;
};
//---------------------------------------------------------
typedef const xmlNode* pXmlNodeC;
//---------------------------------------------------------
class simpleXmlParser
{
  public:
    simpleXmlParser() :
        Root(0), currPos(0), level(0), Buff(0), needDelete(true) {}
    ~simpleXmlParser() { delete Root; if(needDelete) delete []Buff; }

    bool parse(LPWSTR text, bool canUse = false);
    bool parse(LPCSTR text);
    pXmlNodeC getRoot() const { return Root; }
    static bool needConvertPoint() { return ConvertPoint; }
    static void setConvertPoint(bool set) { ConvertPoint = set; }
  private:
    xmlNode* Root;
    uint currPos;
    uint level;
    LPWSTR Buff;
    bool needDelete;
    bool startParse();
    xmlNode* performParse(xmlNode* parent);
    static bool ConvertPoint;
};
//----------------------------------------------------------
#endif
