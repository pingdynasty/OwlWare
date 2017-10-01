#include "arm_math.h"
#include "arm_const_structs.h"
#include "ServiceCall.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"

#include "FastLogTable.h"
#include "FastPowTable.h"

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
  int ret = OWL_SERVICE_INVALID_ARGS;
  switch(service){
  case OWL_SERVICE_VERSION:
    if(len > 0){
      int* value = (int*)params[0];
      *value = OWL_SERVICE_VERSION_V1;
      ret = OWL_SERVICE_OK;
    }
    break;
  case OWL_SERVICE_ARM_RFFT_FAST_INIT_F32:
    if(len == 2){
      arm_rfft_fast_instance_f32* instance = (arm_rfft_fast_instance_f32*)params[0];
      int fftlen = *(int*)params[1];
      arm_rfft_fast_init_f32(instance, fftlen);
      ret = OWL_SERVICE_OK;
    }
    break;
  case OWL_SERVICE_ARM_CFFT_INIT_F32:
    if(len == 2){
      arm_cfft_instance_f32* instance = (arm_cfft_instance_f32*)params[0];
      int fftlen = *(int*)params[1];
      ret = SERVICE_ARM_CFFT_INIT_F32(instance, fftlen);
    }
    break;
  case OWL_SERVICE_GET_PARAMETERS: {
    int index = 0;
    ret = OWL_SERVICE_OK;
    while(len >= index+2){
      char* p = (char*)params[index++];
      int32_t* value = (int32_t*)params[index++];
      if(strncmp(SYSEX_CONFIGURATION_INPUT_OFFSET, p, 2) == 0){
	*value = settings.input_offset;
      }else if(strncmp(SYSEX_CONFIGURATION_INPUT_SCALAR, p, 2) == 0){
	*value = settings.input_scalar;
      }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_OFFSET, p, 2) == 0){
	*value = settings.output_offset;
      }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_SCALAR, p, 2) == 0){
	*value = settings.output_scalar;
      }else{
	ret = OWL_SERVICE_INVALID_ARGS;
      }
    }
    break;
  }
  case OWL_SERVICE_GET_ARRAY: {
    // get array and array size
    // expects three parameters: name, &array and &size
    int index = 0;
    ret = OWL_SERVICE_OK;
    if(len >= index+3){
      char* p = (char*)params[index++];
      void** array = (void**)params[index++];
      int* size = (int*)params[index++];
      if(strncmp(SYSTEM_TABLE_LOG, p, 3) == 0){
	*array = (void*)fast_log_table;
	*size = fast_log_table_size;
      }else if(strncmp(SYSTEM_TABLE_POW, p, 3) == 0){
	*array = (void*)fast_pow_table;
	*size = fast_pow_table_size;
      }else{
	*array = NULL;
	*size = 0;
	ret = OWL_SERVICE_INVALID_ARGS;
      }
    }
    // while(len >= index+2){
    //   char* p = (char*)params[index++];
    //   void** value = (void**)params[index++];
    //   if(strncmp(SYSTEM_TABLE_ICSI_LOG, p, 3) == 0){
    // 	*value = (void*)fast_log_table;
    //   }else if(strncmp(SYSTEM_TABLE_ICSI_E_H, p, 3) == 0){
    // 	*value = (void*)fast_pow_h_table;
    //   }else if(strncmp(SYSTEM_TABLE_ICSI_E_L, p, 3) == 0){
    // 	*value = (void*)fast_pow_l_table;
    //   }else{
    // 	ret = OWL_SERVICE_INVALID_ARGS;
    //   }
    // }
    break;
  }
  }
  return ret;
}     
