// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <memory>
#include <string>

#include <cxxreact/JSExecutor.h>
#include <cxxreact/MessageQueueThread.h>
#include <cxxreact/ReactMarker.h>

#include "Logging.h"

#if defined(USE_CHAKRA)
#if defined(USE_EDGEMODE_JSRT)
#include <jsrt.h>
#else
#include <ChakraCore.h>
#endif

#define CHAKRA_FAILED(jsErrorCode) ((jsErrorCode) != ::JsErrorCode::JsNoError)

#ifdef NDEBUG
#define CHAKRA_ASSERTDO(jsErrorCodeExpr) (jsErrorCodeExpr)
#else
#define CHAKRA_ASSERTDO(jsErrorCodeExpr) \
  do {                                   \
    JsErrorCode ec = (jsErrorCodeExpr);  \
    assert(!CHAKRA_FAILED(ec));          \
  } while (false)
#endif
#endif // defined(USE_CHAKRA)

namespace facebook {
namespace react {

namespace JSNativeHooks {

using LoggingHook = void (*)(RCTLogLevel level, const char *);
extern LoggingHook loggingHook;

using NowHook = double (*)();
extern NowHook nowHook;

#if defined(USE_CHAKRA)
JsValueRef __stdcall nowHookJNF(
    JsValueRef function,
    bool isConstructCall,
    JsValueRef arguments[],
    unsigned short argumentCount,
    void *callbackState);
#endif

} // namespace JSNativeHooks

} // namespace react
} // namespace facebook
