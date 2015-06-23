
#define OWL_SERVICE_VERSION                0x001
#define OWL_SERVICE_ARM_RFFT_FAST_INIT_F32 0x100
#define OWL_SERVICE_INVALID_ARGS -1

#define OWL_SERVICE_VERSION_V1             0x001

#ifdef __cplusplus
 extern "C" {
#endif

   int serviceCall(int service, void** params, int len);

#ifdef __cplusplus
}
#endif
