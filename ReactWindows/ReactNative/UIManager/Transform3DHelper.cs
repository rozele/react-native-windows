using Newtonsoft.Json.Linq;
using System;
using System.Linq;
using Windows.UI.Xaml.Media.Media3D;

namespace ReactNative.UIManager
{
    static class Transform3DHelper
    {
        public static CompositeTransform3D ProcessTransform(JArray transforms)
        {
            var result = new CompositeTransform3D();
            foreach (var transform in transforms)
            {
                var transformMap = (JObject)transform;
                var transformType = transformMap.Properties().SingleOrDefault().Name;
                switch (transformType)
                {
                    case "rotateX":
                        result.RotationX = ConvertToDegrees(transformMap, transformType);
                        break;
                    case "rotateY":
                        result.RotationY = ConvertToDegrees(transformMap, transformType);
                        break;
                    case "rotate":
                    case "rotateZ":
                        result.RotationZ = ConvertToDegrees(transformMap, transformType);
                        break;
                    case "scale":
                        var scale = transformMap.Value<double>(transformType);
                        result.ScaleX = scale;
                        result.ScaleY = scale;
                        break;
                    case "scaleX":
                        result.ScaleX = transformMap.Value<double>(transformType);
                        break;
                    case "scaleY":
                        result.ScaleY = transformMap.Value<double>(transformType);
                        break;
                    case "translate":
                        var value = (JArray)transformMap.GetValue(transformType);
                        result.TranslateX = value.Value<double>(0);
                        result.TranslateY = value.Value<double>(1);
                        result.TranslateZ = value.Count > 2 ? value.Value<double>(2) : 0.0;
                        break;
                    case "translateX":
                        result.TranslateX = transformMap.Value<double>(transformType);
                        break;
                    case "translateY":
                        result.TranslateY = transformMap.Value<double>(transformType);
                        break;
                    case "skewX":
                    case "skewY":
                    case "matrix":
                    case "perspective":
                        break;
                    default:
                        throw new InvalidOperationException(
                            $"Unsupported transform type: '{transformType}'");
                }
            }

            return result;
        }

        private static double ConvertToDegrees(JObject transformMap, string key)
        {
            var value = default(double);
            var inDegrees = false;
            var mapValue = transformMap.GetValue(key);
            if (mapValue.Type == JTokenType.String)
            {
                var stringValue = mapValue.Value<string>();
                if (stringValue.EndsWith("rad"))
                {
                    stringValue = stringValue.Substring(0, stringValue.Length - 3);
                }
                else if (stringValue.EndsWith("deg"))
                {
                    inDegrees = true;
                    stringValue = stringValue.Substring(0, stringValue.Length - 3);
                }

                value = double.Parse(stringValue);
            }
            else
            {
                value = mapValue.Value<double>();
            }

            value *= -1.0;
            return inDegrees ? value : MatrixMathHelper.RadiansToDegrees(value);
        }
    }
}
