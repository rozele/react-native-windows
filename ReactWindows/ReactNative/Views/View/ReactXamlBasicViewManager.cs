using ReactNative.UIManager;
using ReactNative.UIManager.Annotations;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ReactNative.Views.View
{
    class ReactXamlBasicViewManager : XamlBasicViewParentManager
    {
        public override string Name => ViewProps.ViewClassName;

        /// <summary>
        /// Sets whether the view is collapsible.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="collapsible">The flag.</param>
        [ReactProp(ViewProps.Collapsible)]
        public void SetCollapsible(IXamlBasicObject view, bool collapsible)
        {
            // no-op: it's here only so that "collapsable" property is exported to JS. The value is actually
            // handled in NativeViewHierarchyOptimizer
        }

        /// <summary>
        /// Sets whether or not the view is an accessibility element.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <param name="accessible">A flag indicating whether or not the view is an accessibility element.</param>
        [ReactProp("accessible")]
        public void SetAccessible(IXamlBasicObject view, bool accessible)
        {
            // TODO: #557 Provide implementation for View's accessible prop

            // We need to have this stub for this prop so that Views which
            // specify the accessible prop aren't considered to be layout-only.
            // The proper implementation is still to be determined.
        }

        /// <summary>
        /// Sets the background color of the view.
        /// </summary>
        /// <param name="view">The view panel.</param>
        /// <param name="color">The masked color value.</param>
        [ReactProp(
            ViewProps.BackgroundColor,
            CustomType = "Color",
            DefaultUInt32 = ColorHelpers.Transparent)]
        public void SetBackgroundColor(IXamlBasicObject view, uint color)
        {
            var brush = XamlBasic.GetXamlBasicObjectValue(view, XamlPropertyIndex.Panel_Background);
            if (brush == null)
            {
                brush = XamlBasic.CreateInstance(XamlTypeIndex.SolidColorBrush);
                XamlBasic.SetValue(view, XamlPropertyIndex.Panel_Background, brush);
            }

            XamlBasic.SetValue(brush, XamlPropertyIndex.SolidColorBrush_Color, ColorHelpers.Parse(color));
        }

        public override void AddView(IXamlBasicObject parent, IXamlBasicObject child, int index)
        {
            // TODO: how to add child at specific index?
            XamlBasic.AddToCollection(
                XamlBasic.GetXamlBasicObjectValue(parent, XamlPropertyIndex.Panel_Children),
                child);
        }

        public override IXamlBasicObject GetChildAt(IXamlBasicObject parent, int index)
        {
            // TODO: how to get child at specific index?
            return XamlBasic.GetXamlBasicObject(
                ((Panel)XamlBasic.GetDependencyObject(parent)).Children[index]);
        }

        public override int GetChildCount(IXamlBasicObject parent)
        {
            // TODO: how to get children count
            return ((Panel)XamlBasic.GetDependencyObject(parent)).Children.Count;
        }

        public override void RemoveAllChildren(IXamlBasicObject parent)
        {
            XamlBasic.ClearCollection(XamlBasic.GetXamlBasicObjectValue(parent, XamlPropertyIndex.Panel_Children));
        }

        public override void RemoveChildAt(IXamlBasicObject parent, int index)
        {
            // TODO: how to remove child at specific index?
            ((Panel)XamlBasic.GetDependencyObject(parent)).Children.RemoveAt(index);
        }

        protected override IXamlBasicObject CreateViewInstance(ThemedReactContext reactContext)
        {
            return XamlBasic.CreateInstance(XamlTypeIndex.Canvas);
        }
    }
}
