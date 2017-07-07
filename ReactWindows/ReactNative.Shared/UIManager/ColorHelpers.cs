#if WINDOWS_UWP
using Windows.UI;
#else
using System.Windows.Media;
#endif

namespace ReactNative.UIManager
{
    /// <summary>
    /// Helper class for parsing color values.
    /// </summary>
    public static class ColorHelpers
    {
        /// <summary>
        /// Unsigned integer representation of transparent color.
        /// </summary>
        public const uint Transparent = 0x00FFFFFF;

        /// <summary>
        /// Determines if the color's alpha value makes it completely transparent.
        /// <see href="https://msdn.microsoft.com/en-us/library/system.windows.media.color.fromargb(v=vs.110).aspx#Anchor_1"/>
        /// </summary>
        public static bool IsTransparent(uint color)
        {
            return (color & ~Transparent) == 0;
        }

        /// <summary>
        /// Parses a color from an unsigned integer.
        /// </summary>
        /// <param name="value">The unsigned integer color value.</param>
        /// <returns>The parsed color value.</returns>
        public static Color Parse(uint value)
        {
            var color = value;
            var b = (byte)color;
            color >>= 8;
            var g = (byte)color;
            color >>= 8;
            var r = (byte)color;
            color >>= 8;
            var a = (byte)color;
            return Color.FromArgb(a, r, g, b);
        }
    }
}
