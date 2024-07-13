#pragma once
#define SLO_PRAGMA(...) _Pragma(#__VA_ARGS__)

#if defined(__clang__)

#define SLO_WARNING_DISABLE_CLANG(WARNING) SLO_PRAGMA(clang diagnostic ignored WARNING)
#define SLO_WARNING_PUSH SLO_PRAGMA(clang diagnostic push)
#define SLO_WARNING_POP SLO_PRAGMA(clang diagnostic pop)

#else
#define SLO_WARNING_DISABLE_CLANG(...)
#endif

#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)

#define SLO_WARNING_DISABLE_GCC(WARNING) SLO_PRAGMA(GCC diagnostic ignored WARNING)
#define SLO_WARNING_PUSH SLO_PRAGMA(GCC diagnostic push)
#define SLO_WARNING_POP SLO_PRAGMA(GCC diagnostic pop)

#else
#define SLO_WARNING_DISABLE_GCC(...)
#endif

#if defined(_MSC_VER)

#define SLO_WARNING_DISABLE_MSVC(WARNING) SLO_PRAGMA(warning(disable: WARNING))
#define SLO_WARNING_PUSH SLO_PRAGMA(warning(push))
#define SLO_WARNING_POP SLO_PRAGMA(warning(pop))

#else
#define SLO_WARNING_DISABLE_MSVC(...)
#endif

#define SLO_WARNING_DISABLE(COMPILER, WARNING) SLO_WARNING_DISABLE_##COMPILER(WARNING)