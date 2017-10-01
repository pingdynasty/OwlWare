#include "PatchParameter.h"
#include "PatchProcessor.h"
#include "SmoothValue.h"
#include "message.h"

PatchProcessor* getInitialisingPatchProcessor();

#define PATCH_PARAMETER_NO_PID -1

template<typename T>
PatchParameter<T>::PatchParameter() : pid(PATCH_PARAMETER_NO_PID){}

// copy ctors: superceded by assignment operators
/* PatchParameter(PatchParameter<T>& other); */
/* PatchParameter(const PatchParameter<T>& other); */

// template<typename T>
// PatchParameter<T>::PatchParameter(PatchParameter<T>& other) :
//   pid(other.pid), value(other.value) {
//   // copy ctor
//   // register for update callback in copy constructor
//   if(pid != PATCH_PARAMETER_NO_PID)
//     getInitialisingPatchProcessor()->setPatchParameter(pid, this);
// }

// template<typename T>
// PatchParameter<T>::PatchParameter(const PatchParameter<T>& other) :
//   pid(other.pid), value(other.value) {
//   // copy ctor
//   // register for update callback in copy constructor
//   if(pid != PATCH_PARAMETER_NO_PID)
//     getInitialisingPatchProcessor()->setPatchParameter(pid, this);
// }

template<typename T>
PatchParameter<T>& PatchParameter<T>::operator=(const PatchParameter<T>& other){
  pid = other.pid;
  value = other.value;
  if(pid != PATCH_PARAMETER_NO_PID)
    getInitialisingPatchProcessor()->setPatchParameter(pid, this);
  return *this;
}

template class PatchParameter<int>;
template class PatchParameter<float>;
