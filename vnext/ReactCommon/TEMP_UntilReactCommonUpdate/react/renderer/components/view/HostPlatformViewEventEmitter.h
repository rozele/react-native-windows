/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifdef USE_WINUI_FABRIC

#include <react/renderer/components/view/windows/WindowsViewEventEmitter.h>

namespace facebook::react {
using HostPlatformViewEventEmitter = WindowsViewEventEmitter;
} // namespace facebook::react

#else

#include <react/renderer/components/view/TouchEventEmitter.h>

namespace facebook::react {
using HostPlatformViewEventEmitter = TouchEventEmitter;
} // namespace facebook::react

#endif
