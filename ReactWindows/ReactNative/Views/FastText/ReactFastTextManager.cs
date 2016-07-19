using ReactNative.UIManager;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Documents;

namespace ReactNative.Views.FastText
{
    class ReactFastTextManager : ViewParentManager<TextBlock, ReactFastTextShadowNode>
    {
        public override string Name
        {
            get
            {
                return "RCTText";
            }
        }

        public override void AddView(TextBlock parent, DependencyObject child, int index)
        {
            var inline = child.As<Inline>();
            parent.Inlines.Insert(index, inline);
        }

        public override ReactFastTextShadowNode CreateShadowNodeInstance()
        {
            return new ReactFastTextShadowNode();
        }

        public override DependencyObject GetChildAt(TextBlock parent, int index)
        {
            return parent.Inlines[index];
        }

        public override int GetChildCount(TextBlock parent)
        {
            return parent.Inlines.Count;
        }

        public override void RemoveAllChildren(TextBlock parent)
        {
            parent.Inlines.Clear();
        }

        public override void RemoveChildAt(TextBlock parent, int index)
        {
            parent.Inlines.RemoveAt(index);
        }

        public override void UpdateExtraData(TextBlock root, object extraData)
        {
            base.UpdateExtraData(root, extraData);

            var textNode = extraData as ReactFastTextShadowNode;
            if (textNode != null)
            {
                textNode.UpdateTextBlock(root);
            }
        }

        protected override TextBlock CreateViewInstance(ThemedReactContext reactContext)
        {
            return new TextBlock
            {
                IsTextSelectionEnabled = false,
                TextAlignment = TextAlignment.DetectFromContent,
                TextTrimming = TextTrimming.CharacterEllipsis,
                TextWrapping = TextWrapping.Wrap,
            };
        }
    }
}
