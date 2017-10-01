#include "SmoothValue.h"
#include "basicmaths.h"

template<>
SmoothFloat::SmoothValue()
  : lambda(0.9), value(0.0){}

template<>
SmoothFloat::SmoothValue(float l)
  : lambda(l), value(0.0){}

template<>
SmoothFloat::SmoothValue(float l, float initialValue)
  : lambda(l), value(initialValue){}

template<>
SmoothInt::SmoothValue(int divider) : lambda(divider) {
// lambda = 1 - 1/divider
// divider 4:0.75, 5:0.8, 6:0.833, 7:0.857, 8:0.875, 9:0.888, 10:0.9 et c
}

template<>
SmoothInt::SmoothValue(int divider, int initialValue) 
  : lambda(divider), value(initialValue) {}

template<>
void SmoothFloat::update(float newValue){
  value = value*lambda + newValue*(1.0f - lambda);
}

template<>
void SmoothInt::update(int newValue){
  value = (value*lambda + newValue)/(lambda+1);
}

template<>
void SmoothStiffFloat::update(float newValue){
  if(abs(value-newValue) >= delta)
    value = value*lambda + newValue*(1.0f - lambda);
}

template<>
void SmoothStiffInt::update(int newValue){
  if(abs(value-newValue) >= delta)
    value = (value*lambda + newValue)/(lambda+1);
}

template<>
float SmoothValue<float>::normal(float lambda, int blocksize){
  return lambda*128.0f/blocksize;
}

template<>
int SmoothValue<int>::normal(float lambda, int blocksize){
  return (1.0f/(1.0-lambda))*128.0f/blocksize;
}

template<>
float StiffValue<float>::normal(float delta){
  return delta;
}

template<>
int StiffValue<int>::normal(float delta){
  return delta;
}

template class SmoothValue<int>;
template class SmoothValue<float>;
template class SmoothStiffValue<int>;
template class SmoothStiffValue<float>;
