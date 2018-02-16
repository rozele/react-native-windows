using Newtonsoft.Json.Linq;
using ReactNative.Reflection;
using ReactNative.Touch;
using ReactNative.UIManager.Annotations;
using System;
using System.Collections.Generic;
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
    /// It provides support for base view properties such as opacity, etc.
    /// </summary>
    /// <typeparam name="TLayoutShadowNode">Type of shadow node.</typeparam>
    public abstract class XamlBasicBaseViewManager<TLayoutShadowNode> : XamlBasicViewManager<TLayoutShadowNode>
        where TLayoutShadowNode : LayoutShadowNode
    {
        private readonly IDictionary<IXamlBasicObject, DimensionBoundProperties> _dimensionBoundProperties =
            new Dictionary<IXamlBasicObject, DimensionBoundProperties>();

        /// <summary>
        /// Set's the view styling layout properties, based on the <see cref="JObject"/> map.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="transforms">The list of transforms.</param>
        [ReactProp("transform")]
        public void SetTransform(IXamlBasicObject view, JArray transforms)
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
        public void SetOpacity(IXamlBasicObject view, double opacity)
        {
            XamlBasic.SetValue(
                view,
                XamlPropertyIndex.UIElement_Opacity,
                opacity);
        }

        /// <summary>
        /// Sets the overflow property for the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="overflow">The overflow value.</param>
        [ReactProp("overflow")]
        public void SetOverflow(IXamlBasicObject view, string overflow)
        {
            var element = ViewConversion.GetDependencyObject<FrameworkElement>(view);
            if (overflow == "hidden")
            {
                var dimensionBoundProperties = GetOrCreateDimensionBoundProperties(view);
                dimensionBoundProperties.OverflowHidden = true;
                var dimensions = GetDimensions(view);
                SetOverflowHidden(view, dimensions);
                element.SizeChanged += OnSizeChanged;
            }
            else
            {
                element.SizeChanged -= OnSizeChanged;
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
        public void SetZIndex(IXamlBasicObject view, int zIndex)
        {
            XamlBasic.SetValue(
                view,
                XamlPropertyIndex.Canvas_ZIndex,
                zIndex);
        }

        /// <summary>
        /// Sets the manipulation mode for the view.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="manipulationModes">The manipulation modes.</param>
        [ReactProp("manipulationModes")]
        public void SetManipulationModes(IXamlBasicObject view, JArray manipulationModes)
        {
            if (manipulationModes == null)
            {
                XamlBasic.SetValue(
                    view,
                    XamlPropertyIndex.UIElement_ManipulationMode,
                    (int)ManipulationModes.System);

                return;
            }

            var manipulationMode = ManipulationModes.System;
            foreach (var modeString in manipulationModes)
            {
                Debug.Assert(modeString.Type == JTokenType.String);
                var mode = EnumHelpers.Parse<ManipulationModes>(modeString.Value<string>());
                manipulationMode |= mode;
            }

            XamlBasic.SetValue(
                view,
                XamlPropertyIndex.UIElement_ManipulationMode,
                (int)manipulationMode);
        }

        /// <summary>
        /// Sets the accessibility label of the element.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="label">The label.</param>
        [ReactProp("accessibilityLabel")]
        public void SetAccessibilityLabel(IXamlBasicObject view, string label)
        {
            XamlBasic.SetValue(
                view,
                XamlPropertyIndex.AutomationProperties_Name,
                label ?? "");
        }

        /// <summary>
        /// Sets the accessibility live region.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="liveRegion">The live region.</param>
        [ReactProp("accessibilityLiveRegion")]
        public void SetAccessibilityLiveRegion(IXamlBasicObject view, string liveRegion)
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

            XamlBasic.SetValue(
                view,
                XamlPropertyIndex.AutomationProperties_LiveSetting,
                liveSetting);
        }

        /// <summary>
        /// Sets the test ID, i.e., the automation ID.
        /// </summary>
        /// <param name="view">The view instance.</param>
        /// <param name="testId">The test ID.</param>
        [ReactProp("testID")]
        public void SetTestId(IXamlBasicObject view, string testId)
        {
            XamlBasic.SetValue(
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
        public void SetTooltip(IXamlBasicObject view, string tooltip)
        {
            XamlBasic.SetValue(
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
        public override void OnDropViewInstance(ThemedReactContext reactContext, IXamlBasicObject view)
        {
            // TODO: only subscribe to enter and exit events when needed.
            var element = ViewConversion.GetDependencyObject<FrameworkElement>(view);
            element.PointerEntered -= OnPointerEntered;
            element.PointerExited -= OnPointerExited;
            _dimensionBoundProperties.Remove(view);
        }

        /// <summary>
        /// Sets the dimensions of the view.
        /// </summary>
        /// <param name="view">The view.</param>
        /// <param name="dimensions">The dimensions.</param>
        public override void SetDimensions(IXamlBasicObject view, Dimensions dimensions)
        {
            var dimensionBoundProperties = GetDimensionBoundProperties(view);
            var matrixTransform = dimensionBoundProperties?.MatrixTransform;
            var overflowHidden = dimensionBoundProperties?.OverflowHidden ?? false;

            if (matrixTransform != null)
            {
                SetProjectionMatrix(view, dimensions, matrixTransform);
            }

            var element = default(FrameworkElement);
            if (overflowHidden)
            {
                element = ViewConversion.GetDependencyObject<FrameworkElement>(view);
                SetOverflowHidden(view, dimensions);
                element.SizeChanged -= OnSizeChanged;
            }

            base.SetDimensions(view, dimensions);

            if (overflowHidden)
            {
                element.SizeChanged += OnSizeChanged;
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
        protected override void AddEventEmitters(ThemedReactContext reactContext, IXamlBasicObject view)
        {
            // TODO: only subscribe to enter and exit events when needed.
            var element = ViewConversion.GetDependencyObject<FrameworkElement>(view);
            element.PointerEntered += OnPointerEntered;
            element.PointerExited += OnPointerExited;
        }

        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            var view = (FrameworkElement)sender;
            view.Clip = new RectangleGeometry
            {
                Rect = new Rect(0, 0, e.NewSize.Width, e.NewSize.Height),
            };
        }

        private void OnPointerEntered(object sender, PointerRoutedEventArgs e)
        {
            var view = (DependencyObject)sender;
            TouchHandler.OnPointerEntered(view, e);
        }

        private void OnPointerExited(object sender, PointerRoutedEventArgs e)
        {
            var view = (DependencyObject)sender;
            TouchHandler.OnPointerExited(view, e);
        }

        private DimensionBoundProperties GetDimensionBoundProperties(IXamlBasicObject view)
        {
            DimensionBoundProperties properties;
            if (!_dimensionBoundProperties.TryGetValue(view, out properties))
            {
                properties = null;
            }

            return properties;
        }

        private DimensionBoundProperties GetOrCreateDimensionBoundProperties(IXamlBasicObject view)
        {
            DimensionBoundProperties properties;
            if (!_dimensionBoundProperties.TryGetValue(view, out properties))
            {
                properties = new DimensionBoundProperties();
                _dimensionBoundProperties.Add(view, properties);
            }

            return properties;
        }

        private static void SetProjectionMatrix(IXamlBasicObject view, Dimensions dimensions, JArray transforms)
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

        private static void ApplyProjection(IXamlBasicObject view, Matrix3D projectionMatrix)
        {
            if (IsSimpleTranslationOnly(projectionMatrix))
            {
                ResetProjectionMatrix(view);
                // We need to use a new instance of MatrixTransform because matrix
                // updates to an existing MatrixTransform don't seem to take effect.
                var transform = XamlBasic.CreateInstance(XamlTypeIndex.MatrixTransform);
                var matrix = Matrix3D.Identity;
                matrix.OffsetX = projectionMatrix.OffsetX;
                matrix.OffsetY = projectionMatrix.OffsetY;
                // TODO: can we set a Matrix3D struct?
                XamlBasic.SetValue(transform, XamlPropertyIndex.MatrixTransform_Matrix, matrix);
                XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_RenderTransform, transform);
            }
            else
            {
                ResetRenderTransform(view);
                var projection = EnsureProjection(view);
                // TODO: can we set a Matrix3D struct?
                XamlBasic.SetValue(projection, XamlPropertyIndex.Matrix3DProjection_ProjectionMatrix, projectionMatrix);
            }
        }

        private static bool IsSimpleTranslationOnly(Matrix3D matrix)
        {
            // Matrix3D is a struct and passed-by-value. As such, we can modify
            // the values in the matrix without affecting the caller.
            matrix.OffsetX = matrix.OffsetY = 0;
            return matrix.IsIdentity;
        }

        private static void ResetProjectionMatrix(IXamlBasicObject view)
        {
            var projection = XamlBasic.GetXamlBasicObjectValue(view, XamlPropertyIndex.UIElement_Projection);

            // TODO: how to check type of IXamlBasicObject?
            if (projection != null && !(XamlBasic.GetDependencyObject(projection) is Matrix3DProjection))
            {
                throw new InvalidOperationException("Unknown projection set on framework element.");
            }

            XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_Projection, default(IXamlBasicObject));
        }

        private static void ResetRenderTransform(IXamlBasicObject view)
        {
            var transform = XamlBasic.GetXamlBasicObjectValue(view, XamlPropertyIndex.UIElement_RenderTransform);

            // TODO: how to check type of IXamlBasicObject?
            if (transform != null && !(XamlBasic.GetDependencyObject(transform) is MatrixTransform))
            {
                throw new InvalidOperationException("Unknown transform set on framework element.");
            }

            XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_RenderTransform, default(IXamlBasicObject));
        }

        private static IXamlBasicObject EnsureProjection(IXamlBasicObject view)
        {
            var projection = XamlBasic.GetXamlBasicObjectValue(view, XamlPropertyIndex.UIElement_Projection);

            // TODO: how to check type of IXamlBasicObject?
            if (projection != null && !(XamlBasic.GetDependencyObject(projection) is Matrix3DProjection))
            {
                throw new InvalidOperationException("Unknown projection set on framework element.");
            }

            if (projection == null)
            {
                projection = XamlBasic.CreateInstance(XamlTypeIndex.Matrix3DProjection);
                XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_Projection, projection);
            }

            return projection;
        }

        private static void SetOverflowHidden(IXamlBasicObject view, Dimensions dimensions)
        {
            if (double.IsNaN(dimensions.Width) || double.IsNaN(dimensions.Height))
            {
                XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_Clip, default(object));
            }
            else
            {
                var rect = new Rect(0, 0, dimensions.Width, dimensions.Height);
                var clip = XamlBasic.CreateInstance(XamlTypeIndex.RectangleGeometry);
                XamlBasic.SetValue(clip, XamlPropertyIndex.RectangleGeometry_Rect, rect);
                XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_Clip, clip);
            }
        }

        private static void SetOverflowVisible(IXamlBasicObject view)
        {
            XamlBasic.SetValue(view, XamlPropertyIndex.UIElement_Clip, default(object));
        }

        class DimensionBoundProperties
        {
            public bool OverflowHidden { get; set; }

            public JArray MatrixTransform { get; set; }
        }
    }
}
