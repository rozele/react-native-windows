#if XAMLDIRECT
// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using Microsoft.UI.Xaml.Core.Direct;

namespace ReactNative.UIManager
{
    /// <summary>
    /// Class responsible for knowing how to create and update views of a given
    /// type. It is also responsible for creating and updating
    /// <see cref="ReactShadowNode"/> subclasses used for calculating position
    /// and size for the corresponding native view.
    /// </summary>
    /// <typeparam name="TReactShadowNode">The shadow node type.</typeparam>
    public abstract class XamlDirectViewManager<TReactShadowNode> : ViewManagerBase<XamlDirectObject, TReactShadowNode>
        where TReactShadowNode : ReactShadowNode
    {
        /// <summary>
        /// Gets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <returns>The view dimensions.</returns>
        public sealed override Dimensions GetDimensions(XamlDirectObject view)
        {
            return new Dimensions
            {
                X = XamlDirect.GetDefault().GetDoubleProperty(view, XamlPropertyIndex.Canvas_Left),
                Y = XamlDirect.GetDefault().GetDoubleProperty(view, XamlPropertyIndex.Canvas_Top),
                Width = XamlDirect.GetDefault().GetDoubleProperty(view, XamlPropertyIndex.FrameworkElement_Width),
                Height = XamlDirect.GetDefault().GetDoubleProperty(view, XamlPropertyIndex.FrameworkElement_Height),
            };
        }

        /// <summary>
        /// Sets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <param name="dimensions">The output buffer.</param>
        public override void SetDimensions(XamlDirectObject view, Dimensions dimensions)
        {
            XamlDirect.GetDefault().SetDoubleProperty(view, XamlPropertyIndex.Canvas_Left, dimensions.X);
            XamlDirect.GetDefault().SetDoubleProperty(view, XamlPropertyIndex.Canvas_Top, dimensions.Y);
            XamlDirect.GetDefault().SetDoubleProperty(view, XamlPropertyIndex.FrameworkElement_Width, dimensions.Width);
            XamlDirect.GetDefault().SetDoubleProperty(view, XamlPropertyIndex.FrameworkElement_Height, dimensions.Height);
        }
    }
}
#endif
