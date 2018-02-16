using System;
using Windows.UI.Xaml;

namespace ReactNative.UIManager
{
    /// <summary>
    /// Class providing child management API for view managers.
    /// </summary>
    /// <typeparam name="TLayoutShadowNode">
    /// The shadow node type used by this manager class.
    /// </typeparam>
    public abstract class XamlBasicViewParentManager<TLayoutShadowNode> : XamlBasicBaseViewManager<TLayoutShadowNode>, IViewParentManager
        where TLayoutShadowNode : LayoutShadowNode
    {
        /// <summary>
        /// The <see cref="Type"/> instance that represents the type of shadow
        /// node that this manager will return from
        /// <see cref="CreateShadowNodeInstance"/>.
        /// 
        /// This method will be used in the bridge initialization phase to
        /// collect properties exposed using the <see cref="Annotations.ReactPropAttribute"/>
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
        public override void UpdateExtraData(IXamlBasicObject root, object extraData)
        {
        }

        /// <summary>
        /// Adds a child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="child">The child view.</param>
        /// <param name="index">The index.</param>
        public abstract void AddView(IXamlBasicObject parent, IXamlBasicObject child, int index);

        /// <summary>
        /// Gets the number of children in the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <returns>The number of children.</returns>
        public abstract int GetChildCount(IXamlBasicObject parent);

        /// <summary>
        /// Gets the child at the given index.
        /// </summary>
        /// <param name="parent">The parent view.</param>
        /// <param name="index">The index.</param>
        /// <returns>The child view.</returns>
        public abstract IXamlBasicObject GetChildAt(IXamlBasicObject parent, int index);

        /// <summary>
        /// Removes the child at the given index.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        /// <param name="index">The index.</param>
        public abstract void RemoveChildAt(IXamlBasicObject parent, int index);

        /// <summary>
        /// Removes all children from the view parent.
        /// </summary>
        /// <param name="parent">The view parent.</param>
        public abstract void RemoveAllChildren(IXamlBasicObject parent);

        #region IViewParentManager

        void IViewParentManager.AddView(object parent, object child, int index)
        {
            var xamlBasicObject = ViewConversion.GetXamlBasicObject(child);
            AddView((IXamlBasicObject)parent, xamlBasicObject, index);
        }

        int IViewParentManager.GetChildCount(object parent)
        {
            return GetChildCount((IXamlBasicObject)parent);
        }

        object IViewParentManager.GetChildAt(object parent, int index)
        {
            return GetChildAt((IXamlBasicObject)parent, index);
        }

        void IViewParentManager.RemoveChildAt(object parent, int index)
        {
            RemoveChildAt((IXamlBasicObject)parent, index);
        }

        void IViewParentManager.RemoveAllChildren(object parent)
        {
            RemoveAllChildren((IXamlBasicObject)parent);
        }

        #endregion
    }

    /// <summary>
    /// Class providing child management API for view managers.
    /// </summary>
    public abstract class XamlBasicViewParentManager : XamlBasicViewParentManager<LayoutShadowNode>
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
