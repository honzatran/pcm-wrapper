

#include "error_handling.hpp"

#include <iostream>

using namespace std;

void defaultFatalHandler(char const* errorMsg) {
    cerr << errorMsg << endl;
    std::terminate();
}


std::function<void(char const*)> g_fatalErrorHandler = defaultFatalHandler;

void
detailedDefaultFatalHandler(char const* errorMsg,
                            char const* file,
                            int const line) {
    cerr << "fatal error at " << file << ", " << line << ":";
    g_fatalErrorHandler(errorMsg);
}

std::function<void(char const*, char const*, int const)>
    g_detailedFatalErrorHandler = detailedDefaultFatalHandler;

void handleFatalError(char const* errorMsg) {
    g_fatalErrorHandler(errorMsg);
}

void
handleFatalError(char const* errorMsg, char const* file, int const line) {
    g_detailedFatalErrorHandler(errorMsg, file, line);
}



