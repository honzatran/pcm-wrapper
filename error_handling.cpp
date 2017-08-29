

#include "error_handling.hpp"

#include <iostream>
#include <vector>
#include <pcm/cpucounters.h>

using namespace std;

void defaultFatalHandler(char const* errorMsg) {
    cerr << errorMsg << endl;
    std::terminate();
}


std::function<void(char const*)> g_fatalErrorHandler = defaultFatalHandler;

std::vector<std::function<void()>> g_onFatalErrors;

void
detailedDefaultFatalHandler(char const* errorMsg,
                            char const* file,
                            int const line) {
    cerr << "fatal error at " << file << ", " << line << ":";
    g_fatalErrorHandler(errorMsg);
}

std::function<void(char const*, char const*, int const)>
    g_detailedFatalErrorHandler = detailedDefaultFatalHandler;

void
runOnFatalError() {
    auto *const pcm = PCM::getInstance();
    if (pcm != nullptr) {
        pcm->cleanup();
    }
}

void handleFatalError(char const* errorMsg) {
    runOnFatalError();
    g_fatalErrorHandler(errorMsg);
}

void
handleFatalError(char const* errorMsg, char const* file, int const line) {
    runOnFatalError();
    g_detailedFatalErrorHandler(errorMsg, file, line);
}

