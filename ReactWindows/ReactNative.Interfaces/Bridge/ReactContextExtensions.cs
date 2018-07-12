// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using System;

namespace ReactNative.Core.Bridge
{
    public static class ReactContextExtensions
    {
        public static void AssertOnJavaScriptActionQueue(this IReactContext reactContext)
        {
            if (!reactContext.IsOnJavaScriptActionQueue())
            {
                throw new InvalidOperationException("Thread access assertion failed.");
            }
        }

        public static void AssertOnNativeModulesActionQueue(this IReactContext reactContext)
        {
            if (!reactContext.IsOnNativeModulesActionQueue())
            {
                throw new InvalidOperationException("Thread access assertion failed.");
            }
        }
    }
}
