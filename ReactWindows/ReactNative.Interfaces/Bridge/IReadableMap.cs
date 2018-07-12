// Copyright(c) Microsoft Corporation.All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;

namespace ReactNative.Core.Bridge
{
    public interface IReadableMap : IReadableValue
    {
        IEnumerable<string> Keys { get; }

        bool HasKey(string name);

        bool IsNull(string name);

        bool GetBoolean(String name);

        double GetDouble(String name);

        int GetInt(String name);

        string GetString(String name);

        IReadableArray GetArray(String name);

        IReadableMap GetMap(String name);

        ReadableType GetType(String name);
    }
}
