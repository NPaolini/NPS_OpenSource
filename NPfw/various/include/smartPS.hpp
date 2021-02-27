//--------- smartPS.hpp ------------------------------------------------------
//----------------------------------------------------------------------------
template <typename T, bool isArray>
void P_SmartPointer<T, isArray>::move(const P_SmartPointer<T, isArray>& other)
{
  if(P != other.P) {
    if(autoDelete) {
      if(isArray)
        delete []P;
      else
        delete P;
      }
    P = other.P;
    autoDelete = other.autoDelete;
    other.P = 0;
    other.autoDelete = false;
    }
}
//----------------------------------------------------------------------------
