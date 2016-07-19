﻿using Microsoft.Graphics.Canvas.Text;
using ReactNative.UIManager;
using System.Collections.Generic;
using Windows.UI.Xaml.Documents;

namespace ReactNative.Views.Text
{
    /// <summary>
    /// Shadow node base class for virtual text nodes.
    /// </summary>
    public abstract class ReactInlineShadowNode : ReactShadowNode
    {
        /// <summary>
        /// The text managed by the shadow node.
        /// </summary>
        public abstract string Text { get; }

        /// <summary>
        /// Called after a layout step at the end of a UI batch from
        /// <see cref="UIManagerModule"/>. May be used to enqueue additional UI
        /// operations for the native view. Will only be called on nodes marked
        /// as updated.
        /// </summary>
        /// <param name="uiViewOperationQueue">
        /// Interface for enqueueing UI operations.
        /// </param>
        public sealed override void OnCollectExtraUpdates(UIViewOperationQueue uiViewOperationQueue)
        {
            base.OnCollectExtraUpdates(uiViewOperationQueue);
            uiViewOperationQueue.EnqueueUpdateExtraData(ReactTag, this);
        }

        /// <summary>
        /// Create the <see cref="Inline"/> instance for the measurement calculation.
        /// </summary>
        /// <param name="children">The children.</param>
        /// <returns>The instance.</returns>
        public abstract Inline MakeInline(IList<Inline> children);

        /// <summary>
        /// Update the properties on the inline instance.
        /// </summary>
        /// <param name="inline">The instance.</param>
        public abstract void UpdateInline(Inline inline);

        /// <summary>
        /// Update the text layout.
        /// </summary>
        /// <param name="textLayout">The text layout.</param>
        /// <param name="start">The start index.</param>
        /// <returns>The last index of the text.</returns>
        public abstract int UpdateTextLayout(CanvasTextLayout textLayout, int start);
    }
}
