using Windows.UI.Xaml;

namespace ReactNative.UIManager
{
    /// <summary>
    /// Class responsible for knowing how to create and update views of a given
    /// type. It is also responsible for creating and updating
    /// <see cref="ReactShadowNode"/> subclasses used for calculating position
    /// and size for the corresponding native view.
    /// </summary>
    /// <typeparam name="TReactShadowNode">The shadow node type.</typeparam>
    public abstract class XamlBasicViewManager<TReactShadowNode> : ViewManagerBase<IXamlBasicObject, TReactShadowNode>
        where TReactShadowNode : ReactShadowNode
    {
        /// <summary>
        /// Gets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <returns>The view dimensions.</returns>
        public sealed override Dimensions GetDimensions(IXamlBasicObject view)
        {
            return new Dimensions
            {
                X = XamlBasic.GetDoubleValue(view, XamlPropertyIndex.Canvas_Left),
                Y = XamlBasic.GetDoubleValue(view, XamlPropertyIndex.Canvas_Top),
                Width = XamlBasic.GetDoubleValue(view, XamlPropertyIndex.FrameworkElement_Width),
                Height = XamlBasic.GetDoubleValue(view, XamlPropertyIndex.FrameworkElement_Height),
            };
        }

        /// <summary>
        /// Sets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <param name="dimensions">The output buffer.</param>
        public override void SetDimensions(IXamlBasicObject view, Dimensions dimensions)
        {
            XamlBasic.SetValue(view, XamlPropertyIndex.Canvas_Left, dimensions.X);
            XamlBasic.SetValue(view, XamlPropertyIndex.Canvas_Top, dimensions.Y);
            XamlBasic.SetValue(view, XamlPropertyIndex.FrameworkElement_Width, dimensions.Width);
            XamlBasic.SetValue(view, XamlPropertyIndex.FrameworkElement_Height, dimensions.Height);
        }
    }
}
