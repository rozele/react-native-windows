using Facebook.CSSLayout;
using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.Text;
using ReactNative.Reflection;
using ReactNative.UIManager;
using ReactNative.UIManager.Annotations;
using System;
using System.Linq;
using Windows.Foundation;
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using System.Numerics;
using ReactNative.Views.Text;

namespace ReactNative.Views.FastText
{
    /// <summary>
    /// The shadow node implementation for text views.
    /// </summary>
    public class ReactFastTextShadowNode : LayoutShadowNode
    {
        private int _letterSpacing;
        private int _numberOfLines;

        private double? _fontSize;
        private double _lineHeight;

        private FontStyle? _fontStyle;
        private FontWeight? _fontWeight;
        private CanvasHorizontalAlignment _textAlignment = CanvasHorizontalAlignment.Left;

        private string _fontFamily;

        /// <summary>
        /// Instantiates a <see cref="ReactFastTextShadowNode"/>.
        /// </summary>
        public ReactFastTextShadowNode()
        {
            MeasureFunction = MeasureText;
        }

        /// <summary>
        /// Instantiates the <see cref="ReactFastTextShadowNode"/>.
        /// </summary>
        /// <param name="isRoot">
        /// A flag signaling whether or not the node is the root node.
        /// </param>
        public ReactFastTextShadowNode(bool isRoot)
        {
            if (isRoot)
            {
                MeasureFunction = MeasureText;
            }
        }

        /// <summary>
        /// Sets the font size for the node.
        /// </summary>
        /// <param name="fontSize">The font size.</param>
        [ReactProp(ViewProps.FontSize)]
        public void SetFontSize(double? fontSize)
        {
            if (_fontSize != fontSize)
            {
                _fontSize = fontSize;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the font family for the node.
        /// </summary>
        /// <param name="fontFamily">The font family.</param>
        [ReactProp(ViewProps.FontFamily)]
        public void SetFontFamily(string fontFamily)
        {
            if (_fontFamily != fontFamily)
            {
                _fontFamily = fontFamily;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the font weight for the node.
        /// </summary>
        /// <param name="fontWeightString">The font weight string.</param>
        [ReactProp(ViewProps.FontWeight)]
        public void SetFontWeight(string fontWeightString)
        {
            var fontWeight = FontStyleHelpers.ParseFontWeight(fontWeightString);
            if (_fontWeight.HasValue != fontWeight.HasValue ||
                (_fontWeight.HasValue && fontWeight.HasValue &&
                _fontWeight.Value.Weight != fontWeight.Value.Weight))
            {
                _fontWeight = fontWeight;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the font style for the node.
        /// </summary>
        /// <param name="fontStyleString">The font style string.</param>
        [ReactProp(ViewProps.FontStyle)]
        public void SetFontStyle(string fontStyleString)
        {
            var fontStyle = EnumHelpers.ParseNullable<FontStyle>(fontStyleString);
            if (_fontStyle != fontStyle)
            {
                _fontStyle = fontStyle;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the letter spacing for the node.
        /// </summary>
        /// <param name="letterSpacing">The letter spacing.</param>
        [ReactProp(ViewProps.LetterSpacing)]
        public void SetLetterSpacing(int letterSpacing)
        {
            var spacing = 50 * letterSpacing; // TODO: Find exact multiplier (50) to match iOS

            if (_letterSpacing != spacing)
            {
                _letterSpacing = spacing;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the line height.
        /// </summary>
        /// <param name="lineHeight">The line height.</param>
        [ReactProp(ViewProps.LineHeight)]
        public virtual void SetLineHeight(double lineHeight)
        {
            if (_lineHeight != lineHeight)
            {
                _lineHeight = lineHeight;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the maximum number of lines.
        /// </summary>
        /// <param name="numberOfLines">Max number of lines.</param>
        [ReactProp(ViewProps.NumberOfLines)]
        public virtual void SetNumberOfLines(int numberOfLines)
        {
            if (_numberOfLines != numberOfLines)
            {
                _numberOfLines = numberOfLines;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Sets the text alignment.
        /// </summary>
        /// <param name="textAlign">The text alignment string.</param>
        [ReactProp(ViewProps.TextAlign)]
        public void SetTextAlign(string textAlign)
        {
            var textAlignment = textAlign == "auto" || textAlign == null ?
                CanvasHorizontalAlignment.Left :
                EnumHelpers.Parse<CanvasHorizontalAlignment>(textAlign);

            if (_textAlignment != textAlignment)
            {
                _textAlignment = textAlignment;
                MarkUpdated();
            }
        }

        /// <summary>
        /// Called after a layout step at the end of a UI batch from
        /// <see cref="UIManagerModule"/>. May be used to enqueue additional UI
        /// operations for the native view. Will only be called on nodes marked
        /// as updated.
        /// </summary>
        /// <param name="uiViewOperationQueue">
        /// Interface for enqueueing UI operations.
        /// </param>
        public override void OnCollectExtraUpdates(UIViewOperationQueue uiViewOperationQueue)
        {
            base.OnCollectExtraUpdates(uiViewOperationQueue);
            uiViewOperationQueue.EnqueueUpdateExtraData(ReactTag, this);
        }

        /// <summary>
        /// Marks a node as updated.
        /// </summary>
        protected override void MarkUpdated()
        {
            base.MarkUpdated();
            dirty();
        }

        private static MeasureOutput MeasureText(CSSNode node, float width, CSSMeasureMode widthMode, float height, CSSMeasureMode heightMode)
        {
            var reactNode = (ReactFastTextShadowNode)node;
            var text = string.Join(" ", reactNode.Children.Cast<ReactInlineShadowNode>().Select(n => n.Text));
            var normalizedWidth = CSSConstants.IsUndefined(width) ? float.PositiveInfinity : width;
            var normalizedHeight = CSSConstants.IsUndefined(height) ? float.PositiveInfinity : height;
            
            var textLayout = new CanvasTextLayout(new DummyCanvasResourceCreator(), text, new CanvasTextFormat(), normalizedWidth, normalizedHeight);
            //textLayout.LineSpacing = (float)reactNode._lineHeight;
            //textLayout.HorizontalAlignment = reactNode._textAlignment;

            if (reactNode._fontFamily != null)
            {
                textLayout.SetFontFamily(0, text.Length, reactNode._fontFamily);
            }

            textLayout.SetFontSize(0, text.Length, (float)(reactNode._fontSize ?? 15));
            textLayout.SetFontWeight(0, text.Length, reactNode._fontWeight ?? FontWeights.Normal);
            
            var current = 0;
            for (var i = 0; i < reactNode.ChildCount; ++i)
            {
                var child = (ReactInlineShadowNode)reactNode.GetChildAt(i);
                current = child.UpdateTextLayout(textLayout, 0);
            }

            var size = textLayout.LayoutBounds;
            return new MeasureOutput(
                (float)Math.Ceiling(size.Width),
                (float)Math.Ceiling(size.Height));
        }

        /// <summary>
        /// Updates the properties of a <see cref="RichTextBlock"/> view.
        /// </summary>
        /// <param name="textBlock">The view.</param>
        public void UpdateTextBlock(TextBlock textBlock)
        {
            UpdateTextBlockCore(textBlock, false);
        }

        private void UpdateTextBlockCore(TextBlock textBlock, bool measureOnly)
        {
            //textBlock.CharacterSpacing = _letterSpacing;
            //textBlock.LineHeight = _lineHeight;
            //textBlock.MaxLines = _numberOfLines;
            //textBlock.TextAlignment = GetTextAlignment(_textAlignment);
            textBlock.FontFamily = _fontFamily != null ? new FontFamily(_fontFamily) : FontFamily.XamlAutoFontFamily;
            textBlock.FontSize = _fontSize ?? 15;
            textBlock.FontStyle = _fontStyle ?? FontStyle.Normal;
            textBlock.FontWeight = _fontWeight ?? FontWeights.Normal;

            if (!measureOnly)
            {
                textBlock.Padding = new Thickness(
                    this.GetPaddingSpace(CSSSpacingType.Left),
                    this.GetPaddingSpace(CSSSpacingType.Top),
                    0,
                    0);
            }
        }

        private TextAlignment GetTextAlignment(CanvasHorizontalAlignment alignment)
        {
            switch (alignment)
            {
                case CanvasHorizontalAlignment.Left:
                    return TextAlignment.Left;
                case CanvasHorizontalAlignment.Right:
                    return TextAlignment.Right;
                case CanvasHorizontalAlignment.Center:
                    return TextAlignment.Center;
                case CanvasHorizontalAlignment.Justified:
                    return TextAlignment.Justify;
                default:
                    return TextAlignment.DetectFromContent;
            }
        }

        class DummyCanvasResourceCreator : ICanvasResourceCreator
        {
            private static readonly CanvasDevice s_device = CanvasDevice.GetSharedDevice();

            public CanvasDevice Device
            {
                get
                {
                    return s_device;
                }
            }
        }
    }
}
