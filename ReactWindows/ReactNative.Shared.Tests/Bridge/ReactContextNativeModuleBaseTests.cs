#if WINDOWS_UWP
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
#else
using Microsoft.VisualStudio.TestTools.UnitTesting;
#endif
using ReactNative.Bridge;
using System;

namespace ReactNative.Tests.Bridge
{
    [TestClass]
    public class ReactContextNativeModuleBaseTests
    {
        [TestMethod]
        public void ReactContextNativeModuleBase_ArgumentChecks()
        {
            AssertEx.Throws<ArgumentNullException>(
                () => new TestModule(null),
                ex => Assert.AreEqual("reactContext", ex.ParamName));

            var context = new ReactContext();
            var module = new TestModule(context);
            Assert.AreSame(context, module.Context);
        }

        class TestModule : ReactContextNativeModuleBase
        {
            public TestModule(ReactContext reactContext)
                : base(reactContext)
            {
            }

            public override string Name
            {
                get
                {
                    return "Test";
                }
            }
        }
    }
}