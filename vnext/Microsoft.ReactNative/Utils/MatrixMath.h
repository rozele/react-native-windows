// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "JSValue.h"

namespace Microsoft::ReactNative {

struct MatrixMath {
  static winrt::Windows::Foundation::Numerics::float4x4 GetTransformMatrix(
      winrt::Microsoft::ReactNative::JSValueArray const &transforms);
};
 
} // namespace Microsoft::ReactNative
