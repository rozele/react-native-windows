#if XAMLDIRECT
// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using Microsoft.UI.Xaml.Core.Direct;
using System;
using ReactNative.Accessibility;
using Windows.UI.Xaml;

namespace ReactNative.UIManager
{
    /// <summary>
    /// Class providing child management API for view managers.
    /// </summary>
    /// <typeparam name="TLayoutShadowNode">
    /// The shadow node type used by this manager class.
    /// </typeparam>
    public abstract class XamlDirectViewParentManager<TLayoutShadowNode> : XamlDirectBaseViewManager<TLayoutShadowNode>, IViewParentManager
        where TLayoutShadowNode : LayoutShadowNode
    {
        /// <summary>
        /// The <see cref="Type"/> instance that represents the type of shadow
        /// node that this manager will return from
        /// <see cref="CreateShadowNodeInstance"/>.
        /// 
        /// This method will be used in the bridge initialization phase to
        /// collect props exposed using the <see cref="Annotations.ReactPropAttribute"/>
        /// annotation from the <see cref="ReactShadowNode"/> subclass.
        /// </summary>
        public sealed override Type ShadowNodeType
        {
            get
            {
                return base.ShadowNodeType;
            }
        }

        /// <summary>
        /// Signals whether the view type needs to handle laying out its own
        /// children instead of deferring to the standard CSS layout algorithm.
        /// </summary>
        public virtual bool NeedsCustomLayoutForChildren
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Creates a shadow node instance for the view manager.
        /// </summary>
        /// <returns>The shadow node instance.</returns>
        public abstract override TLayoutShadowNode CreateShadowNodeInstance();

        /// <summary>
        /// Implement this method to receive optional extra data enqueued from
        /// the corresponding instance of <see cref="ReactShadowNode"/> in
        /// <see cref="ReactShadowNode.OnCollectExtraUpdates"/>.
        /// </summary>
        /// <param name="root">The root view.</param>
        /// <param name="extraData">The extra data.</param>
        public override void UpdateExtraData(XamlDirectObject root, object extraData)
        {
        }

        /// <summary>
        /// Adds a child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="child">The child view.</param>
        /// <param name="index">The index.</param>
        public abstract void AddView(XamlDirectObject parent, XamlDirectObject child, int index);

        /// <summary>
        /// Gets the number of children in the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <returns>The number of children.</returns>
        public abstract int GetChildCount(XamlDirectObject parent);

        /// <summary>
        /// Gets the child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="index">The index.</param>
        /// <returns>The child view.</returns>
        public abstract XamlDirectObject GetChildAt(XamlDirectObject parent, int index);

        /// <summary>
        /// Removes the child at the given index.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <param name="index">The index.</param>
        public abstract void RemoveChildAt(XamlDirectObject parent, int index);

        /// <summary>
        /// Removes all children from the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        public abstract void RemoveAllChildren(XamlDirectObject parent);

        #region IViewParentManager

        void IViewParentManager.AddView(object parent, object child, int index)
        {
            var parentXamlDirectObject = (XamlDirectObject)parent;
            var childXamlDirectObject = ViewConversion.GetXamlDirectObject(child);
            AddView(parentXamlDirectObject, childXamlDirectObject, index);
            // TODO: what to do with AccessibilityHelper
            var parentElement = (UIElement)XamlDirect.GetDefault().GetObject(parentXamlDirectObject);
            var childDependencyObject = ViewConversion.GetDependencyObject(child);
            AccessibilityHelper.OnChildAdded(parentElement, childDependencyObject);
        }

        int IViewParentManager.GetChildCount(object parent)
        {
            return GetChildCount((XamlDirectObject)parent);
        }

        object IViewParentManager.GetChildAt(object parent, int index)
        {
            return GetChildAt((XamlDirectObject)parent, index);
        }

        void IViewParentManager.RemoveChildAt(object parent, int index)
        {
            var parentXamlDirectObject = (XamlDirectObject)parent;
            RemoveChildAt(parentXamlDirectObject, index);
            // TODO: what to do with AccessibilityHelper
            var parentElement = (UIElement)XamlDirect.GetDefault().GetObject(parentXamlDirectObject);
            AccessibilityHelper.OnChildRemoved(parentElement);
        }

        void IViewParentManager.RemoveAllChildren(object parent)
        {
            var parentXamlDirectObject = (XamlDirectObject)parent;
            RemoveAllChildren(parentXamlDirectObject);
            // TODO: what to do with AccessibilityHelper
            var parentElement = (UIElement)XamlDirect.GetDefault().GetObject(parentXamlDirectObject);
            AccessibilityHelper.OnChildRemoved(parentElement);
        }

        #endregion
    }

    /// <summary>
    /// Class providing child management API for view managers.
    /// </summary>
    public abstract class XamlDirectViewParentManager : XamlDirectViewParentManager<LayoutShadowNode>
    {
        /// <summary>
        /// Creates a shadow node instance for the view manager.
        /// </summary>
        /// <returns>The shadow node instance.</returns>
        public sealed override LayoutShadowNode CreateShadowNodeInstance()
        {
            return new LayoutShadowNode();
        }
    }
}
#endif
