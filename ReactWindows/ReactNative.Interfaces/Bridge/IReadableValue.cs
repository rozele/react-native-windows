
namespace ReactNative.Core.Bridge
{
    /// <summary>
    /// An interface for wrapping readable value types.
    /// </summary>
    public interface IReadableValue
    {
        /// <summary>
        /// Type of value.
        /// </summary>
        ReadableType Type { get; }

        /// <summary>
        /// Gets the value.
        /// </summary>
        /// <typeparam name="T">Type of value.</typeparam>
        /// <returns>The value.</returns>
        T Value<T>();
    }
}
