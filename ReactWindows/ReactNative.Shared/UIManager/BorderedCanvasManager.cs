#if WINDOWS_UWP
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;
#else
using System.Windows;
using System.Windows.Controls;
#endif

namespace ReactNative.UIManager
{
    /// <summary>
    /// View parent manager for bordered canvases.
    /// </summary>
    public abstract class BorderedCanvasManager<TCanvas> : BorderedViewParentManager<TCanvas>
        where TCanvas : Canvas
    {
        static object _tag = new object();

        /// <summary>
        /// Checks if the Canvas has a Border already.
        /// </summary>
        protected override bool HasBorder(TCanvas view)
        {
            return GetBorder(view) != null;
        }

        /// <summary>
        /// Gets a Border associated with this Canvas or null if it doesn't have a border.
        /// </summary>
        private Border GetBorder(TCanvas view)
        {
            var border = view.Children[0] as Border;
            return border?.Tag == _tag ? border : null;
        }

        /// <summary>
        /// Adds a Border to a Canvas if it hasn't been added already.
        /// </summary>
        protected override Border GetOrCreateBorder(TCanvas view)
        {
            var border = GetBorder(view);

            if (border == null)
            {
                border = new Border { BorderBrush = s_defaultBorderBrush, Tag = _tag };
                view.Children.Insert(0, border);
            }

            return border;
        }

        /// <summary>
        /// Keeps the Border's dimensions in sync with with parent Canvas.
        /// </summary>
        public override void SetDimensions(TCanvas view, Dimensions dimensions)
        {
            base.SetDimensions(view, dimensions);

            var border = GetBorder(view);

            if (border != null)
            {
                border.Width = dimensions.Width;
                border.Height = dimensions.Height;
            }
        }

        /// <summary>
        /// Adds a child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="child">The child view.</param>
        /// <param name="index">The index.</param>
        public override void AddView(TCanvas parent, DependencyObject child, int index)
        {
            if (HasBorder(parent))
                index++;

            var uiElementChild = child.As<UIElement>();
            parent.Children.Insert(index, uiElementChild);
        }

        /// <summary>
        /// Gets the child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="index">The index.</param>
        /// <returns>The child view.</returns>
        public override DependencyObject GetChildAt(TCanvas parent, int index)
        {
            if (HasBorder(parent))
                index++;

            return parent.Children[index];
        }

        /// <summary>
        /// Gets the number of children in the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <returns>The number of children.</returns>
        public override int GetChildCount(TCanvas parent)
        {
            var count = parent.Children.Count;

            if (HasBorder(parent))
                count--;

            return count;
        }

        /// <summary>
        /// Removes all children from the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        public override void RemoveAllChildren(TCanvas parent)
        {
            for (var i = GetChildCount(parent) - 1; i >= 0; i--)
                RemoveChildAt(parent, i);
        }

        /// <summary>
        /// Removes the child at the given index.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <param name="index">The index.</param>
        public override void RemoveChildAt(TCanvas parent, int index)
        {
            if (HasBorder(parent))
                index++;

            parent.Children.RemoveAt(index);
        }
    }
}
