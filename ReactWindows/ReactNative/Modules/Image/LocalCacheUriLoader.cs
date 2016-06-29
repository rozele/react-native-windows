using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Streams;

namespace ReactNative.Modules.Image
{
    class LocalCacheUriLoader : IUriLoader
    {
        private const int BufferSize = 8192;

        private readonly IDictionary<string, Task> _downloading =
            new Dictionary<string, Task>();

        private readonly string _subfolder;

        public LocalCacheUriLoader(string subfolder)
        {
            _subfolder = subfolder;
        }

        public async Task PrefetchAsync(string uri)
        {
            var task = default(Task);
            var taskCompletionSource = default(TaskCompletionSource<bool>);
            lock (_downloading)
            {
                if (!_downloading.TryGetValue(uri, out task))
                {
                    taskCompletionSource = new TaskCompletionSource<bool>();
                    _downloading.Add(uri, taskCompletionSource.Task);
                }
            }

            if (taskCompletionSource != null)
            {
                var cacheKey = CreateKey(uri);
                var folder = await ApplicationData.Current.LocalCacheFolder
                    .CreateFolderAsync(
                        _subfolder,
                        CreationCollisionOption.OpenIfExists);

                var itemInfo = await folder.TryGetItemAsync(cacheKey);
                if (itemInfo == null)
                {
                    var file = await folder.CreateFileAsync(cacheKey, CreationCollisionOption.ReplaceExisting);
                    using (var imageStream = await OpenReadCoreAsync(uri))
                    using (var inputStream = imageStream.AsStreamForRead())
                    using (var fileStream = await file.OpenAsync(FileAccessMode.ReadWrite))
                    using (var outputStream = fileStream.AsStreamForWrite())
                    {
                        var buffer = new byte[BufferSize];
                        var read = int.MaxValue;
                        while (read > 0)
                        {
                            read = await inputStream.ReadAsync(buffer, 0, BufferSize);
                            await outputStream.WriteAsync(buffer, 0, read);
                        }
                    }
                }

                taskCompletionSource.SetResult(true);
            }
            else
            {
                await task;
            }
        }

        public async Task<IRandomAccessStreamWithContentType> OpenReadAsync(string uri)
        {
            // Check if file is being downloaded
            var task = default(Task);
            lock (_downloading)
            {
                _downloading.TryGetValue(uri, out task);
            }

            if (task != null)
            {
                await task;
            }

            // Check if file is in folder
            var folder = await ApplicationData.Current.LocalCacheFolder
                .CreateFolderAsync(
                    _subfolder,
                    CreationCollisionOption.OpenIfExists);

            var cacheKey = CreateKey(uri);
            var itemInfo = await folder.TryGetItemAsync(cacheKey);
            if (itemInfo != null)
            {
                var file = await folder.GetFileAsync(cacheKey);
                file.OpenSequentialReadAsync();
                return await file.OpenReadAsync();
            }

            return await OpenReadCoreAsync(uri);
        }

        private async Task<IRandomAccessStreamWithContentType> OpenReadCoreAsync(string uri)
        {
            var streamRef = RandomAccessStreamReference.CreateFromUri(new Uri(uri));
            return await streamRef.OpenReadAsync();
        }

        private static readonly IDictionary<char, string> s_urlSpecialChars =
            new Dictionary<char, string>
            {
                { '!',"%21" },
                { '#',"%23" },
                { '$',"%24" },
                { '&',"%26" },
                { '\'',"%27" },
                { '(',"%28" },
                { ')',"%29" },
                { '*',"%2A" },
                { '+',"%2B" },
                { ',',"%2C" },
                { '/',"%2F" },
                { ':',"%3A" },
                { ';',"%3B" },
                { '=',"%3D" },
                { '?',"%3F" },
                { '@',"%40" },
	            { '[',"%5B" },
                { ']',"%5D" },
            };

        private static string CreateKey(string uriString)
        {
            var builder = new StringBuilder();
            foreach (var c in uriString)
            {
                var replacement = default(string);
                if (s_urlSpecialChars.TryGetValue(c, out replacement))
                {
                    builder.Append(replacement);
                }
                else
                {
                    builder.Append(c);
                }
            }

            return builder.ToString();
        }
    }
}
