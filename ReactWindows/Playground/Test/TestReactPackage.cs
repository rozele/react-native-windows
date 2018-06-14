using System.Collections.Generic;
using Newtonsoft.Json.Linq;
using ReactNative.Bridge;
using ReactNative.Modules.Core;
using ReactNative.UIManager;
using ReactNative.UIManager.Events;
using Windows.UI.Xaml.Controls;

namespace Playground
{
    class TestReactPackage : IReactPackage
    {
        public IReadOnlyList<INativeModule> CreateNativeModules(ReactContext reactContext)
        {
            return new List<INativeModule>
            {
                new TestModule(),
            };            
        }

        public IReadOnlyList<IViewManager> CreateViewManagers(ReactContext reactContext)
        {
            return new List<IViewManager>
            {
                new TestViewManager(),
            };
        }

        class TestModule : NativeModuleBase
        {
            public override string Name => "TestModule";

            public List<Canvas> Views { get; } = new List<Canvas>();

            [ReactMethod]
            public void sendEvent()
            {
                Views.ForEach(v =>
                    v.GetReactContext()
                        .GetNativeModule<UIManagerModule>()
                        .EventDispatcher
                        .DispatchEvent(new TestEvent(v.GetTag())));
            }

            class TestEvent : Event
            {
                public TestEvent(int viewTag)
                    : base(viewTag)
                {
                }

                public override string EventName => "topTest";

                public override void Dispatch(RCTEventEmitter eventEmitter)
                {
                    eventEmitter.receiveEvent(ViewTag, EventName, new JObject());
                }
            }
        }

        class TestViewManager : SimpleViewManager<Canvas>
        {
            public override string Name => "TestView";

            public override JObject CustomDirectEventTypeConstants
            {
                get
                {
                    return new JObject
                    {
                        {
                            "topTest",
                            new JObject
                            {
                                { "registrationName", "onTest" },
                            }
                        },
                    };
                }
            }

            protected override Canvas CreateViewInstance(ThemedReactContext reactContext)
            {
                var view = new Canvas();
                reactContext.GetNativeModule<TestModule>().Views.Add(view);
                return view;
            }

            public override void OnDropViewInstance(ThemedReactContext reactContext, Canvas view)
            {
                base.OnDropViewInstance(reactContext, view);
                reactContext.GetNativeModule<TestModule>().Views.Remove(view);
            }
        }
    }
}
