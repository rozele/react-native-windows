// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using System;

namespace ReactNative.Core.Bridge
{
    /// <summary>
    /// A simple action queue that allows us to control the order certain
    /// callbacks are executed within a given frame.
    /// </summary>
    public interface IReactChoreographer : IDisposable
    {
        /// <summary>
        /// For use by <see cref="UIManager.UIManagerModule"/>. 
        /// </summary>
        event EventHandler<IFrameEventArgs> DispatchUICallback;

        /// <summary>
        /// For use by <see cref="Animated.NativeAnimatedModule"/>. 
        /// </summary>
        event EventHandler<IFrameEventArgs> NativeAnimatedCallback;

        /// <summary>
        /// For events that make JavaScript do things.
        /// </summary>
        event EventHandler<IFrameEventArgs> JavaScriptEventsCallback;

        /// <summary>
        /// Event used to trigger the idle callback. Called after all UI work has been
        /// dispatched to JavaScript.
        /// </summary>
        event EventHandler<IFrameEventArgs> IdleCallback;

        /// <summary>
        /// Activate the callback for the given key.
        /// </summary>
        /// <param name="callbackKey">The callback key.</param>
        void ActivateCallback(string callbackKey);

        /// <summary>
        /// Deactivate the callback for the given key.
        /// </summary>
        /// <param name="callbackKey">The callback key.</param>
        void DeactivateCallback(string callbackKey);
    }
}
