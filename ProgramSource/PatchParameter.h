#ifndef __PatchParameter_h__
#define __PatchParameter_h__

template<typename T>
class PatchParameter {
private:
  int pid;
  T value;
public:
  PatchParameter();
  PatchParameter(int parameterId) : pid(parameterId){}
  /* assignment operator */
  PatchParameter<T>& operator=( const PatchParameter<T>& other );
  void update(T newValue){
    value = newValue;
  }
  T getValue(){
    return value;
  }
  operator T(){
    return getValue();
  }
};

typedef PatchParameter<float> FloatParameter;
typedef PatchParameter<int> IntParameter;

#endif /* __PatchParameter_h__ */
