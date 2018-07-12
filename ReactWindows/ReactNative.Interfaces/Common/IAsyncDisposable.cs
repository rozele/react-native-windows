// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using System.Threading;
using System.Threading.Tasks;

namespace ReactNative.Core.Common
{
    /// <summary>
    /// A resource that can be disposed asynchronously.
    /// </summary>
    public interface IAsyncDisposable
    {
        /// <summary>
        /// Asynchronously disposes the instance.
        /// </summary>
        /// <param name="token">The cancellation token.</param>
        /// <returns>A task to await dispose operation.</returns>
        Task DisposeAsync(CancellationToken token);
    }
}
