// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using System.Threading;
using System.Threading.Tasks;

namespace ReactNative.Core.Common
{
    public static class AsyncDisposableExtensions
    {
        public static Task DisposeAsync(this IAsyncDisposable disposable)
        {
            return disposable.DisposeAsync(CancellationToken.None);
        }
    }
}
