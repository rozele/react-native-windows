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
    /// <summary>
    /// Helpers to convert between view types.
    /// </summary>
    public static class ViewConversion
    {
        /// <summary>
        /// Gets the <see cref="DependencyObject"/> from the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <returns>The converted view.</returns>
        public static DependencyObject GetDependencyObject(object view)
        {
            if (view is DependencyObject dependencyObject)
            {
                return dependencyObject;
            }
#if XAMLDIRECT
            else if (view is Microsoft.UI.Xaml.Core.Direct.XamlDirectObject xamlDirectObject)
            {
                var convertedObject = Microsoft.UI.Xaml.Core.Direct.XamlDirect.GetDefault().GetObject(xamlDirectObject);
                if (convertedObject is DependencyObject convertedDependencyObject)
                {
                    return convertedDependencyObject;
                }
            }
#endif

            throw new InvalidOperationException("Cannot convert view to DependencyObject.");
        }

        /// <summary>
        /// Gets the <see cref="DependencyObject"/> from the view.
        /// </summary>
        /// <typeparamref name="T">
        /// The type of <see cref="DependencyObject"/>.
        /// </typeparamref>
        /// <param name="view">The view.</param>
        /// <returns>The converted view.</returns>
        public static T GetDependencyObject<T>(object view)
            where T : DependencyObject
        {
            if (GetDependencyObject(view) is T typedView)
            {
                return typedView;
            }

            throw new InvalidOperationException(Invariant($"Cannot convert view to '{typeof(T)}'."));
        }
#if XAMLDIRECT

        /// <summary>
        /// Gets the <see cref="Microsoft.UI.Xaml.Core.Direct.XamlDirectObject"/> from the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <returns>The converted view.</returns>
        public static Microsoft.UI.Xaml.Core.Direct.XamlDirectObject GetXamlDirectObject(object view)
        {
            if (view is Microsoft.UI.Xaml.Core.Direct.XamlDirectObject xamlDirectObject)
            {
                return xamlDirectObject;
            }
            else if (view is DependencyObject dependencyObject)
            {
                return Microsoft.UI.Xaml.Core.Direct.XamlDirect.GetDefault().GetXamlDirectObject(dependencyObject);
            }

            throw new InvalidOperationException("Cannot convert view to DependencyObject.");
        }
#endif
    }
}
