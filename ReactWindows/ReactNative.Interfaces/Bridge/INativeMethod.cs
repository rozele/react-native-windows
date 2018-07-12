// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

namespace ReactNative.Core.Bridge
{
    /// <summary>
    /// An interface representing native methods.
    /// </summary>
    public interface INativeMethod
    {
        /// <summary>
        /// The type of method.
        /// </summary>
        string Type { get; }

        /// <summary>
        /// Invoke the native method.
        /// </summary>
        /// <param name="reactInstance">The React instance.</param>
        /// <param name="arguments">The arguments.</param>
        /// <returns>The native method result.</returns>
        IReadableValue Invoke(IReactInstance reactInstance, IReadableArray arguments);
    }
}
