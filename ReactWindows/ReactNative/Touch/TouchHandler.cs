﻿using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using ReactNative.UIManager;
using ReactNative.UIManager.Events;
using System;
using System.Collections.Generic;
using Windows.Foundation;
using Windows.UI.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

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
            _view.RightTapped += OnRightTapped;
            _view.PointerMoved += OnPointerMoved;
            _view.PointerReleased += OnPointerReleased;
            _view.PointerCanceled += OnPointerCanceled;
            _view.PointerCaptureLost += OnPointerCaptureLost;
        }

        public void Dispose()
        {
            _view.PointerPressed -= OnPointerPressed;
            _view.RightTapped -= OnRightTapped;
            _view.PointerMoved -= OnPointerMoved;
            _view.PointerReleased -= OnPointerReleased;
            _view.PointerCanceled -= OnPointerCanceled;
            _view.PointerCaptureLost -= OnPointerCaptureLost;
        }

        private void OnPointerPressed(object sender, PointerRoutedEventArgs e)
        {
            var pointerId = e.Pointer.PointerId;
            if (IndexOfPointerWithId(pointerId) != -1)
            {
                throw new InvalidOperationException("A pointer with this ID already exists.");
            }

            var pointerPoint = e.GetCurrentPoint(_view);
            if (IsRightClick(e))
            {
                // TODO: We could also do right-click stuff here if we want to support topRightClickStart, topRightClickEnd, etc.
                return;
            }

            var reactView = GetReactViewTarget(e);
            if (reactView != null && _view.CapturePointer(e.Pointer))
            {
                var reactTag = reactView.GetReactCompoundView().GetReactTagAtPoint(reactView,
                    e.GetCurrentPoint(reactView).Position);
                var pointer = new ReactPointer();
                pointer.Target = reactTag;
                pointer.PointerId = e.Pointer.PointerId;
                pointer.Identifier = ++_pointerIDs;
                pointer.ReactView = reactView;
                UpdatePointerForEvent(pointer, e);

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

        private void OnRightTapped(object sender, RightTappedRoutedEventArgs e)
        {
            // 
            // TODO: Currently, we are ignoring PointerUpdateKind == RightButtonPressed 
            // to ensure this is the only handler for right click events. We
            // may not want to use this "special" event at all and just use the
            // Pointer[Pressed|Moved|Released|Canceled] events
            //

            var originalSource = e.OriginalSource as DependencyObject;
            var pointInRoot = e.GetPosition(_view);
            var reactView = GetReactViewTargetForRightClick(originalSource, pointInRoot);
            if (reactView != null)
            {
                var pointInView = e.GetPosition(reactView);
                var reactTag = reactView.GetReactCompoundView().GetReactTagAtPoint(reactView, pointInView);
                var pointer = new ReactPointerForRightClick();
                pointer.Target = reactTag;
                pointer.PageX = (float)pointInRoot.X;
                pointer.PageY = (float)pointInRoot.Y;
                pointer.LocationX = (float)pointInView.X;
                pointer.LocationY = (float)pointInView.Y;

                var rightClickEvent = new RightClickEvent(pointer);
                _view.GetReactContext()
                    .GetNativeModule<UIManagerModule>()
                    .EventDispatcher
                    .DispatchEvent(rightClickEvent);
            }
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

        private UIElement GetReactViewTargetForRightClick(DependencyObject originalSource, Point point)
        {
            // If the target is not a child of the root view, then this pointer
            // event does not belong to React.
            if (!RootViewHelper.IsReactSubview(originalSource))
            {
                return null;
            }

            var sources = VisualTreeHelper.FindElementsInHostCoordinates(point, _view);

            // Get the first React view that does not have pointer events set
            // to 'none' or 'box-none', and is not a child of a view with 
            // 'box-only' or 'none' settings for pointer events.

            // TODO: use pooled data structure
            var isBoxOnlyCache = new Dictionary<DependencyObject, bool>();
            foreach (var source in sources)
            {
                if (!source.HasTag())
                {
                    continue;
                }


                var pointerEvents = source.GetPointerEvents();
                if (pointerEvents == PointerEvents.None || pointerEvents == PointerEvents.BoxNone)
                {
                    continue;
                }

                var viewHierarchy = RootViewHelper.GetReactViewHierarchy(source);
                var isBoxOnly = IsBoxOnlyWithCache(viewHierarchy, isBoxOnlyCache);
                if (!isBoxOnly && source.GetRightClickEnabled())
                {
                    return source;
                }
            }

            return null;

        }

        private UIElement GetReactViewTarget(PointerRoutedEventArgs e)
        {
            // If the target is not a child of the root view, then this pointer
            // event does not belong to React.
            if (!RootViewHelper.IsReactSubview(e.OriginalSource as DependencyObject))
            {
                return null;
            }

            var point = e.GetCurrentPoint(_view).Position;
            var sources = VisualTreeHelper.FindElementsInHostCoordinates(point, _view);

            // Get the first React view that does not have pointer events set
            // to 'none' or 'box-none', and is not a child of a view with 
            // 'box-only' or 'none' settings for pointer events.

            // TODO: use pooled data structure
            var isBoxOnlyCache = new Dictionary<DependencyObject, bool>();
            foreach (var source in sources)
            {
                if (!source.HasTag())
                {
                    continue;
                }

                var pointerEvents = source.GetPointerEvents();
                if (pointerEvents == PointerEvents.None || pointerEvents == PointerEvents.BoxNone)
                {
                    continue;
                }

                var viewHierarchy = RootViewHelper.GetReactViewHierarchy(source);
                var isBoxOnly = IsBoxOnlyWithCache(viewHierarchy, isBoxOnlyCache);
                if (!isBoxOnly)
                {
                    return source;
                }
            }

            return null;
        }

        private void UpdatePointerForEvent(ReactPointer pointer, PointerRoutedEventArgs e)
        {
            var viewPoint = e.GetCurrentPoint(_view);
            var positionInRoot = viewPoint.Position;
            var positionInView = e.GetCurrentPoint(pointer.ReactView).Position;

            pointer.PageX = (float)positionInRoot.X;
            pointer.PageY = (float)positionInRoot.Y;
            pointer.LocationX = (float)positionInView.X;
            pointer.LocationY = (float)positionInView.Y;
            pointer.Timestamp = viewPoint.Timestamp / 1000; // Convert microseconds to milliseconds;
        }

        private void DispatchTouchEvent(TouchEventType touchEventType, List<ReactPointer> activePointers, int pointerIndex)
        {
            var touches = new JArray();
            foreach (var pointer in activePointers)
            {
                touches.Add(JObject.FromObject(pointer));
            }

            var changedIndices = new JArray();
            changedIndices.Add(JToken.FromObject(pointerIndex));

            var coalescingKey = activePointers[pointerIndex].PointerId;

            var touchEvent = new TouchEvent(touchEventType, touches, changedIndices, coalescingKey);

            _view.GetReactContext()
                .GetNativeModule<UIManagerModule>()
                .EventDispatcher
                .DispatchEvent(touchEvent);
        }

        private bool IsRightClick(PointerRoutedEventArgs e)
        {
            var pointerPoint = e.GetCurrentPoint(_view);
            return pointerPoint.Properties.PointerUpdateKind == PointerUpdateKind.RightButtonPressed;
        }

        private static bool IsBoxOnlyWithCache(IEnumerable<DependencyObject> hierarchy, IDictionary<DependencyObject, bool> cache)
        {
            var enumerator = hierarchy.GetEnumerator();

            // Skip the first element (only checking ancestors)
            if (!enumerator.MoveNext())
            {
                return false;
            }

            return IsBoxOnlyWithCacheRecursive(enumerator, cache);
        }

        private static bool IsBoxOnlyWithCacheRecursive(IEnumerator<DependencyObject> enumerator, IDictionary<DependencyObject, bool> cache)
        {
            if (!enumerator.MoveNext())
            {
                return false;
            }

            var currentView = enumerator.Current;
            var isBoxOnly = default(bool);
            if (!cache.TryGetValue(currentView, out isBoxOnly))
            {
                var pointerEvents = currentView.GetPointerEvents();

                isBoxOnly = pointerEvents == PointerEvents.BoxOnly 
                    || pointerEvents == PointerEvents.None
                    || IsBoxOnlyWithCacheRecursive(enumerator, cache);

                cache.Add(currentView, isBoxOnly);
            }

            return isBoxOnly;
        }

        class TouchEvent : Event
        {
            private readonly TouchEventType _touchEventType;
            private readonly JArray _touches;
            private readonly JArray _changedIndices;
            private readonly uint _coalescingKey;

            public TouchEvent(TouchEventType touchEventType, JArray touches, JArray changedIndices, uint coalescingKey)
                : base(-1, TimeSpan.FromTicks(Environment.TickCount))
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

        class RightClickEvent : Event
        {
            private readonly ReactPointerForRightClick _pointer;
            public RightClickEvent(ReactPointerForRightClick pointer)
                : base(pointer.Target, TimeSpan.FromTicks(Environment.TickCount))
            {
                _pointer = pointer;
            }

            public override string EventName
            {
                get
                {
                    return "topRightClick";
                }
            }

            public override void Dispatch(RCTEventEmitter eventEmitter)
            {
                eventEmitter.receiveEvent(ViewTag, EventName, JObject.FromObject(_pointer));
            }
        }

        class ReactPointerBase
        {
            [JsonProperty(PropertyName = "target")]
            public int Target { get; set; }

            [JsonProperty(PropertyName = "locationX")]
            public float LocationX { get; set; }

            [JsonProperty(PropertyName = "locationY")]
            public float LocationY { get; set; }

            [JsonProperty(PropertyName = "pageX")]
            public float PageX { get; set; }

            [JsonProperty(PropertyName = "pageY")]
            public float PageY { get; set; }
        }

        class ReactPointer : ReactPointerBase
        {
            [JsonIgnore]
            public uint PointerId { get; set; }

            [JsonIgnore]
            public UIElement ReactView { get; set; }

            [JsonProperty(PropertyName = "identifier")]
            public uint Identifier { get; set; }

            [JsonProperty(PropertyName = "timestamp")]
            public ulong Timestamp { get; set; }
        }

        class ReactPointerForRightClick : ReactPointerBase
        {
        }
    }
}
