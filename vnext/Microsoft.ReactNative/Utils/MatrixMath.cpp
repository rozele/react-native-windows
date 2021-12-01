// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Utils/MatrixMath.h"

#include <winrt/Windows.Foundation.h>

namespace Microsoft::ReactNative {

winrt::Windows::Foundation::Numerics::float4x4 Identity() {
  return winrt::Windows::Foundation::Numerics::float4x4::identity();
}

winrt::Windows::Foundation::Numerics::float4x4 GetPerspective(float perspective) {
  auto result = Identity();
  result.m34 = -1 / perspective;
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetRotateX(float radians) {
  auto result = Identity();
  result.m22 = std::cos(radians);
  result.m23 = std::sin(radians);
  result.m32 = -std::sin(radians);
  result.m33 = std::cos(radians);
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetRotateY(float radians) {
  auto result = Identity();
  result.m11 = std::cos(radians);
  result.m13 = -std::sin(radians);
  result.m31 = std::sin(radians);
  result.m33 = std::cos(radians);
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetRotateZ(float radians) {
  auto result = Identity();
  result.m11 = std::cos(radians);
  result.m12 = std::sin(radians);
  result.m21 = -std::sin(radians);
  result.m22 = std::cos(radians);
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetScaleX(float factor) {
  auto result = Identity();
  result.m11 = factor;
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetScaleY(float factor) {
  auto result = Identity();
  result.m22 = factor;
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetScaleXY(float factor) {
  auto result = Identity();
  result.m11 = factor;
  result.m22 = factor;
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetScaleZ(float factor) {
  auto result = Identity();
  result.m33 = factor;
  return result;
}

winrt::Windows::Foundation::Numerics::float4x4 GetSkewX(float radians) {
  auto result = Identity();
  result.m21 = std::sin(radians);
  result.m22 = std::cos(radians);
  return result;
}

static winrt::Windows::Foundation::Numerics::float4x4 GetSkewY(float radians) {
  auto result = Identity();
  result.m21 = std::cos(radians);
  result.m22 = std::sin(radians);
  return result;
}

static winrt::Windows::Foundation::Numerics::float4x4 GetTranslate(float x, float y, float z) {
  auto result = Identity();
  result.m41 = x;
  result.m42 = y;
  result.m43 = z;
  return result;
}

static float ConvertToRadians(winrt::Microsoft::ReactNative::JSValue const &value) {
  if (value.Type() == winrt::Microsoft::ReactNative::JSValueType::String) {
    const auto stringValue = value.AsString();
    const auto length = stringValue.length() - 3;
    if (stringValue.compare(length, 3, "rad")) {
      return static_cast<float>(std::atof((stringValue.substr(0, length)).c_str()));
    } else if (stringValue.compare(stringValue.length() - 3, 3, "deg")) {
      const auto degrees = std::atof((stringValue.substr(0, length)).c_str());
      return static_cast<float>(degrees * std::atan(1) * 4 / 180);
    }

    return static_cast<float>(std::atof(stringValue.c_str()));
  }

  return static_cast<float>(value.AsDouble());
}

static void MultiplyInto(
  winrt::Windows::Foundation::Numerics::float4x4 &m,
  winrt::Windows::Foundation::Numerics::float4x4 o) {
  float a00 = m.m11, a01 = m.m12, a02 = m.m13, a03 = m.m14,
    a10 = m.m21, a11 = m.m22, a12 = m.m23, a13 = m.m24,
    a20 = m.m31, a21 = m.m32, a22 = m.m33, a23 = m.m34,
    a30 = m.m41, a31 = m.m42, a32 = m.m43, a33 = m.m44;

  float b0 = o.m11, b1 = o.m12, b2 = o.m13, b3 = o.m14;
  m.m11 = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  m.m12 = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  m.m13 = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  m.m14 = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = o.m21; b1 = o.m22; b2 = o.m23; b3 = o.m24;
  m.m21 = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  m.m22 = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  m.m23 = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  m.m24 = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = o.m31; b1 = o.m32; b2 = o.m33; b3 = o.m34;
  m.m31 = b0* a00 + b1 * a10 + b2 * a20 + b3 * a30;
  m.m32 = b0* a01 + b1 * a11 + b2 * a21 + b3 * a31;
  m.m33 = b0* a02 + b1 * a12 + b2 * a22 + b3 * a32;
  m.m34 = b0* a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = o.m41; b1 = o.m42; b2 = o.m43; b3 = o.m44;
  m.m41 = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  m.m42 = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  m.m43 = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  m.m44 = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
}

/*static*/ winrt::Windows::Foundation::Numerics::float4x4 MatrixMath::GetTransformMatrix(
    winrt::Microsoft::ReactNative::JSValueArray const &transforms) {
  if (transforms.size() == 16 && transforms.at(0).Type() != winrt::Microsoft::ReactNative::JSValueType::Object) {
    winrt::Windows::Foundation::Numerics::float4x4 transformMatrix;
    transformMatrix.m11 = static_cast<float>(transforms[0].AsDouble());
    transformMatrix.m12 = static_cast<float>(transforms[1].AsDouble());
    transformMatrix.m13 = static_cast<float>(transforms[2].AsDouble());
    transformMatrix.m14 = static_cast<float>(transforms[3].AsDouble());
    transformMatrix.m21 = static_cast<float>(transforms[4].AsDouble());
    transformMatrix.m22 = static_cast<float>(transforms[5].AsDouble());
    transformMatrix.m23 = static_cast<float>(transforms[6].AsDouble());
    transformMatrix.m24 = static_cast<float>(transforms[7].AsDouble());
    transformMatrix.m31 = static_cast<float>(transforms[8].AsDouble());
    transformMatrix.m32 = static_cast<float>(transforms[9].AsDouble());
    transformMatrix.m33 = static_cast<float>(transforms[10].AsDouble());
    transformMatrix.m34 = static_cast<float>(transforms[11].AsDouble());
    transformMatrix.m41 = static_cast<float>(transforms[12].AsDouble());
    transformMatrix.m42 = static_cast<float>(transforms[13].AsDouble());
    transformMatrix.m43 = static_cast<float>(transforms[14].AsDouble());
    transformMatrix.m44 = static_cast<float>(transforms[15].AsDouble());
    return transformMatrix;
  }

  auto matrix = winrt::Windows::Foundation::Numerics::float4x4::identity();
  for (const auto &transform : transforms) {
    auto transformMatrix = Identity();
    const auto &transformMap = transform.AsObject();
    assert(transformMap.size() == 1);
    const auto entry = transformMap.begin();
    if (entry->first == "matrix") {
      transformMatrix = GetTransformMatrix(entry->second.AsArray());
    } else if (entry->first == "perspective") {
      transformMatrix = GetPerspective(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "rotateX") {
      transformMatrix = GetRotateX(ConvertToRadians(entry->second));
    } else if (entry->first == "rotateY") {
      transformMatrix = GetRotateY(ConvertToRadians(entry->second));
    } else if (entry->first == "rotate" || entry->first == "rotateZ") {
      transformMatrix = GetRotateZ(ConvertToRadians(entry->second));
    } else if (entry->first == "scale") {
      transformMatrix = GetScaleXY(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "scaleX") {
      transformMatrix = GetScaleX(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "scaleY") {
      transformMatrix = GetScaleY(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "skewX") {
      transformMatrix = GetSkewX(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "skewY") {
      transformMatrix = GetSkewY(static_cast<float>(entry->second.AsDouble()));
    } else if (entry->first == "translate") {
      auto z = 0.0f;
      const auto &translateValues = entry->second.AsArray();
      const auto x = static_cast<float>(translateValues.at(0).AsDouble());
      const auto y = static_cast<float>(translateValues.at(1).AsDouble());
      if (translateValues.size() > 2) {
        z = static_cast<float>(translateValues.at(2).AsDouble());
      }
      transformMatrix = GetTranslate(x, y, z);
    } else if (entry->first == "translateX") {
      transformMatrix = GetTranslate(static_cast<float>(entry->second.AsDouble()), 0.0f, 0.0f);
    } else if (entry->first == "translateY") {
      transformMatrix = GetTranslate(0.0f, static_cast<float>(entry->second.AsDouble()), 0.0f);
    }

    MultiplyInto(matrix, transformMatrix);
  }

  return matrix;
}

} // namespace Microsoft::ReactNative
