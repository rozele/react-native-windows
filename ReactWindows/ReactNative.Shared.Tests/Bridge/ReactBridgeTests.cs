﻿#if WINDOWS_UWP
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
#else
using Microsoft.VisualStudio.TestTools.UnitTesting;
#endif
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using ReactNative.Bridge;
using ReactNative.Bridge.Queue;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ReactNative.Tests.Bridge
{
    [TestClass]
    public class ReactBridgeTests
    {
        [TestMethod]
        public async Task ReactBridge_Ctor_ArgumentChecks()
        {
            await JavaScriptHelpers.Run((executor, jsQueueThread) =>
            {
                using (var nativeThread = CreateNativeModulesThread())
                {
                    var reactCallback = new MockReactCallback();

                    AssertEx.Throws<ArgumentNullException>(
                        () => new ReactBridge(null, reactCallback, nativeThread),
                        ex => Assert.AreEqual("executor", ex.ParamName));

                    AssertEx.Throws<ArgumentNullException>(
                        () => new ReactBridge(executor, null, nativeThread),
                        ex => Assert.AreEqual("reactCallback", ex.ParamName));

                    AssertEx.Throws<ArgumentNullException>(
                        () => new ReactBridge(executor, reactCallback, null),
                        ex => Assert.AreEqual("nativeModulesQueueThread", ex.ParamName));
                }
            });
        }

        [TestMethod]
        public async Task ReactBridge_Method_ArgumentChecks()
        {
            await JavaScriptHelpers.Run((executor, jsQueueThread) =>
            {
                using (var nativeThread = CreateNativeModulesThread())
                {
                    var reactCallback = new MockReactCallback();
                    var bridge = new ReactBridge(executor, reactCallback, nativeThread);

                    AssertEx.Throws<ArgumentNullException>(
                        () => bridge.SetGlobalVariable(null, null),
                        ex => Assert.AreEqual("propertyName", ex.ParamName));
                }
            });
        }

        [TestMethod]
        public async Task ReactBridge_CallFunction()
        {
            await JavaScriptHelpers.Run(async (executor, jsQueueThread) =>
            {
                using (var nativeThread = CreateNativeModulesThread())
                {
                    var reactCallback = new MockReactCallback();
                    var bridge = new ReactBridge(executor, new MockReactCallback(), nativeThread);
                    var token = await jsQueueThread.CallOnQueue(() =>
                    {
                        bridge.CallFunction("module", "method", new JArray());
                        return executor.GetGlobalVariable("FunctionCalls");
                    });

                    var expected = new JArray
                    {
                        new JArray
                        {
                            "module",
                            "method",
                            new JArray(),
                        },
                    };

                    Assert.AreEqual(expected.ToString(Formatting.None), token.ToString(Formatting.None));
                }
            });
        }

        [TestMethod]
        public async Task ReactBridge_InvokeCallback()
        {
            await JavaScriptHelpers.Run(async (executor, jsQueueThread) =>
            {
                using (var nativeThread = MessageQueueThread.Create(MessageQueueThreadSpec.Create("native", MessageQueueThreadKind.BackgroundAnyThread), ex => { Assert.Fail(); }))
                {
                    var reactCallback = new MockReactCallback();
                    var bridge = new ReactBridge(executor, new MockReactCallback(), nativeThread);
                    var token = await jsQueueThread.CallOnQueue(() =>
                    {
                        bridge.InvokeCallback(1, new JArray());
                        return executor.GetGlobalVariable("CallbackCalls");
                    });

                    var expected = new JArray
                    {
                        new JArray
                        {
                            1,
                            new JArray(),
                        },
                    };

                    Assert.AreEqual(expected.ToString(Formatting.None), token.ToString(Formatting.None));
                }
            });
        }

        [TestMethod]
        public void ReactBridge_ReactCallbacks()
        {
            using (var nativeThread = CreateNativeModulesThread())
            {
                var jsonResponse = JArray.Parse("[[42,17],[16,22],[[],[\"foo\"]]]");

                var executor = new MockJavaScriptExecutor
                {
                    OnCallFunctionReturnFlushedQueue = (module, method, args) =>
                    {
                        Assert.AreEqual(module, "module");
                        Assert.AreEqual(method, "method");
                        return jsonResponse;
                    }
                };

                var callbacks = new List<Tuple<int, int, JArray>>();
                var eventHandler = new AutoResetEvent(false);
                var callback = new MockReactCallback(
                    (moduleId, methodId, args) => callbacks.Add(Tuple.Create(moduleId, methodId, args)),
                    () => eventHandler.Set());

                var bridge = new ReactBridge(executor, callback, nativeThread);
                bridge.CallFunction("module", "method", new JArray());

                Assert.IsTrue(eventHandler.WaitOne());

                Assert.AreEqual(2, callbacks.Count);

                Assert.AreEqual(42, callbacks[0].Item1);
                Assert.AreEqual(16, callbacks[0].Item2);
                Assert.AreEqual(0, callbacks[0].Item3.Count);

                Assert.AreEqual(17, callbacks[1].Item1);
                Assert.AreEqual(22, callbacks[1].Item2);
                Assert.AreEqual(1, callbacks[1].Item3.Count);
                Assert.AreEqual("foo", callbacks[1].Item3[0].Value<string>());
            }
        }

        [TestMethod]
        public void ReactBridge_ValidJavaScriptResponse()
        {
            var responses = new[]
            {
                JToken.Parse("null"),
                JToken.Parse("undefined"),
                JArray.Parse("[[],[],[]]"),
                JArray.Parse("[[1],[1],[[]]]"),
                JArray.Parse("[[1],[1],[[1,2,3]], 42]"),
            };

            var n = responses.Length;
            using (var nativeThread = CreateNativeModulesThread())
            {
                var count = 0;
                var executor = new MockJavaScriptExecutor
                {
                    OnCallFunctionReturnFlushedQueue = (module, method, args) =>
                    {
                        return responses[count++];
                    }
                };

                var bridge = new ReactBridge(executor, new MockReactCallback(), nativeThread);

                for (var i = 0; i < n; ++i)
                {
                    bridge.CallFunction("module", "method", new JArray());
                }

                Assert.AreEqual(n, count);
            }
        }


        [TestMethod]
        public void ReactBridge_InvalidJavaScriptResponse()
        {
            var responses = new[]
            {
                JArray.Parse("[null,[],[]]"),
                JArray.Parse("[[],null,[]]"),
                JArray.Parse("[[],[],null]"),
                JArray.Parse("[[42],[],[]]"),
                JArray.Parse("[[],[42],[]]"),
                JArray.Parse("[[],[],[42]]"),
            };

            var n = responses.Length;
            using (var nativeThread = CreateNativeModulesThread())
            {
                var count = 0;
                var executor = new MockJavaScriptExecutor
                {
                    OnCallFunctionReturnFlushedQueue = (module, method, args) =>
                    {
                        return responses[count++];
                    }
                };

                var bridge = new ReactBridge(executor, new MockReactCallback(), nativeThread);

                for (var i = 0; i < n; ++i)
                {
                    AssertEx.Throws<InvalidOperationException>(() => bridge.CallFunction("module", "method", new JArray()));
                }

                Assert.AreEqual(n, count);
            }
        }

        private static MessageQueueThread CreateNativeModulesThread()
        {
            return CreateNativeModulesThread(ex => Assert.Fail(ex.ToString()));
        }

        private static MessageQueueThread CreateNativeModulesThread(Action<Exception> exceptionHandler)
        {
            return MessageQueueThread.Create(
                MessageQueueThreadSpec.Create("native", MessageQueueThreadKind.BackgroundAnyThread), exceptionHandler);
        }

        class MockReactCallback : IReactCallback
        {
            private readonly Action<int, int, JArray> _invoke;
            private readonly Action _onBatchComplete;

            public MockReactCallback()
                : this(() => { })
            {
            }

            public MockReactCallback(Action<int, int, JArray> invoke)
                : this(invoke, () => { })
            {
            }

            public MockReactCallback(Action onBatchComplete)
                : this((p0, p1, p2) => { }, onBatchComplete)
            {
            }

            public MockReactCallback(Action<int, int, JArray> invoke, Action onBatchComplete)
            {
                _invoke = invoke;
                _onBatchComplete = onBatchComplete;
            }

            public void Invoke(int moduleId, int methodId, JArray parameters)
            {
                _invoke(moduleId, methodId, parameters);
            }

            public void OnBatchComplete()
            {
                _onBatchComplete();
            }
        }
    }
}