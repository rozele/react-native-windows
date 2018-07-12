// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using System;

namespace ReactNative.Core.Bridge
{
    public interface IFrameEventArgs
    {
        /// <summary>
        /// The relative frame time.
        /// </summary>
        TimeSpan RenderingTime { get; }

        /// <summary>
        /// The absolute frame time.
        /// </summary>
        DateTimeOffset FrameTime { get; }
    }
}
