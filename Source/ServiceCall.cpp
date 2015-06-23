#include "arm_math.h"
#include "ServiceCall.h"
#include "ProgramVector.h"

int serviceCall(int service, void** params, int len){
  switch(service){
  case OWL_SERVICE_VERSION:
    if(len > 0){
      int* value = (int*)params[0];
      *value = OWL_SERVICE_VERSION_V1;
      return 0;
    }
    break;
  case OWL_SERVICE_ARM_RFFT_FAST_INIT_F32:
    if(len == 2){
      arm_rfft_fast_instance_f32* instance = (arm_rfft_fast_instance_f32*)params[0];
      int len = *(int*)params[1];
      arm_rfft_fast_init_f32(instance, len);
      return 0;
    }
    break;
  }
  return OWL_SERVICE_INVALID_ARGS;
}     
