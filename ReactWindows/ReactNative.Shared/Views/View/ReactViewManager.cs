using ReactNative.UIManager;
#if WINDOWS_UWP
using Windows.UI.Xaml.Controls;
#else
using System.Windows.Controls;
#endif

namespace ReactNative.Views.View
{
    /// <summary>
    /// View manager for React view instances.
    /// </summary>
    public class ReactViewManager : BorderedCanvasManager<BorderedCanvas>
    {
        /// <summary>
        /// Checks if the Canvas has a Border already.
        /// </summary>
        protected override bool HasBorder(BorderedCanvas view)
        {
            return view.Border != null;
        }

        /// <summary>
        /// Adds a Border to a Canvas if it hasn't been added already.
        /// </summary>
        protected override Border GetOrCreateBorder(BorderedCanvas view)
        {
            if (view.Border == null)
            {
                view.Border = new Border { BorderBrush = s_defaultBorderBrush };
            }

            return view.Border;
        }

        /// <summary>
        /// The name of this view manager. This will be the name used to 
        /// reference this view manager from JavaScript.
        /// </summary>
        public override string Name
        {
            get
            {
                return "RCTView";
            }
        }

        /// <summary>
        /// Creates a new view instance of type <see cref="Canvas"/>.
        /// </summary>
        /// <param name="reactContext">The React context.</param>
        /// <returns>The view instance.</returns>
        protected override BorderedCanvas CreateViewInstance(ThemedReactContext reactContext)
        {
            return new BorderedCanvas();
        }
    }
}
