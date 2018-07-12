namespace ReactNative.Core.Bridge
{
    public interface IReactInstance
    {
        /// <summary>
        /// Invokes a JavaScript callback.
        /// </summary>
        /// <param name="callbackId">The callback ID.</param>
        /// <param name="arguments">The arguments.</param>
        void InvokeCallback(int callbackId, IReadableArray arguments);
    }
}
