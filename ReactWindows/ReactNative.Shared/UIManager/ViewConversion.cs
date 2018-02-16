// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using System;
#if WINDOWS_UWP
using Windows.UI.Xaml;
#else
using System.Windows;
#endif
using static System.FormattableString;

namespace ReactNative.UIManager
{
    static class ViewConversion
    {
        public static DependencyObject GetDependencyObject(object view)
        {
            if (view is DependencyObject dependencyObject)
            {
                return dependencyObject;
            }
#if XAMLBASIC
            else if (view is IXamlBasicObject xamlBasicObject)
            {
                return XamlBasic.GetDependencyObject(xamlBasicObject);
            }
#endif

            throw new InvalidOperationException("Cannot convert view to DependencyObject.");
        }

        public static T GetDependencyObject<T>(object view)
            where T : DependencyObject
        {
            if (GetDependencyObject(view) is T typedView)
            {
                return typedView;
            }

            throw new InvalidOperationException(Invariant($"Cannot convert view to '{typeof(T)}'."));
        }
#if XAMLBASIC

        public static IXamlBasicObject GetXamlBasicObject(object view)
        {
            if (view is IXamlBasicObject xamlBasicObject)
            {
                return xamlBasicObject;
            }
            else if (view is DependencyObject dependencyObject)
            {
                return XamlBasic.GetXamlBasicObject(dependencyObject);
            }

            throw new InvalidOperationException("Cannot convert view to DependencyObject.");
        }
#endif
    }
}
