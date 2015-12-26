#ifdef TRACER_EXPORTS
#define TRACER_API __declspec(dllexport)
#define TRACER_DATA extern __declspec(dllexport)
#else
#define TRACER_API __declspec(dllimport)
#define TRACER_DATA extern __declspec(dllimport)
#endif
