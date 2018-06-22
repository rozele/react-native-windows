// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using Microsoft.UI.Xaml.Core.Direct;
using System.Threading;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace ReactNative.UIManager
{
    class XamlDirectBorderedCanvas
    {
        private static readonly ThreadLocal<XamlDirectObject> s_defaultBorderBrush =
            new ThreadLocal<XamlDirectObject>(() =>
            {
                var brush = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.SolidColorBrush);
                XamlDirect.GetDefault().SetColorProperty(brush, XamlPropertyIndex.SolidColorBrush_Color, Colors.Black);
                return brush;
            });

        private static XamlDirectObject DefaultBorderBrush => s_defaultBorderBrush.Value;

        private readonly XamlDirectObject _view;

        private XamlDirectObject _border = null;
        private XamlDirectObject _borderBrush = null;

        public XamlDirectBorderedCanvas(XamlDirectObject view)
        {
            _view = view;
        }

        public void SetBackgroundColor(uint? color)
        {
            if (_border == null)
            {
                if (color == null)
                {
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Background, null);
                }
                else
                {
                    var brush = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.SolidColorBrush);
                    XamlDirect.GetDefault().SetColorProperty(brush, XamlPropertyIndex.SolidColorBrush_Color, ColorHelpers.Parse(color.Value));
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Background, brush);
                }
            }
            else
            {
                if (color == null)
                {
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_border, XamlPropertyIndex.Border_Background, null);
                }
                else
                {
                    var brush = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.SolidColorBrush);
                    XamlDirect.GetDefault().SetColorProperty(brush, XamlPropertyIndex.SolidColorBrush_Color, ColorHelpers.Parse(color.Value));
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_border, XamlPropertyIndex.Border_Background, brush);
                }
            }
        }

        public CornerRadius GetCornerRadius()
        {
            if (_border == null)
            {
                return default(CornerRadius);
            }

            return XamlDirect.GetDefault().GetCornerRadiusProperty(_border, XamlPropertyIndex.Border_CornerRadius);
        }

        public void SetCornerRadius(CornerRadius cornerRadius)
        {
            var border = GetOrCreateBorder();
            XamlDirect.GetDefault().SetCornerRadiusProperty(
                border,
                XamlPropertyIndex.Border_CornerRadius,
                cornerRadius);
        }

        public void SetBorderColor(uint? color)
        {
            if (_border == null)
            {
                if (color == null)
                {
                    _borderBrush = null;
                }
                else
                {
                    _borderBrush = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.SolidColorBrush);
                    XamlDirect.GetDefault().SetColorProperty(
                        _borderBrush,
                        XamlPropertyIndex.SolidColorBrush_Color,
                        ColorHelpers.Parse(color.Value));
                }
            }
            else
            {
                if (color == null)
                {
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                        _border,
                        XamlPropertyIndex.Border_BorderBrush,
                        DefaultBorderBrush);
                }
                else
                {
                    var brush = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.SolidColorBrush);

                    XamlDirect.GetDefault().SetColorProperty(
                        brush,
                        XamlPropertyIndex.SolidColorBrush_Color,
                        ColorHelpers.Parse(color.Value));

                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                        _border,
                        XamlPropertyIndex.Border_BorderBrush,
                        brush);
                }
            }
        }

        public void SetBorderWidth(int spacingType, double width)
        {
            var border = GetOrCreateBorder();
            var thickness = XamlDirect.GetDefault().GetThicknessProperty(border, XamlPropertyIndex.Border_BorderThickness);
            switch (spacingType)
            {
                case EdgeSpacing.Left:
                    thickness.Left = width;
                    break;
                case EdgeSpacing.Top:
                    thickness.Top = width;
                    break;
                case EdgeSpacing.Right:
                    thickness.Right = width;
                    break;
                case EdgeSpacing.Bottom:
                    thickness.Bottom = width;
                    break;
                case EdgeSpacing.All:
                    thickness = new Thickness(width);
                    break;
            }

            XamlDirect.GetDefault().SetThicknessProperty(border, XamlPropertyIndex.Border_BorderThickness, thickness);
        }

        public void AddView(XamlDirectObject child, int index)
        {
            var offset = _border != null ? 1 : 0;
            var children = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Children);
            XamlDirect.GetDefault().InsertIntoCollectionAt(children, (uint)(index + offset), child);
        }

        public XamlDirectObject GetChildAt(int index)
        {
            var offset = _border != null ? 1 : 0;
            var children = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Children);
            return XamlDirect.GetDefault().GetXamlDirectObjectFromCollectionAt(children, (uint)(index + offset));
        }

        public int GetChildCount()
        {
            var offset = _border != null ? 1 : 0;
            var children = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Children);
            var count = XamlDirect.GetDefault().GetCollectionCount(children);
            return (int)(count - offset);
        }

        public void RemoveAllChildren()
        {
            for (var i = 0; i < GetChildCount(); ++i)
            {
                RemoveChildAt(i);
            }
        }

        public void RemoveChildAt(int index)
        {
            var offset = _border != null ? 1 : 0;
            var children = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Children);
            XamlDirect.GetDefault().RemoveFromCollectionAt(children, (uint)(index + offset));
        }

        private XamlDirectObject GetOrCreateBorder()
        {
            if (_border == null)
            {
                // Create border with current or default brush
                _border = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.Border);
                XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                    _border,
                    XamlPropertyIndex.Border_BorderBrush,
                    _borderBrush ?? DefaultBorderBrush);

                // Bind border width and height to canvas dimensions
                // TODO: remove FrameworkElement dependency
                var element = (FrameworkElement)XamlDirect.GetDefault().GetObject(_border);
                element.SetBinding(FrameworkElement.WidthProperty, new Binding
                {
                    Source = XamlDirect.GetDefault().GetObject(_view),
                    Path = new PropertyPath("Width")
                });

                element.SetBinding(FrameworkElement.HeightProperty, new Binding
                {
                    Source = XamlDirect.GetDefault().GetObject(_view),
                    Path = new PropertyPath("Height")
                });

                // Transfer background to border
                var background = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Background);
                if (background != null)
                {
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_border, XamlPropertyIndex.Border_Background, background);
                    XamlDirect.GetDefault().SetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Background, null);
                }

                // Add border to children
                var children = XamlDirect.GetDefault().GetXamlDirectObjectProperty(_view, XamlPropertyIndex.Panel_Children);
                XamlDirect.GetDefault().InsertIntoCollectionAt(children, 0, _border);
            }

            return _border;
        }
    }
}
