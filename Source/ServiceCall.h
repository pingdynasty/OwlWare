#define OWL_SERVICE_VERSION_V1             0x0001
#define OWL_SERVICE_ARM_RFFT_FAST_INIT_F32 0x0100
#define OWL_SERVICE_ARM_CFFT_INIT_F32      0x0110
#define OWL_SERVICE_GET_PARAMETERS         0x1000
#define OWL_SERVICE_OK                     0x000
#define OWL_SERVICE_INVALID_ARGS           -1

#define OWL_SERVICE_VERSION                OWL_SERVICE_VERSION_V1

#ifdef __cplusplus
 extern "C" {
#endif

   int serviceCall(int service, void** params, int len);

#ifdef __cplusplus
}
#endif
