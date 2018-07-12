// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using System;

namespace ReactNative.Core.Bridge
{
    /// <summary>
    /// Abstract context wrapper for the React instance to manage
    /// lifecycle events.
    /// </summary>
    public interface IReactContext
    {
        /// <summary>
        /// Gets the instance of the <see cref="IJavaScriptModule"/> associated
        /// with the <see cref="IReactInstance"/>.
        /// </summary>
        /// <typeparam name="T">Type of JavaScript module.</typeparam>
        /// <returns>The JavaScript module instance.</returns>
        T GetJavaScriptModule<T>() where T : IJavaScriptModule, new();

        /// <summary>
        /// Gets the instance of the <see cref="INativeModule"/> associated
        /// with the <see cref="IReactInstance"/>.
        /// </summary>
        /// <typeparam name="T">Type of native module.</typeparam>
        /// <returns>The native module instance.</returns>
        T GetNativeModule<T>() where T : INativeModule;

        /// <summary>
        /// Adds a lifecycle event listener to the context.
        /// </summary>
        /// <param name="listener">The listener.</param>
        void AddLifecycleEventListener(ILifecycleEventListener listener);

        /// <summary>
        /// Removes a lifecycle event listener from the context.
        /// </summary>
        /// <param name="listener">The listener.</param>
        void RemoveLifecycleEventListener(ILifecycleEventListener listener);

        /// <summary>
        /// Adds a background event listener to the context.
        /// </summary>
        /// <param name="listener">The listener.</param>
        void AddBackgroundEventListener(IBackgroundEventListener listener);

        /// <summary>
        /// Removes a background event listener from the context.
        /// </summary>
        /// <param name="listener">The listener.</param>
        void RemoveBackgroundEventListener(IBackgroundEventListener listener);

        /// <summary>
        /// Checks if the current thread is running in the context of the React
        /// instance JavaScript action queue.
        /// </summary>
        /// <returns>
        /// <b>true</b> if the call is from the JavaScript action queue,
        /// <b>false</b> otherwise.
        /// </returns>
        bool IsOnJavaScriptActionQueue();

        /// <summary>
        /// Enqueues an action on the JavaScript action queue.
        /// </summary>
        /// <param name="action">The action.</param>
        void RunOnJavaScriptActionQueue(Action action);

        /// <summary>
        /// Checks if the current thread is running in the context of the React
        /// instance native modules action queue.
        /// </summary>
        /// <returns>
        /// <b>true</b> if the call is from the native modules action queue,
        /// <b>false</b> otherwise.
        /// </returns>
        bool IsOnNativeModulesActionQueue();

        /// <summary>
        /// Enqueues an action on the native modules action queue.
        /// </summary>
        /// <param name="action">The action.</param>
        void RunOnNativeModulesActionQueue(Action action);

        /// <summary>
        /// Passes the exception to the configured exception handling mechanism,
        /// or rethrows if none is configured.
        /// </summary>
        /// <param name="exception">The exception.</param>
        void HandleException(Exception exception);
    }
}
