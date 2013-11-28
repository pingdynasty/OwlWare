#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__
// #include <map>
#include <string>

#include "StompBox.h"
#include "device.h"
#include "basicmaths.h"

class PatchRegistry;
extern PatchRegistry patches;

class PatchRegistry {
  typedef Patch* (*PatchCreator)(); // function pointer to create Patch
public:
  PatchRegistry();
/*   StringArray getNames(); */
  std::string getName(unsigned int index); 
  Patch* create(unsigned int index);
  Patch* create(const std::string& name);
  unsigned int getNumberOfPatches();
  void registerPatch(const std::string& name, PatchCreator creator);
private:
  template<class T> struct Register {
    static Patch* construct() {
      return new T();
    };
    static PatchCreator* creator;
  };
private:
  std::string names[MAX_NUMBER_OF_PATCHES];
  PatchCreator creators[MAX_NUMBER_OF_PATCHES];
  int nofPatches;
  // typedef std::map<std::string, PatchCreator*> Creators; // map from id to creator
  // static Creators& getCreators() { 
  //   static Creators creators; 
  //   return creators; 
  // }
};

#endif // __PatchRegistry_h__
