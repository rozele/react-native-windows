using ReactNative.Reflection;
using System;
using System.Diagnostics;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace ReactNative.DevSupport
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    sealed partial class DevSettingsPage : Page
    {
        public static DependencyProperty IsJavaScriptDevModeEnabledProperty = DependencyProperty.Register(
            ReflectionHelpers.InfoOf((DevSettingsPage p) => p.IsJavaScriptDevModeEnabled).Name,
            typeof(bool),
            typeof(DevSettingsPage),
            PropertyMetadata.Create(true)); 

        private DevInternalSettings _settings;

        public DevSettingsPage()
        {
            this.InitializeComponent();
        }

        public bool? IsJavaScriptDevModeEnabled
        {
            get
            {
                return _settings.IsJavaScriptDevModeEnabled;
            }
            set
            {
                _settings.IsJavaScriptDevModeEnabled = value ?? false;
            }
        }

        public bool? IsJavaScriptMinifyEnabled
        {
            get
            {
                return _settings.IsJavaScriptMinifyEnabled;
            }
            set
            {
                _settings.IsJavaScriptMinifyEnabled = value ?? false;
            }
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            _settings = (DevInternalSettings)e.Parameter;
        }

        private void DevModeButton_Click(object sender, RoutedEventArgs e)
        {
            DevModeCheckBox.IsChecked = !DevModeCheckBox.IsChecked;
        }

        private void MinifyButton_Click(object sender, RoutedEventArgs e)
        {
            MinifyCheckBox.IsChecked = !MinifyCheckBox.IsChecked;
        }
    }
}
