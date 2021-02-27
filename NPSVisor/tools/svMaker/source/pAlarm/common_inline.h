//----------- common_inline.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifdef pAl_common_H_
#ifndef common_inline_H_
#define common_inline_H_
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void PRow_Alarm::getDescr(LPTSTR buff, size_t sz) const
{
  GetWindowText(*Descr, buff, sz);
}
//----------------------------------------------------------------------------
inline
void PRow_Alarm::invalidateNum(PEditEmpty* ctrl) const
{
  ctrl->invalidateNum();
}
//----------------------------------------------------------------------------
inline
void PRow_Alarm::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
  invalidateNum(Descr);
}
//----------------------------------------------------------------------------
inline DWORD PRow_Alarm::getGroup() {   return getGen(Group); }
//----------------------------------------------------------------------------
inline void PRow_Alarm::setGroup(DWORD grp) {   setGen(Group, grp); }
//----------------------------------------------------------------------------
inline DWORD PRow_Alarm::getFilter() {  return getGen(Filter); }
//----------------------------------------------------------------------------
inline void PRow_Alarm::setFilter(DWORD flt) {  setGen(Filter, flt); }
//----------------------------------------------------------------------------
inline
void PRow_Alarm::setAssoc(LPCTSTR p) const
{
  SetWindowText(*Assoc, p);
}
//----------------------------------------------------------------------------
inline void PRow_Alarm::getAll(DWORD& group, DWORD& filter)
{
  group = getGroup();
  filter = getFilter();
}
//----------------------------------------------------------------------------
inline
void PRow_Alarm::setAll(DWORD group, DWORD filter, LPCTSTR assoc)
{
  setGroup(group);
  setFilter(filter);
  setAssoc(assoc);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc::getPrph() {  return getGen(Prph); }
//----------------------------------------------------------------------------
inline void PRow_Assoc::setPrph(DWORD prph) {   setGen(Prph, prph); }
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc::getAddr() {  return getGen(Addr); }
//----------------------------------------------------------------------------
inline void PRow_Assoc::setAddr(DWORD addr) {   setGen(Addr, addr); }
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc::getType() {  return getGen(Type); }
//----------------------------------------------------------------------------
inline void PRow_Assoc::setType(DWORD type) {   setGen(Type, type); }
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc::getNBit() {  return getGen(nBit); }
//----------------------------------------------------------------------------
inline void PRow_Assoc::setNBit(DWORD nbit) {   setGen(nBit, nbit); }
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc::getOffs() {  return getGen(Offs); }
//----------------------------------------------------------------------------
inline void PRow_Assoc::setOffs(DWORD offs) {   setGen(Offs, offs); }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void PRow_Assoc_File::getDescr(LPTSTR buff, size_t sz) const
{
  GetWindowText(*Descr, buff, sz);
}
//----------------------------------------------------------------------------
inline
void PRow_Assoc_File::invalidateNum(PEditEmpty* ctrl) const
{
  ctrl->invalidateNum();
}
//----------------------------------------------------------------------------
inline
void PRow_Assoc_File::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
  invalidateNum(Descr);
}
//----------------------------------------------------------------------------
inline DWORD PRow_Assoc_File::getValue() {   return getGen(Value); }
//----------------------------------------------------------------------------
inline void PRow_Assoc_File::setValue(DWORD val) {   setGen(Value, val); }
//----------------------------------------------------------------------------
#endif
#endif
