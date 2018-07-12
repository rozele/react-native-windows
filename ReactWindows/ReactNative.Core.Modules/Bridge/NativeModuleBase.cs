// Copyright (c) Microsoft Corporation. All rights reserved.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using static System.FormattableString;

namespace ReactNative.Core.Bridge
{
    /// <summary>
    /// Base class for React Native modules. Implementations can be linked
    /// to lifecycle events, such as the creation and disposal of the
    /// <see cref="IReactInstance"/> by overriding the appropriate methods.
    /// 
    /// Native methods are exposed to JavaScript with the
    /// <see cref="ReactMethodAttribute"/> annotation. These methods may only
    /// use arguments that can be parsed by <see cref="JToken.ToObject{T}()"/> or
    /// <see cref="ICallback"/>, which maps from a JavaScript function and can
    /// be used only as a last parameter, or in the case of success and error
    /// callback pairs, the last two arguments respectively.
    /// 
    /// All methods annotated with <see cref="ReactMethodAttribute"/> must
    /// return <see cref="void"/>.
    /// 
    /// Please note that it is not allowed to have multiple methods annotated
    /// with <see cref="ReactMethodAttribute"/> that share the same name.
    /// </summary>
    /// <remarks>
    /// Default implementations of <see cref="Initialize"/> and 
    /// <see cref="OnReactInstanceDispose"/> are provided for convenience.
    /// Subclasses need not call these base methods should they choose to
    /// override them.
    /// </remarks>
    public abstract class NativeModuleBase : INativeModule
    {
        private readonly IReadOnlyDictionary<string, INativeMethod> _methods;
        
        /// <summary>
        /// Instantiates a <see cref="NativeModuleBase"/>.
        /// </summary>
        /// <param name="delegateFactory">
        /// Factory responsible for creating delegates for method invocations.
        /// </param>
        /// <param name="actionQueue">
        /// The action queue that native modules should execute on.
        /// </param>
        protected NativeModuleBase()
        {
            _methods = InitializeMethods();
        }

        /// <summary>
        /// Return true if you intend to override some other native module that
        /// was registered, e.g., as part of a different package (such as the
        /// core one). Trying to override without returning true from this 
        /// method is considered an error and will throw an exception during
        /// initialization. By default, all modules return false.
        /// </summary>
        public virtual bool CanOverrideExistingModule
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// The constants exported by this module.
        /// </summary>
        public virtual IReadableMap ModuleConstants
        {
            get
            {
                return null;
            }
        }

        /// <summary>
        /// The methods callabke from JavaScript on this module.
        /// </summary>
        public IReadOnlyDictionary<string, INativeMethod> Methods
        {
            get
            {
                if (_methods == null)
                {
                    throw new InvalidOperationException("Module has not been initialized.");
                }

                return _methods;
            }
        }

        /// <summary>
        /// The name of the module.
        /// </summary>
        /// <remarks>
        /// This will be the name used to <code>require()</code> this module
        /// from JavaScript.
        /// </remarks>
        public abstract string Name
        {
            get;
        }

        /// <summary>
        /// Called after the creation of a <see cref="IReactInstance"/>, in
        /// order to initialize native modules that require the React or
        /// JavaScript modules.
        /// </summary>
        public virtual void Initialize()
        {
        }

        /// <summary>
        /// Called before a <see cref="IReactInstance"/> is disposed.
        /// </summary>
        /// <returns>A task to await the dispose operation.</returns>
        public virtual Task DisposeAsync(CancellationToken token)
        {
            return Task.CompletedTask;
        }

        private IReadOnlyDictionary<string, INativeMethod> InitializeMethods()
        {
            var declaredMethods = GetType().GetTypeInfo().DeclaredMethods;
            var exportedMethods = new List<Tuple<MethodInfo, ReactMethodAttribute>>();
            foreach (var method in declaredMethods)
            {
                var attribute = (ReactMethodAttribute)method.GetCustomAttribute(typeof(ReactMethodAttribute));
                if (attribute != null)
                {
                    exportedMethods.Add(Tuple.Create(method, attribute));
                }
            }

            var methodMap = new Dictionary<string, INativeMethod>(exportedMethods.Count);
            foreach (var methodData in exportedMethods)
            {
                var method = methodData.Item1;
                var attribute = methodData.Item2;
                if (methodMap.TryGetValue(method.Name, out var existingMethod))
                    throw new NotSupportedException(
                        Invariant($"React module '{GetType()}' with name '{Name}' has more than one ReactMethod with the name '{method.Name}'."));

                methodMap.Add(method.Name, new NativeMethod(this, method, attribute));
            }

            return methodMap;
        }

        class NativeMethod : INativeMethod
        {
            private readonly Lazy<Func<IReactInstance, JArray, JToken>> _invokeDelegate;

            public NativeMethod(NativeModuleBase instance, MethodInfo method, ReactMethodAttribute attribute)
            {
                var delegateFactory = instance._delegateFactory;
                delegateFactory.Validate(method, attribute);
                _invokeDelegate = new Lazy<Func<IReactInstance, JArray, JToken>>(() => delegateFactory.Create(instance, method));
                Type = delegateFactory.GetMethodType(method, attribute);
            }

            public string Type
            {
                get;
            }

            public IReadableValue Invoke(IReactInstance reactInstance, IReadableArray jsArguments)
            {
                using (Tracer.Trace(Tracer.TRACE_TAG_REACT_BRIDGE, "callNativeModuleMethod").Start())
                {
                    return _invokeDelegate.Value(reactInstance, jsArguments);
                }
            }
        }
    }
}
