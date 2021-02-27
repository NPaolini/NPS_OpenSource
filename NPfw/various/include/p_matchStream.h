//--------------- p_matchStream.h --------------------------------
#ifndef p_matchStream_H_
#define p_matchStream_H_
//----------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------
template <typename T, int maxDimMatch=256>
class p_matchStream
{
  public:
    p_matchStream() { }
    virtual ~p_matchStream() { }
    typedef const T* cTp;
    virtual int checkMatch(cTp match, int dim);
  protected:
    virtual bool has_data() = 0;
    virtual void read_one_data(T& buff) = 0;
    virtual T converter(T readed) { return readed; }
  private:
    bool is_equ(cTp p1, cTp p2, int len);
    int checkPrevPos(cTp data_flow, int len, cTp match);
};
//----------------------------------------------------------------
template <typename T, int maxDimMatch>
int p_matchStream<T, maxDimMatch>::checkMatch(cTp match, int dim)
{
  T t[maxDimMatch];
  uint pos = 0;
  for(int i = 0;; ++i) {
    if(!has_data())
      return -1;
    read_one_data(t[pos]);
    t[pos] = converter(t[pos]);
    if(t[pos] == match[pos]) {
      ++pos;
      if(pos == dim)
        return i - pos + 1;
      }
    else if(pos)
      pos = checkPrevPos(t + 1, pos, match);
    }
  return -1;
}
//----------------------------------------------------------------
template <typename T, int maxDimMatch>
bool p_matchStream<T, maxDimMatch>::is_equ(cTp p1, cTp p2, int len)
{
  for(int i = 0; i < len; ++i)
    if(p1[i] != p2[i])
      return false;
  return true;
}
//----------------------------------------------------------------
template <typename T, int maxDimMatch>
int p_matchStream<T, maxDimMatch>::checkPrevPos(cTp data_flow, int len, cTp match)
{
  while(len > 0) {
    if(is_equ(data_flow, match, len))
      return len;
    --len;
    ++data_flow;
    }
  return 0;
}
//----------------------------------------------------------------
#include "restorePack.h"

#endif
