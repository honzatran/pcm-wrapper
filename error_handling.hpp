

#ifndef ERROR_HANDLING
#define ERROR_HANDLING 

#include <string>
#include <functional>

#define STR_CONCAT(A,B) #A " :" #B

#define FATAL_ERROR(MSG) handleFatalError(MSG, __FILE__, __LINE__)

#define ASSERT(cond, msg) if (!(cond)) { FATAL_ERROR(STR_CONCAT(msg, #cond)); }

#define ASSERT_COND(cond) if (!(cond)) { FATAL_ERROR(#cond); }


void
resetDefaultErrorHandler();

void
setDefaultErrorHandler(std::function<void(char const*)> const& handler);

void
handleFatalError(char const* errorMsg);

void
handleFatalError(char const* errorMsg, char const* file, int const line);

#endif

