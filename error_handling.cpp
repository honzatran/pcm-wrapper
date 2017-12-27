

#include "error_handling.hpp"

#include <pcm/cpucounters.h>
#include <iostream>
#include <vector>

using namespace std;

void
defaultFatalHandler(char const* errorMsg)
{
    cerr << errorMsg << endl;
    std::terminate();
}

std::function<void(char const*)> g_fatalErrorHandler = defaultFatalHandler;

void
resetDefaultErrorHandler()
{
    g_fatalErrorHandler = defaultFatalHandler;
}

void
setDefaultErrorHandler(std::function<void(char const*)> const& handler)
{
    g_fatalErrorHandler = handler;
}

void
detailedDefaultFatalHandler(
    char const* errorMsg,
    char const* file,
    int const line)
{
    cerr << "fatal error at " << file << ", " << line << ":";
    g_fatalErrorHandler(errorMsg);
}

std::function<void(char const*, char const*, int const)>
    g_detailedFatalErrorHandler = detailedDefaultFatalHandler;

void
runOnFatalError()
{
    auto* const pcm = PCM::getInstance();
    if (pcm != nullptr)
    {
        pcm->cleanup();
    }
}

void
handleFatalError(char const* errorMsg)
{
    runOnFatalError();
    g_fatalErrorHandler(errorMsg);
}

void
handleFatalError(char const* errorMsg, char const* file, int const line)
{
    runOnFatalError();
    g_detailedFatalErrorHandler(errorMsg, file, line);
}
