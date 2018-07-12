// Copyright(c) Microsoft Corporation.All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

namespace ReactNative.Core.Bridge
{
    public interface IReadableArray : IReadableValue
    {
        int Count { get; }

        bool IsNull(int index);

        bool GetBoolean(int index);

        double GetDouble(int index);

        int GetInt(int index);

        string GetString(int index);

        IReadableArray GetArray(int index);

        IReadableMap GetMap(int index);

        ReadableType GetType(int index);
    }
}
