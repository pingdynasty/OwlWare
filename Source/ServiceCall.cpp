#include "arm_math.h"
#include "arm_const_structs.h"
#include "ServiceCall.h"
#include "ProgramVector.h"

int SERVICE_ARM_CFFT_INIT_F32(arm_cfft_instance_f32* instance, int len){
  switch(len) { 
  case 16:
    *instance = arm_cfft_sR_f32_len16;
    break;
  case 32:
    *instance = arm_cfft_sR_f32_len32;
    break;
  case 64:
    *instance = arm_cfft_sR_f32_len64;
    break;
  case 128:
    *instance = arm_cfft_sR_f32_len128;
    break;
  case 256:
    *instance = arm_cfft_sR_f32_len256;
    break;
  case 512:
    *instance = arm_cfft_sR_f32_len512;
    break;
  case 1024:
    *instance = arm_cfft_sR_f32_len1024;
    break;      
  case 2048:
    *instance = arm_cfft_sR_f32_len2048;
    break;
  case 4096:
    *instance = arm_cfft_sR_f32_len4096;
    break;
  default:
    return OWL_SERVICE_INVALID_ARGS;
  }
  return OWL_SERVICE_OK;
}

int serviceCall(int service, void** params, int len){
  switch(service){
  case OWL_SERVICE_VERSION:
    if(len > 0){
      int* value = (int*)params[0];
      *value = OWL_SERVICE_VERSION_V1;
      return OWL_SERVICE_OK;
    }
    break;
  case OWL_SERVICE_ARM_RFFT_FAST_INIT_F32:
    if(len == 2){
      arm_rfft_fast_instance_f32* instance = (arm_rfft_fast_instance_f32*)params[0];
      int len = *(int*)params[1];
      arm_rfft_fast_init_f32(instance, len);
      return OWL_SERVICE_OK;
    }
    break;
  case OWL_SERVICE_ARM_CFFT_INIT_F32:
    if(len == 2){
      arm_cfft_instance_f32* instance = (arm_cfft_instance_f32*)params[0];
      int len = *(int*)params[1];
      return SERVICE_ARM_CFFT_INIT_F32(instance, len);
    }
    break;
  }
  return OWL_SERVICE_INVALID_ARGS;
}     
