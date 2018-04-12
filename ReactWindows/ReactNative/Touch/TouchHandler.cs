// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using ReactNative.UIManager;
using ReactNative.UIManager.Events;
using System;
using System.Collections.Generic;
using Windows.Foundation;
using Windows.Foundation.Metadata;
using Windows.Graphics.Display;
using Windows.UI.Core;
using Windows.UI.Input;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Input;

namespace ReactNative.Touch
{
    class TouchHandler : IDisposable
    {
        private readonly FrameworkElement _view;
        private readonly List<ReactPointer> _pointers;

        private uint _pointerIDs;

        public TouchHandler(FrameworkElement view)
        {
            _view = view;
            _pointers = new List<ReactPointer>();
            _view.PointerPressed += OnPointerPressed;
            _view.PointerMoved += OnPointerMoved;
            _view.PointerReleased += OnPointerReleased;
            _view.PointerCanceled += OnPointerCanceled;
            _view.PointerCaptureLost += OnPointerCaptureLost;
        }

        public void Dispose()
        {
            _view.PointerPressed -= OnPointerPressed;
            _view.PointerMoved -= OnPointerMoved;
            _view.PointerReleased -= OnPointerReleased;
            _view.PointerCanceled -= OnPointerCanceled;
            _view.PointerCaptureLost -= OnPointerCaptureLost;
            _view.PointerEntered -= OnPointerEntered;
            _view.PointerExited -= OnPointerExited;
        }

        public void OnPointerEntered(object sender, PointerRoutedEventArgs e)
        {
            var originalSource = e.OriginalSource as DependencyObject;
            var view = GetReactViewTarget(originalSource);
            // TODO: improve performance by bailing early
            if (view != null && view == originalSource)
            {
                view.GetReactContext()
                    .GetNativeModule<UIManagerModule>()
                    .EventDispatcher
                    .DispatchEvent(
                        new PointerEnterExitEvent(TouchEventType.Entered, view.GetTag()));
            }
        }

        public void OnPointerExited(object sender, PointerRoutedEventArgs e)
        {
            var originalSource = e.OriginalSource as DependencyObject;
            var view = GetReactViewTarget(originalSource);
            // TODO: improve performance by bailing early
            if (view != null && view == originalSource)
            {
                view.GetReactContext()
                    .GetNativeModule<UIManagerModule>()
                    .EventDispatcher
                    .DispatchEvent(
                        new PointerEnterExitEvent(TouchEventType.Exited, view.GetTag()));
            }
        }

        private void OnPointerPressed(object sender, PointerRoutedEventArgs e)
        {
            var pointerId = e.Pointer.PointerId;
            if (IndexOfPointerWithId(pointerId) != -1)
            {
                throw new InvalidOperationException("A pointer with this ID already exists.");
            }

            var originalSource = e.OriginalSource as DependencyObject;
            var rootPoint = e.GetCurrentPoint(_view);
            var reactView = GetReactViewTarget(originalSource) as UIElement;
            if (reactView != null && _view.CapturePointer(e.Pointer))
            {
                var viewPoint = e.GetCurrentPoint(reactView);
                var reactTag = reactView.GetReactCompoundView().GetReactTagAtPoint(reactView, viewPoint.Position);
                var pointer = new ReactPointer
                {
                    Target = reactTag,
                    PointerId = e.Pointer.PointerId,
                    Identifier = ++_pointerIDs,
                    PointerType = e.Pointer.PointerDeviceType.GetPointerDeviceTypeName(),
                    IsLeftButton = viewPoint.Properties.IsLeftButtonPressed,
                    IsRightButton = viewPoint.Properties.IsRightButtonPressed,
                    IsMiddleButton = viewPoint.Properties.IsMiddleButtonPressed,
                    IsHorizontalMouseWheel = viewPoint.Properties.IsHorizontalMouseWheel,
                    IsEraser = viewPoint.Properties.IsEraser,
                    ReactView = reactView,
                };

                UpdatePointerForEvent(pointer, rootPoint, viewPoint);

                var pointerIndex = _pointers.Count;
                _pointers.Add(pointer);
                DispatchTouchEvent(TouchEventType.Start, _pointers, pointerIndex);
            }
        }

        private void OnPointerMoved(object sender, PointerRoutedEventArgs e)
        {
            var pointerIndex = IndexOfPointerWithId(e.Pointer.PointerId);
            if (pointerIndex != -1)
            {
                var pointer = _pointers[pointerIndex];
                UpdatePointerForEvent(pointer, e);
                DispatchTouchEvent(TouchEventType.Move, _pointers, pointerIndex);
            }
        }

        private void OnPointerReleased(object sender, PointerRoutedEventArgs e)
        {
            OnPointerConcluded(TouchEventType.End, e);
        }

        private void OnPointerCanceled(object sender, PointerRoutedEventArgs e)
        {
            OnPointerConcluded(TouchEventType.Cancel, e);
        }

        private void OnPointerCaptureLost(object sender, PointerRoutedEventArgs e)
        {
            OnPointerConcluded(TouchEventType.Cancel, e);
        }

        private void OnPointerConcluded(TouchEventType touchEventType, PointerRoutedEventArgs e)
        {
            var pointerIndex = IndexOfPointerWithId(e.Pointer.PointerId);
            if (pointerIndex != -1)
            {
                var pointer = _pointers[pointerIndex];
                UpdatePointerForEvent(pointer, e);
                DispatchTouchEvent(touchEventType, _pointers, pointerIndex);

                _pointers.RemoveAt(pointerIndex);

                if (_pointers.Count == 0)
                {
                    _pointerIDs = 0;
                }

                _view.ReleasePointerCapture(e.Pointer);
            }
        }

        private int IndexOfPointerWithId(uint pointerId)
        {
            for (var i = 0; i < _pointers.Count; ++i)
            {
                if (_pointers[i].PointerId == pointerId)
                {
                    return i;
                }
            }

            return -1;
        }

        private DependencyObject GetReactViewTarget(DependencyObject originalSource)
        {
            var viewHierarchy = RootViewHelper.GetReactViewHierarchy(originalSource);
            var enumerator = viewHierarchy.GetEnumerator();
            while (enumerator.MoveNext())
            {
                var current = enumerator.Current;
                var pointerEvents = current.GetPointerEvents();
                // Walk the React view hierarchy looking for the first view that
                // does not have `pointerEvents` == [`none`|`box-none`].
                if (pointerEvents != PointerEvents.BoxNone &&
                    pointerEvents != PointerEvents.None)
                {
                    // Continue walking the React view hierarchy looking for
                    // parent views with `pointerEvents` == [`none`|`box-only`].
                    var source = current;
                    while (enumerator.MoveNext())
                    {
                        current = enumerator.Current;
                        pointerEvents = current.GetPointerEvents();
                        // If the parent view has `pointerEvents` == `none`,
                        // continue the search from the next parent.
                        if (pointerEvents == PointerEvents.None)
                        {
                            source = null;
                            break;
                        }
                        // If the parent view has `pointerEvents` == `box-none`,
                        // set the target view to the parent view and continue
                        // walking the React view hierarchy.
                        else if (pointerEvents == PointerEvents.BoxOnly)
                        {
                            source = current;
                        }
                    }

                    if (source != null)
                    {
                        return source;
                    }
                }
                // If `pointerEvents` == `box-none`, we have to check occluded
                // views, as well as compound views such as text.
                else if (pointerEvents == PointerEvents.BoxNone)
                {

                }
            }

            return null;
        }

        private void UpdatePointerForEvent(ReactPointer pointer, PointerRoutedEventArgs e)
        {
            var rootPoint = e.GetCurrentPoint(_view);
            var viewPoint = e.GetCurrentPoint(pointer.ReactView);
            UpdatePointerForEvent(pointer, rootPoint, viewPoint);
        }

        private void UpdatePointerForEvent(ReactPointer pointer, PointerPoint rootPoint, PointerPoint viewPoint)
        {
            var positionInRoot = rootPoint.Position;
            var positionInView = viewPoint.Position;

            pointer.PageX = (float)positionInRoot.X;
            pointer.PageY = (float)positionInRoot.Y;
            pointer.LocationX = (float)positionInView.X;
            pointer.LocationY = (float)positionInView.Y;
            pointer.Timestamp = rootPoint.Timestamp / 1000; // Convert microseconds to milliseconds;
            pointer.Force = rootPoint.Properties.Pressure;
            pointer.IsBarrelButtonPressed = rootPoint.Properties.IsBarrelButtonPressed;

            pointer.ShiftKey = Window.Current.CoreWindow.GetKeyState(Windows.System.VirtualKey.Shift).HasFlag(CoreVirtualKeyStates.Down);
            pointer.AltKey = Window.Current.CoreWindow.GetKeyState(Windows.System.VirtualKey.Menu).HasFlag(CoreVirtualKeyStates.Down);
            pointer.CtrlKey = Window.Current.CoreWindow.GetKeyState(Windows.System.VirtualKey.Control).HasFlag(CoreVirtualKeyStates.Down);
        }

        private void DispatchTouchEvent(TouchEventType touchEventType, List<ReactPointer> activePointers, int pointerIndex)
        {
            var touches = new JArray();
            foreach (var pointer in activePointers)
            {
                touches.Add(JObject.FromObject(pointer));
            }

            var changedIndices = new JArray
            {
                JToken.FromObject(pointerIndex)
            };

            var coalescingKey = activePointers[pointerIndex].PointerId;

            var touchEvent = new TouchEvent(touchEventType, touches, changedIndices, coalescingKey);

            _view.GetReactContext()
                .GetNativeModule<UIManagerModule>()
                .EventDispatcher
                .DispatchEvent(touchEvent);
        }
        
        private static Point AdjustPointForStatusBar(Point point)
        {
            var currentOrientation = DisplayInformation.GetForCurrentView().CurrentOrientation;
            if (currentOrientation != DisplayOrientations.Landscape &&
                currentOrientation != DisplayOrientations.LandscapeFlipped &&
                ApiInformation.IsTypePresent("Windows.UI.ViewManagement.StatusBar"))
            {
                var rect = StatusBar.GetForCurrentView().OccludedRect;
                point.Y += rect.Height;
            }

            return point;
        }

        class TouchEvent : Event
        {
            private readonly TouchEventType _touchEventType;
            private readonly JArray _touches;
            private readonly JArray _changedIndices;
            private readonly uint _coalescingKey;

            public TouchEvent(TouchEventType touchEventType, JArray touches, JArray changedIndices, uint coalescingKey)
                : base(-1)
            {
                _touchEventType = touchEventType;
                _touches = touches;
                _changedIndices = changedIndices;
                _coalescingKey = coalescingKey;
            }

            public override string EventName
            {
                get
                {
                    return _touchEventType.GetJavaScriptEventName();
                }
            }

            public override bool CanCoalesce
            {
                get
                {
                    return _touchEventType == TouchEventType.Move;
                }
            }

            public override short CoalescingKey
            {
                get
                {
                    unchecked
                    {
                        return (short)_coalescingKey;
                    }
                }
            }

            public override void Dispatch(RCTEventEmitter eventEmitter)
            {
                eventEmitter.receiveTouches(EventName, _touches, _changedIndices);
            }
        }

        class PointerEnterExitEvent : Event
        {
            private readonly TouchEventType _touchEventType;

            public PointerEnterExitEvent(TouchEventType touchEventType, int viewTag)
                : base(viewTag)
            {
                _touchEventType = touchEventType;
            }

            public override string EventName
            {
                get
                {
                    return _touchEventType.GetJavaScriptEventName();
                }
            }

            public override bool CanCoalesce
            {
                get
                {
                    return false;
                }
            }

            public override void Dispatch(RCTEventEmitter eventEmitter)
            {
                var eventData = new JObject
                {
                    { "target", ViewTag },
                };

                var enterLeaveEventName = default(string);
                if (_touchEventType == TouchEventType.Entered)
                {
                    enterLeaveEventName = "topMouseEnter";
                }
                else if (_touchEventType == TouchEventType.Exited)
                {
                    enterLeaveEventName = "topMouseLeave";
                }

                if (enterLeaveEventName != null)
                {
                    eventEmitter.receiveEvent(ViewTag, enterLeaveEventName, eventData);
                }

                eventEmitter.receiveEvent(ViewTag, EventName, eventData);
            }
        }

        class ReactPointer
        {
            [JsonProperty(PropertyName = "target")]
            public int Target { get; set; }

            [JsonIgnore]
            public uint PointerId { get; set; }

            [JsonProperty(PropertyName = "identifier")]
            public uint Identifier { get; set; }

            [JsonIgnore]
            public UIElement ReactView { get; set; }

            [JsonProperty(PropertyName = "timestamp")]
            public ulong Timestamp { get; set; }

            [JsonProperty(PropertyName = "locationX")]
            public float LocationX { get; set; }

            [JsonProperty(PropertyName = "locationY")]
            public float LocationY { get; set; }

            [JsonProperty(PropertyName = "pageX")]
            public float PageX { get; set; }

            [JsonProperty(PropertyName = "pageY")]
            public float PageY { get; set; }

            [JsonProperty(PropertyName = "pointerType")]
            public string PointerType { get; set; }

            [JsonProperty(PropertyName = "force")]
            public double Force { get; set; }

            [JsonProperty(PropertyName = "isLeftButton", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsLeftButton { get; set; }

            [JsonProperty(PropertyName = "isRightButton", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsRightButton { get; set; }

            [JsonProperty(PropertyName = "isMiddleButton", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsMiddleButton { get; set; }

            [JsonProperty(PropertyName = "isBarrelButtonPressed", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsBarrelButtonPressed { get; set; }

            [JsonProperty(PropertyName = "isHorizontalScrollWheel", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsHorizontalMouseWheel { get; set; }

            [JsonProperty(PropertyName = "isEraser", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool IsEraser { get; set; }

            [JsonProperty(PropertyName = "shiftKey", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool ShiftKey { get; set; }

            [JsonProperty(PropertyName = "ctrlKey", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool CtrlKey { get; set; }

            [JsonProperty(PropertyName = "altKey", DefaultValueHandling = DefaultValueHandling.Ignore)]
            public bool AltKey { get; set; }
        }
    }
}
