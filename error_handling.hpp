

#ifndef ERROR_HANDLING
#define ERROR_HANDLING 

#include <string>
#include <functional>

#define FATAL_ERROR(MSG) handleFatalError(MSG, __FILE__, __LINE__)

void
handleFatalError(char const* errorMsg);

void
handleFatalError(char const* errorMsg, char const* file, int const line);

#endif

