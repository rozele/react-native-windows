using Microsoft.Graphics.Canvas.Text;
using ReactNative.Reflection;
using ReactNative.UIManager;
using ReactNative.UIManager.Annotations;
using System.Collections.Generic;
using System.Linq;
using Windows.UI.Text;
using Windows.UI.Xaml.Documents;
using Windows.UI.Xaml.Media;

namespace ReactNative.Views.Text
{
    /// <summary>
    /// Shadow node for virtual text nodes.
    /// </summary>
    public class ReactSpanShadowNode : ReactInlineShadowNode
    {
        private double? _fontSize;
        private int _letterSpacing;

        private FontStyle? _fontStyle;
        private FontWeight? _fontWeight;

        private string _fontFamily;

        /// <summary>
        /// The text managed by the shadow node.
        /// </summary>
        public override string Text
        {
            get
            {
                return string.Join(" ", Children.Cast<ReactInlineShadowNode>().Select(n => n.Text));
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
        /// Create the <see cref="Span"/> instance for the measurement calculation.
        /// </summary>
        /// <param name="children">The children.</param>
        /// <returns>The instance.</returns>
        public override Inline MakeInline(IList<Inline> children)
        {
            var span = new Span();
            UpdateInline(span);

            foreach (var child in children)
            {
                span.Inlines.Add(child);
            }

            return span;
        }

        /// <summary>
        /// Update the properties on the inline instance.
        /// </summary>
        /// <param name="inline">The instance.</param>
        public override void UpdateInline(Inline inline)
        {
            inline.CharacterSpacing = _letterSpacing;
            inline.FontSize = _fontSize ?? 15;
            inline.FontStyle = _fontStyle ?? FontStyle.Normal;
            inline.FontWeight = _fontWeight ?? FontWeights.Normal;
            inline.FontFamily = _fontFamily != null ? new FontFamily(_fontFamily) : FontFamily.XamlAutoFontFamily;
        }

        /// <summary>
        /// Update the text layout.
        /// </summary>
        /// <param name="textLayout">The text layout.</param>
        /// <param name="start">The start index.</param>
        /// <returns>The last index of the text.</returns>
        public override int UpdateTextLayout(CanvasTextLayout textLayout, int start)
        {
            var current = start;
            for (var i = 0; i < ChildCount; ++i)
            {
                if (i > 0)
                {
                    current++;
                }

                var child = (ReactInlineShadowNode)GetChildAt(i);
                current = child.UpdateTextLayout(textLayout, current);
            }

            var length = current - start;
            textLayout.SetFontSize(start, length, (float)(_fontSize ?? 15));
            textLayout.SetFontStyle(start, length, _fontStyle ?? FontStyle.Normal);
            textLayout.SetFontWeight(start, length, _fontWeight ?? FontWeights.Normal);
            if (_fontFamily != null)
            {
                textLayout.SetFontFamily(start, length, _fontFamily);
            }

            return current;
        }
    }
}
