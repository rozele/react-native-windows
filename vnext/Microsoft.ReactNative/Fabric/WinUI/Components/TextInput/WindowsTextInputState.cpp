/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WindowsTextInputState.h"

#include <react/renderer/components/text/conversions.h>
#include <react/renderer/debug/debugStringConvertibleUtils.h>

#include <utility>

namespace facebook {
namespace react {

WindowsTextInputState::WindowsTextInputState(
    int32_t mostRecentEventCount,
    AttributedString attributedString,
    AttributedString reactTreeAttributedString,
    ParagraphAttributes paragraphAttributes,
    TextAttributes defaultTextAttributes,
    ShadowView defaultParentShadowView,
    float defaultThemePaddingStart,
    float defaultThemePaddingEnd,
    float defaultThemePaddingTop,
    float defaultThemePaddingBottom)
    : mostRecentEventCount(mostRecentEventCount),
      attributedString(std::move(attributedString)),
      reactTreeAttributedString(std::move(reactTreeAttributedString)),
      paragraphAttributes(std::move(paragraphAttributes)),
      defaultTextAttributes(std::move(defaultTextAttributes)),
      defaultParentShadowView(std::move(defaultParentShadowView)),
      defaultThemePaddingStart(defaultThemePaddingStart),
      defaultThemePaddingEnd(defaultThemePaddingEnd),
      defaultThemePaddingTop(defaultThemePaddingTop),
      defaultThemePaddingBottom(defaultThemePaddingBottom) {}

WindowsTextInputState::WindowsTextInputState(WindowsTextInputState const &previousState, folly::dynamic const &data)
    : mostRecentEventCount(
          static_cast<int32_t>(data.getDefault("mostRecentEventCount", previousState.mostRecentEventCount).getInt())),
      attributedString(previousState.attributedString),
      reactTreeAttributedString(previousState.reactTreeAttributedString),
      paragraphAttributes(previousState.paragraphAttributes),
      defaultTextAttributes(previousState.defaultTextAttributes),
      defaultParentShadowView(previousState.defaultParentShadowView),
      defaultThemePaddingStart(
          static_cast<float>(data.getDefault("themePaddingStart", previousState.defaultThemePaddingStart).getDouble())),
      defaultThemePaddingEnd(
          static_cast<float>(data.getDefault("themePaddingEnd", previousState.defaultThemePaddingEnd).getDouble())),
      defaultThemePaddingTop(
          static_cast<float>(data.getDefault("themePaddingTop", previousState.defaultThemePaddingTop).getDouble())),
      defaultThemePaddingBottom(static_cast<float>(
          data.getDefault("themePaddingBottom", previousState.defaultThemePaddingBottom).getDouble())){};

} // namespace react
} // namespace facebook
