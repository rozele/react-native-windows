#if XAMLDIRECT
// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using Microsoft.UI.Xaml.Core.Direct;
using Newtonsoft.Json.Linq;
using ReactNative.Accessibility;
using ReactNative.Reflection;
using ReactNative.Touch;
using ReactNative.UIManager.Annotations;
using System.Collections.Concurrent;
using System.Diagnostics;
using Windows.Foundation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Automation.Peers;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Media3D;

namespace ReactNative.UIManager
{
    /// <summary>
    /// Base class that should be suitable for the majority of subclasses of <see cref="IViewManager"/>.
    /// It provides support for base view props such as opacity, etc.
    /// </summary>
    /// <typeparam name="TLayoutShadowNode">Type of shadow node.</typeparam>
    public abstract class XamlDirectBaseViewManager<TLayoutShadowNode> :
            XamlDirectViewManager<TLayoutShadowNode>
        where TLayoutShadowNode : LayoutShadowNode
    {
        private static readonly SizeChangedEventHandler s_sizeChangedHandler = OnSizeChanged;
        private static readonly PointerEventHandler s_pointerEnteredHandler = OnPointerEntered;
        private static readonly PointerEventHandler s_pointerExitedHandler = OnPointerExited;

        private readonly ConcurrentDictionary<XamlDirectObject, DimensionBoundProperties> _dimensionBoundProperties =
            new ConcurrentDictionary<XamlDirectObject, DimensionBoundProperties>();

        /// <summary>
        /// Sets the 3D tranform on the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="transforms">
        /// The transform matrix or the list of transforms.
        /// </param>
        [ReactProp("transform")]
        public void SetTransform(XamlDirectObject view, JArray transforms)
        {
            if (transforms == null)
            {
                var dimensionBoundProperties = GetDimensionBoundProperties(view);
                if (dimensionBoundProperties?.MatrixTransform != null)
                {
                    dimensionBoundProperties.MatrixTransform = null;
                    ResetProjectionMatrix(view);
                    ResetRenderTransform(view);
                }
            }
            else
            {
                var dimensionBoundProperties = GetOrCreateDimensionBoundProperties(view);
                dimensionBoundProperties.MatrixTransform = transforms;
                var dimensions = GetDimensions(view);
                SetProjectionMatrix(view, dimensions, transforms);
            }
        }

        /// <summary>
        /// Sets the opacity of the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="opacity">The opacity value.</param>
        [ReactProp("opacity", DefaultDouble = 1.0)]
        public void SetOpacity(XamlDirectObject view, double opacity)
        {
            XamlDirect.GetDefault().SetDoubleProperty(
                view, 
                XamlPropertyIndex.UIElement_Opacity,
                opacity);
        }

        /// <summary>
        /// Sets the overflow prop for the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="overflow">The overflow value.</param>
        [ReactProp("overflow")]
        public void SetOverflow(XamlDirectObject view, string overflow)
        {
            if (overflow == "hidden")
            {
                var dimensionBoundProperties = GetOrCreateDimensionBoundProperties(view);
                dimensionBoundProperties.OverflowHidden = true;
                var dimensions = GetDimensions(view);
                SetOverflowHidden(view, dimensions);
                XamlDirect.GetDefault().AddEventHandler(view, XamlEventIndex.FrameworkElement_SizeChanged, s_sizeChangedHandler);
            }
            else
            {
                XamlDirect.GetDefault().RemoveEventHandler(view, XamlEventIndex.FrameworkElement_SizeChanged, s_sizeChangedHandler);
                var dimensionBoundProperties = GetDimensionBoundProperties(view);
                if (dimensionBoundProperties != null && dimensionBoundProperties.OverflowHidden)
                {
                    dimensionBoundProperties.OverflowHidden = false;
                    SetOverflowVisible(view);
                }
            }
        }

        /// <summary>
        /// Sets the z-index of the element.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="zIndex">The z-index.</param>
        [ReactProp("zIndex")]
        public void SetZIndex(XamlDirectObject view, int zIndex)
        {
            XamlDirect.GetDefault().SetInt32Property(
                view, 
                XamlPropertyIndex.Canvas_ZIndex, 
                zIndex);
        }

        /// <summary>
        /// Sets the display mode of the element.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="display">The display mode.</param>
        [ReactProp(ViewProps.Display)]
        public void SetDisplay(XamlDirectObject view, string display)
        {
            var visibility = display == "none" ? Visibility.Collapsed : Visibility.Visible;
            XamlDirect.GetDefault().SetEnumProperty(view, XamlPropertyIndex.UIElement_Visibility, (uint)visibility);
            // TODO: what to do for XamlDirect in AccessibilityHelpers?
            var element = (UIElement)XamlDirect.GetDefault().GetObject(view);
            AccessibilityHelper.OnElementChanged(element, UIElement.VisibilityProperty);
        }

        /// <summary>
        /// Sets the manipulation mode for the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="manipulationModes">The manipulation modes.</param>
        [ReactProp("manipulationModes")]
        public void SetManipulationModes(XamlDirectObject view, JArray manipulationModes)
        {
            if (manipulationModes == null)
            {
                XamlDirect.GetDefault().SetEnumProperty(
                    view, 
                    XamlPropertyIndex.UIElement_ManipulationMode, 
                    (uint)ManipulationModes.System);
                
                return;
            }

            var manipulationMode = ManipulationModes.System;
            foreach (var modeString in manipulationModes)
            {
                Debug.Assert(modeString.Type == JTokenType.String);
                var mode = EnumHelpers.Parse<ManipulationModes>(modeString.Value<string>());
                manipulationMode |= mode;
            }

            XamlDirect.GetDefault().SetEnumProperty(
                view, 
                XamlPropertyIndex.UIElement_ManipulationMode, 
                (uint)manipulationMode);
        }

        /// <summary>
        /// Sets the accessibility label of the element.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="label">The label.</param>
        [ReactProp(ViewProps.AccessibilityLabel)]
        public void SetAccessibilityLabel(XamlDirectObject view, string label)
        {
            // TODO: what to do for XamlDirect in AccessibilityHelpers?
            var element = (UIElement)XamlDirect.GetDefault().GetObject(view);
            AccessibilityHelper.SetAccessibilityLabel(element, label ?? "");
        }

        /// <summary>
        /// Sets the accessibility live region.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="liveRegion">The live region.</param>
        [ReactProp(ViewProps.AccessibilityLiveRegion)]
        public void SetAccessibilityLiveRegion(XamlDirectObject view, string liveRegion)
        {
            var liveSetting = AutomationLiveSetting.Off;
            switch (liveRegion)
            {
                case "polite":
                    liveSetting = AutomationLiveSetting.Polite;
                    break;
                case "assertive":
                    liveSetting = AutomationLiveSetting.Assertive;
                    break;
            }

            XamlDirect.GetDefault().SetEnumProperty(
                view, 
                XamlPropertyIndex.AutomationProperties_LiveSetting, 
                (uint)liveSetting);
        }

        /// <summary>
        /// Sets the test ID, i.e., the automation ID.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="testId">The test ID.</param>
        [ReactProp("testID")]
        public void SetTestId(XamlDirectObject view, string testId)
        {
            XamlDirect.GetDefault().SetStringProperty(
                view, 
                XamlPropertyIndex.AutomationProperties_AutomationId, 
                testId ?? "");
        }

        /// <summary>
        /// Sets a tooltip for the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="tooltip">String to display in the tooltip.</param>
        [ReactProp("tooltip")]
        public void SetTooltip(XamlDirectObject view, string tooltip)
        {
            XamlDirect.GetDefault().SetStringProperty(
                view, 
                XamlPropertyIndex.ToolTipService_ToolTip, 
                tooltip);
        }

        /// <summary>
        /// Called when view is detached from view hierarchy and allows for 
        /// additional cleanup by the <see cref="IViewManager"/> subclass.
        /// </summary>
        /// <param name="reactContext">The React context.</param>
        /// <param name="view">The view.</param>
        /// <remarks>
        /// Be sure to call this base class method to register for pointer 
        /// entered and pointer exited events.
        /// </remarks>
        public override void OnDropViewInstance(ThemedReactContext reactContext, XamlDirectObject view)
        {
            XamlDirect.GetDefault().RemoveEventHandler(
                view, 
                XamlEventIndex.UIElement_PointerEntered, 
                s_pointerEnteredHandler);

            XamlDirect.GetDefault().RemoveEventHandler(
                view, 
                XamlEventIndex.UIElement_PointerExited, 
                s_pointerExitedHandler);

            _dimensionBoundProperties.TryRemove(view, out _);
        }

        /// <summary>
        /// Sets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <param name="dimensions">The dimensions.</param>
        public override void SetDimensions(XamlDirectObject view, Dimensions dimensions)
        {
            var dimensionBoundProperties = GetDimensionBoundProperties(view);
            var matrixTransform = dimensionBoundProperties?.MatrixTransform;
            var overflowHidden = dimensionBoundProperties?.OverflowHidden ?? false;
            if (matrixTransform != null)
            {
                SetProjectionMatrix(view, dimensions, matrixTransform);
            }

            if (overflowHidden)
            {
                SetOverflowHidden(view, dimensions);
                XamlDirect.GetDefault().RemoveEventHandler(
                    view,
                    XamlEventIndex.FrameworkElement_SizeChanged,
                    s_sizeChangedHandler);
            }

            base.SetDimensions(view, dimensions);

            if (overflowHidden)
            {
                XamlDirect.GetDefault().AddEventHandler(
                    view, 
                    XamlEventIndex.FrameworkElement_SizeChanged, 
                    s_sizeChangedHandler);
            }
        }

        /// <summary>
        /// Subclasses can override this method to install custom event 
        /// emitters on the given view.
        /// </summary>
        /// <param name="reactContext">The React context.</param>
        /// <param name="view">The view instance.</param>
        /// <remarks>
        /// Consider overriding this method if your view needs to emit events
        /// besides basic touch events to JavaScript (e.g., scroll events).
        /// 
        /// Make sure you call the base implementation to ensure base pointer
        /// event handlers are subscribed.
        /// </remarks>
        protected override void AddEventEmitters(ThemedReactContext reactContext, XamlDirectObject view)
        {
            XamlDirect.GetDefault().AddEventHandler(
                view, 
                XamlEventIndex.UIElement_PointerEntered, 
                s_pointerEnteredHandler);

            XamlDirect.GetDefault().AddEventHandler(
                view, 
                XamlEventIndex.UIElement_PointerExited,
                s_pointerExitedHandler);
        }

        private static void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            var view = (FrameworkElement)sender; // TODO: confirm
            view.Clip = new RectangleGeometry
            {
                Rect = new Rect(0, 0, e.NewSize.Width, e.NewSize.Height),
            };
        }

        private static void OnPointerEntered(object sender, PointerRoutedEventArgs e)
        {
            var view = (DependencyObject)sender; // TODO: confirm
            TouchHandler.OnPointerEntered(view, e);
        }

        private static void OnPointerExited(object sender, PointerRoutedEventArgs e)
        {
            var view = (DependencyObject)sender; // TODO: confirm
            TouchHandler.OnPointerExited(view, e);
        }

        private DimensionBoundProperties GetDimensionBoundProperties(XamlDirectObject view)
        {
            if (!_dimensionBoundProperties.TryGetValue(view, out var properties))
            {
                properties = null;
            }

            return properties;
        }

        private DimensionBoundProperties GetOrCreateDimensionBoundProperties(XamlDirectObject view)
        {
            if (!_dimensionBoundProperties.TryGetValue(view, out var properties))
            {
                properties = new DimensionBoundProperties();
                _dimensionBoundProperties.AddOrUpdate(view, properties, (k, v) => properties);
            }

            return properties;
        }

        private static void SetProjectionMatrix(XamlDirectObject view, Dimensions dimensions, JArray transforms)
        {
            var transformMatrix = TransformHelper.ProcessTransform(transforms);

            var translateMatrix = Matrix3D.Identity;
            var translateBackMatrix = Matrix3D.Identity;
            if (!double.IsNaN(dimensions.Width))
            {
                translateMatrix.OffsetX = -dimensions.Width / 2;
                translateBackMatrix.OffsetX = dimensions.Width / 2;
            }

            if (!double.IsNaN(dimensions.Height))
            {
                translateMatrix.OffsetY = -dimensions.Height / 2;
                translateBackMatrix.OffsetY = dimensions.Height / 2;
            }

            var projectionMatrix = translateMatrix * transformMatrix * translateBackMatrix;
            ApplyProjection(view, projectionMatrix);
        }

        private static void ApplyProjection(XamlDirectObject view, Matrix3D projectionMatrix)
        {
            if (IsSimpleTranslationOnly(projectionMatrix))
            {
                ResetProjectionMatrix(view);
                // We need to use a new instance of MatrixTransform because matrix
                // updates to an existing MatrixTransform don't seem to take effect.
                var transform = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.MatrixTransform);
                var matrix = Matrix.Identity;
                matrix.OffsetX = projectionMatrix.OffsetX;
                matrix.OffsetY = projectionMatrix.OffsetY;
                XamlDirect.GetDefault().SetMatrixProperty(
                    transform, 
                    XamlPropertyIndex.MatrixTransform_Matrix, 
                    matrix);

                XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                    view, 
                    XamlPropertyIndex.UIElement_RenderTransform, 
                    transform);
            }
            else
            {
                ResetRenderTransform(view);
                var projection = EnsureProjection(view);
                XamlDirect.GetDefault().SetMatrix3DProperty(
                    projection, 
                    XamlPropertyIndex.Matrix3DProjection_ProjectionMatrix, 
                    projectionMatrix);
            }
        }

        private static bool IsSimpleTranslationOnly(Matrix3D matrix)
        {
            // Matrix3D is a struct and passed-by-value. As such, we can modify
            // the values in the matrix without affecting the caller.
            matrix.OffsetX = matrix.OffsetY = 0;
            return matrix.IsIdentity;
        }

        private static void ResetProjectionMatrix(XamlDirectObject view)
        {
            XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                view, 
                XamlPropertyIndex.Matrix3DProjection_ProjectionMatrix, 
                null);
        }

        private static void ResetRenderTransform(XamlDirectObject view)
        {
            XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                view, 
                XamlPropertyIndex.UIElement_RenderTransform, 
                null);
        }

        private static XamlDirectObject EnsureProjection(XamlDirectObject view)
        {
            // TODO: type check?
            var matrixProjection = XamlDirect.GetDefault().GetXamlDirectObjectProperty(
                view, 
                XamlPropertyIndex.UIElement_Projection);

            if (matrixProjection == null)
            {
                matrixProjection = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.Matrix3DProjection);
                XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                    view, 
                    XamlPropertyIndex.UIElement_Projection, 
                    matrixProjection);
            }

            return matrixProjection;
        }

        private static void SetOverflowHidden(XamlDirectObject view, Dimensions dimensions)
        {
            if (double.IsNaN(dimensions.Width) || double.IsNaN(dimensions.Height))
            {
                XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                    view, 
                    XamlPropertyIndex.UIElement_Clip, 
                    null);
            }
            else
            {
                var geometry = XamlDirect.GetDefault().CreateInstance(XamlTypeIndex.RectangleGeometry);
                XamlDirect.GetDefault().SetRectProperty(
                    geometry, 
                    XamlPropertyIndex.RectangleGeometry_Rect,
                    new Rect(0, 0, dimensions.Width, dimensions.Height));

                XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                    view, 
                    XamlPropertyIndex.UIElement_Clip, 
                    geometry);
            }
        }

        private static void SetOverflowVisible(XamlDirectObject view)
        {
            XamlDirect.GetDefault().SetXamlDirectObjectProperty(
                view, 
                XamlPropertyIndex.UIElement_Clip,
                null);
        }

        class DimensionBoundProperties
        {
            public bool OverflowHidden { get; set; }

            public JArray MatrixTransform { get; set; }
        }
    }
}
#endif
