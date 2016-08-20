using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace ReactNative.DevSupport
{
    sealed partial class DevSettingsCheckbox : UserControl
    {
        public static DependencyProperty TitleProperty = DependencyProperty.Register("Title", typeof(string), typeof(DevSettingsCheckbox), new PropertyMetadata(null));
        public static DependencyProperty DescriptionProperty = DependencyProperty.Register("Description", typeof(string), typeof(DevSettingsCheckbox), new PropertyMetadata(null));

        public event RoutedEventHandler Checked;

        public DevSettingsCheckbox()
        {
            this.InitializeComponent();
        }

        public string Title
        {
            get;
            set;
        }

        public string Description
        {
            get;
            set;
        }

        private void SettingCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            Checked?.Invoke(sender, e);
        }

        private void Grid_PointerReleased(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {

        }
    }
}
