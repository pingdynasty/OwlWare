#ifndef __BitState_h__
#define __BitState_h__

class BitState32 {
private:
  uint32_t state;
public:
  BitState32() : state(0) {}
  inline bool get(uint8_t ch){
    return state & (1<<ch);
  }
  inline void clear(uint8_t ch){
    state &= ~(1<<ch); // clear engaged flag
  }
  inline void set(uint8_t ch){
    state |= (1<<ch); // set engaged flag
  }
  inline void clear(){
    state = 0x0;
  }
  inline void set(){
    state = 0xffff;
  }
  inline int getFirstSetIndex(){
    return 31-countLeadingZeros();
  }
  inline int countLeadingZeros(){
    return __builtin_clz(state);
  }
  inline uint16_t getState(){
    return state;
  }
};

/* 128 bits boolean state management */
class BitState128 {
private:
  uint32_t state[4];
public:  
  BitState128() {
    clear();
  }
  void clear(){
    memset(state, 0, sizeof(state));
  }
  inline bool isClear(){
    return !(state[0] || state[1] || state[2] || state[3]);
  }
  inline bool isEngaged(uint8_t key){
    if(key < 64){
      if(key < 32)
	return (state[3] & (1<<key));
      else
	return (state[2] & (1<<(key-32)));
    }else if(key < 96){
      return (state[1] & (1<<(key-64)));
    }else
      return (state[0] & (1<<(key-96)));
  }
  inline int getFirstEngagedKeyIndex(){
    return 127-countLeadingZeros();
  }
  inline int countLeadingZeros(){
    if(state[0])
      return __builtin_clz(state[0]);
    if(state[1])
      return __builtin_clz(state[1])+32;
    if(state[2])
      return __builtin_clz(state[2])+64;
    return  __builtin_clz(state[3])+96;
  }
  inline void setEngaged(uint8_t key){
    if(key < 64){
      if(key < 32)
	(state[3] |= (1<<key));
      else
	(state[2] |= (1<<(key-32)));
    }else if(key < 96){
      (state[1] |= (1<<(key-64)));
    }else
      (state[0] |= (1<<(key-96)));
    // key < 64 ? state[0] |= (1<<key) 
    //   : state[1] |= (1<<(key-64));
  }
  inline void clearEngaged(uint8_t key){
    if(key < 64){
      if(key < 32)
	(state[3] &= ~(1<<key));
      else 
	(state[2] &= ~(1<<(key-32)));
    }else if(key < 96){
      (state[1] &= ~(1<<(key-64)));
    }else
      (state[0] &= ~(1<<(key-96)));
    // key < 64 ? state[0] &= ~(1<<key) 
    //   : state[1] &= ~(1<<(key-64));
  }
};

#endif /* defined(__BitState_h__) */
