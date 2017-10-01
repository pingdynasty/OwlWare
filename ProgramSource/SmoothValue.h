#ifndef __SmoothValue_h__
#define __SmoothValue_h__

/**
 * Applies exponential smoothing to a scalar value.
 * y(n) = lambda*y(n-1) + (1.0-lambda)*y(n) for floats
 * y(n) = (lambda*y(n-1) + y(n))/(lambda+1) for ints
 * Smoothing is applied when the value is written to, not when it is read.
 * todo: adjust for sampling frequency / blocksize (time delay of the response (mean) is 1/α data points)
 * https://en.wikipedia.org/wiki/Exponential_smoothing
 */
template<typename T>
class SmoothValue {
private:
  T value;
public:
  T lambda;
  SmoothValue();
  SmoothValue(T lambda);
  SmoothValue(T lambda, T initialValue);
  SmoothValue(const SmoothValue<T>& other){
    // copy constructor: not needed?
    value = other.value;
    lambda = other.lambda;
  }
  void update(T newValue);
  T getValue(){
    return value;
  }
  SmoothValue<T>& operator=(const T& other){
    update(other);
    return *this;
  }
  SmoothValue<T>& operator+=(const T& other){
    update(value+other);
    return *this;
  }
  SmoothValue<T>& operator-=(const T& other){
    update(value-other);
    return *this;
  }
  SmoothValue<T>& operator*=(const T& other){
    update(value*other);
    return *this;
  }
  SmoothValue<T>& operator/=(const T& other){
    update(value/other);
    return *this;
  }
  operator T(){
    return getValue();
  }
  static T normal(float lambda, int blocksize);
};

typedef SmoothValue<float> SmoothFloat;
typedef SmoothValue<int> SmoothInt;

/**
 * Applies simple hysteresis to a scalar.
 * Only updates the value if the absolute difference is greater than delta
 */
template<typename T>
class StiffValue {  
private:
  T value;
public:
  T delta;
  StiffValue()
    : value(0.0), delta(0.02){}
  StiffValue(T d)
    : value(0.0), delta(d){}
  StiffValue(T d, T initialValue)
    : value(initialValue), delta(d){}
  void update(T newValue){
    if(abs(value-newValue) > delta)
      value = newValue;
  }
  T getValue(){
    return value;
  }
  StiffValue<T>& operator=(const T& other){
    update(other);
    return *this;
  }
  StiffValue<T>& operator+=(const T& other){
    update(value+other);
    return *this;
  }
  StiffValue<T>& operator-=(const T& other){
    update(value-other);
    return *this;
  }
  StiffValue<T>& operator*=(const T& other){
    update(value*other);
    return *this;
  }
  StiffValue<T>& operator/=(const T& other){
    update(value/other);
    return *this;
  }
  operator T(){
    return getValue();
  }
  static T normal(float delta);
};

typedef StiffValue<float> StiffFloat;
typedef StiffValue<int> StiffInt;

/**
 * Applies hysteresis and smoothing to a scalar.
 */
template<typename T>
class SmoothStiffValue {
  // simple hysteresis
private:
  T lambda;
  T delta;
  T value;
public:
  SmoothStiffValue(){}
  SmoothStiffValue(T l, T d)
    : lambda(d), delta(d){}
  SmoothStiffValue(T l, T d, T initialValue)
    : lambda(d), delta(d), value(initialValue) {}
  void update(T newValue);
  T getValue(){
    return value;
  }
  SmoothStiffValue<T>& operator=(const T& other){
    update(other);
    return *this;
  }
  SmoothStiffValue<T>& operator+=(const T& other){
    update(value+other);
    return *this;
  }
  SmoothStiffValue<T>& operator-=(const T& other){
    update(value-other);
    return *this;
  }
  SmoothStiffValue<T>& operator*=(const T& other){
    update(value*other);
    return *this;
  }
  SmoothStiffValue<T>& operator/=(const T& other){
    update(value/other);
    return *this;
  }
  operator T(){
    return getValue();
  }
};

typedef SmoothStiffValue<float> SmoothStiffFloat;
typedef SmoothStiffValue<int> SmoothStiffInt;

#endif /* __SmoothValue_h__ */
