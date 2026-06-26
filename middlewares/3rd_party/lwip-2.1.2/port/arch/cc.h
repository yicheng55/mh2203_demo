#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Bare-metal STM32 ports normally do not provide POSIX unistd.h. */
//#define LWIP_NO_UNISTD_H 1

/* Keil/ARM toolchains are often missing full inttypes.h support. */
#define LWIP_NO_INTTYPES_H 1

/* Let lwIP provide errno values on embedded targets. */
#define LWIP_PROVIDE_ERRNO 1

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x) do { } while(0)
#endif

#ifndef LWIP_PLATFORM_ASSERT
extern void lwip_platform_assert(const char *msg);
#define LWIP_PLATFORM_ASSERT(x) lwip_platform_assert(x)
//#define LWIP_PLATFORM_ASSERT(x) do { (void)(x); for (;;) { } } while(0)
#endif

#ifndef LWIP_RAND
#define LWIP_RAND() ((u32_t)0x12345678UL)
#endif

#ifndef PACK_STRUCT_BEGIN
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define PACK_STRUCT_BEGIN __packed
#else
#define PACK_STRUCT_BEGIN
#endif
#endif

#ifndef PACK_STRUCT_END
#define PACK_STRUCT_END
#endif

#ifndef PACK_STRUCT_FIELD
#define PACK_STRUCT_FIELD(x) x
#endif

#ifndef PACK_STRUCT_FLD_8
#define PACK_STRUCT_FLD_8(x) PACK_STRUCT_FIELD(x)
#endif

#ifndef PACK_STRUCT_FLD_S
#define PACK_STRUCT_FLD_S(x) PACK_STRUCT_FIELD(x)
#endif

#ifndef PACK_STRUCT_STRUCT
#if defined(__GNUC__) || defined(__clang__)
#define PACK_STRUCT_STRUCT __attribute__((packed))
#else
#define PACK_STRUCT_STRUCT
#endif
#endif

#ifndef X8_F
#define X8_F "02x"
#endif

#ifndef U16_F
#define U16_F "u"
#endif

#ifndef S16_F
#define S16_F "d"
#endif

#ifndef X16_F
#define X16_F "x"
#endif

#ifndef U32_F
#define U32_F "lu"
#endif

#ifndef S32_F
#define S32_F "ld"
#endif

#ifndef X32_F
#define X32_F "lx"
#endif

#ifndef SZT_F
#define SZT_F "u"
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWIP_ARCH_CC_H */
