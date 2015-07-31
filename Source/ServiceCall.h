#define OWL_SERVICE_VERSION_V1             0x001
#define OWL_SERVICE_ARM_RFFT_FAST_INIT_F32 0x100
#define OWL_SERVICE_ARM_CFFT_INIT_F32      0x110
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
