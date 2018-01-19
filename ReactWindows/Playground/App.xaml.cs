using Playground.Modules;
using ReactNative;
using ReactNative.Bridge;
using ReactNative.Modules.Core;
using ReactNative.Modules.Launch;
using ReactNative.UIManager;
using System;
using System.Diagnostics;
using System.Threading;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace Playground
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        private readonly ReactNativeHost _host = new MainReactNativeHost();

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            Microsoft.ApplicationInsights.WindowsAppInitializer.InitializeAsync(
                Microsoft.ApplicationInsights.WindowsCollectors.Metadata |
                Microsoft.ApplicationInsights.WindowsCollectors.Session);
            this.InitializeComponent();
            this.Suspending += OnSuspending;
            this.Resuming += OnResuming;
            this.EnteredBackground += OnEnteredBackground;
            this.LeavingBackground += OnLeavingBackground;
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            base.OnLaunched(e);
            OnCreate(e.Arguments);
        }

        /// <summary>
        /// Invoked when the application is activated.
        /// </summary>
        /// <param name="args">The activated event arguments.</param>
        protected override void OnActivated(IActivatedEventArgs args)
        {
            base.OnActivated(args);

            switch (args.Kind)
            {
                case ActivationKind.Protocol:
                case ActivationKind.ProtocolForResults:
                    var protocolArgs = (IProtocolActivatedEventArgs)args;
                    LauncherModule.SetActivatedUrl(protocolArgs.Uri.AbsoluteUri);
                    break;
            }

            if (args.PreviousExecutionState != ApplicationExecutionState.Running &&
                args.PreviousExecutionState != ApplicationExecutionState.Suspended)
            {
                OnCreate(null);
            }
        }

        /// <summary>
        /// Called whenever the app is opened to initia
        /// </summary>
        /// <param name="arguments"></param>
        private void OnCreate(string arguments)
        {
            _host.OnResume(Exit);
            _host.ApplyArguments(arguments);

#if DEBUG
            if (System.Diagnostics.Debugger.IsAttached)
            {
                this.DebugSettings.EnableFrameRateCounter = true;
            }

            SystemNavigationManager.GetForCurrentView().AppViewBackButtonVisibility =
                AppViewBackButtonVisibility.Visible;
#endif

            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();

                rootFrame.NavigationFailed += OnNavigationFailed;

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }

            if (rootFrame.Content == null)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame.Content = new Page
                {
                    Content = _host.OnCreate(),
                };
            }

            // Ensure the current window is active
            Window.Current.Activate();
        }

        protected override async void OnBackgroundActivated(BackgroundActivatedEventArgs args)
        {
            base.OnBackgroundActivated(args);
            SynchronizationContext.SetSynchronizationContext(new CoreDispatcherSynchronizationContext(CoreApplication.MainView.Dispatcher));
            UnhandledException += App_UnhandledException;
            var deferral = args.TaskInstance.GetDeferral();
            var taskId = args.TaskInstance.Task.TaskId.ToString();
            Debug.WriteLine($"[ReactNative] Background task {taskId} started.");
            _host.ReactInstanceManager.OnResume(Exit);
            var reactContext = await _host.ReactInstanceManager.GetOrCreateReactContextAsync(CancellationToken.None);
            var backgroundModule = reactContext.GetNativeModule<BackgroundModule>();
            var deferralId = Guid.NewGuid().ToString();
            reactContext.RunOnNativeModulesQueueThread(() => backgroundModule.RegisterDeferral(deferralId, deferral));
            reactContext.GetJavaScriptModule<AppBackgroundModule>().doWork(taskId, deferralId);
        }

        private void App_UnhandledException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            Debug.WriteLine($"[ReactNative] {e.Message} {e.Exception}");
        }

        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        private void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            Debug.WriteLine($"[ReactNative] OnSuspend");
            _host.OnSuspend();
        }

        /// <summary>
        /// Invoked when application execution is being resumed.
        /// </summary>
        /// <param name="sender">The source of the resume request.</param>
        /// <param name="e">Details about the resume request.</param>
        private void OnResuming(object sender, object e)
        {
            _host.OnResume(Exit);
        }

        /// <summary>
        /// Invoked when application entered the background.
        /// </summary>
        /// <param name="sender">The source of the entered background request.</param>
        /// <param name="e">Details about the entered background request.</param>
        private void OnEnteredBackground(object sender, EnteredBackgroundEventArgs e)
        {
            _host.OnEnteredBackground();
        }

        /// <summary>
        /// Invoked when application leaving the background.
        /// </summary>
        /// <param name="sender">The source of the leaving background request.</param>
        /// <param name="e">Details about the leaving background request.</param>
        private void OnLeavingBackground(object sender, LeavingBackgroundEventArgs e)
        {
            _host.OnLeavingBackground();
        }

        class CoreDispatcherSynchronizationContext : SynchronizationContext
        {
            private readonly CoreDispatcher _coreDispatcher;

            public CoreDispatcherSynchronizationContext(CoreDispatcher coreDispatcher)
            {
                _coreDispatcher = coreDispatcher;
            }

            public override void Post(SendOrPostCallback d, object state)
            {
                _coreDispatcher.RunAsync(CoreDispatcherPriority.Normal, () => d(state));
            }
        }
    }
}
