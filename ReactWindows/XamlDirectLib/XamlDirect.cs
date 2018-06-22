using System;

using Windows.Foundation.Metadata;

using WUXCD = Windows.UI.Xaml.Core.Direct;

namespace Microsoft.UI.Xaml.Core.Direct
{
    public static partial class XamlDirect
    {
        #region Statics

        private static IXamlDirect DefaultInstance = null;

        public static IXamlDirect GetDefault()
        {
            if (DefaultInstance == null)
            {
                DefaultInstance = ApiInformation.IsTypePresent("Windows.UI.Xaml.Core.Direct.XamlDirect")
                    ? (IXamlDirect)new XamlDirectWrapper()
                    : new XamlDirectCompat();
            }

            return DefaultInstance;
        }

        #endregion

        #region Properties

        public partial interface IXamlDirect
        {
            bool IsXamlDirectEnabled { get; }
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public bool IsXamlDirectEnabled => true;
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public bool IsXamlDirectEnabled => false;
        }

        #endregion

        #region Initializers

        partial class XamlDirectWrapper : IXamlDirect
        {
            private static readonly WUXCD.XamlDirect XD = WUXCD.XamlDirect.GetDefault();
        }

        #endregion

        #region To/From API Objects

        public partial interface IXamlDirect
        {
            object GetObject(XamlDirectObject xamlDirectObject);

            XamlDirectObject GetXamlDirectObject(object @object);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public object GetObject(XamlDirectObject xamlDirectObject)
            {
                if (xamlDirectObject == null)
                {
                    return null;
                }

                return XD.GetObject((WUXCD.IXamlDirectObject)(xamlDirectObject.obj));
            }

            public XamlDirectObject GetXamlDirectObject(object @object)
            {
                if (@object == null)
                {
                    return null;
                }

                return new XamlDirectObject(XD.GetXamlDirectObject(@object));
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public object GetObject(XamlDirectObject xamlDirectObject)
            {
                return xamlDirectObject?.obj;
            }

            public XamlDirectObject GetXamlDirectObject(object @object)
            {
                if (@object == null)
                {
                    return null;
                }

                return new XamlDirectObject(@object);
            }
        }

        #endregion

        #region CreateInstance

        public partial interface IXamlDirect
        {
            XamlDirectObject CreateInstance(XamlTypeIndex typeIndex);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public XamlDirectObject CreateInstance(XamlTypeIndex typeIndex)
            {
                return new XamlDirectObject(XD.CreateInstance((WUXCD.XamlTypeIndex)typeIndex));
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public XamlDirectObject CreateInstance(XamlTypeIndex typeIndex)
            {
                return new XamlDirectObject(CreateInstanceGenerated(typeIndex));
            }
        }

        #endregion

        #region Property Setters

        public partial interface IXamlDirect
        {
            void SetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, object value);

            void SetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, XamlDirectObject value);

            void SetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, DateTime value);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public void SetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, object value)
            {
                XD.SetObjectProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex, value);
            }

            public void SetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, XamlDirectObject value)
            {
                XD.SetXamlDirectObjectProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex, (WUXCD.IXamlDirectObject)(value?.obj));
            }

            public void SetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, DateTime value)
            {
                XD.SetDateTimeProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex, value);
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public void SetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, object value)
            {
                SetPropertyGenerated(xamlDirectObject.obj, propertyIndex, value);
            }

            public void SetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, XamlDirectObject value)
            {
                SetPropertyGenerated(xamlDirectObject.obj, propertyIndex, value?.obj);
            }

            public void SetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex, DateTime value)
            {
                SetPropertyGenerated(xamlDirectObject.obj, propertyIndex, value);
            }
        }

        #endregion

        #region Property Getters

        public partial interface IXamlDirect
        {
            object GetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex);

            XamlDirectObject GetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex);

            DateTime GetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public object GetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                return XD.GetObjectProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex);
            }

            public XamlDirectObject GetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                var propertyValue = XD.GetXamlDirectObjectProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex);
                if (propertyValue == null)
                {
                    return null;
                }

                return new XamlDirectObject(propertyValue);
            }

            public DateTime GetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                return XD.GetDateTimeProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex).DateTime;
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public object GetObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                return GetPropertyGenerated(xamlDirectObject.obj, propertyIndex);
            }

            public XamlDirectObject GetXamlDirectObjectProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                var propertyValue = GetPropertyGenerated(xamlDirectObject.obj, propertyIndex);
                if (propertyValue == null)
                {
                    return null;
                }

                return new XamlDirectObject(propertyValue);
            }

            public DateTime GetDateTimeProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                return (DateTime)GetPropertyGenerated(xamlDirectObject.obj, propertyIndex);
            }
        }

        #endregion

        #region ClearProperty

        public partial interface IXamlDirect
        {
            void ClearProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public void ClearProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                XD.ClearProperty((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlPropertyIndex)propertyIndex);
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public void ClearProperty(XamlDirectObject xamlDirectObject, XamlPropertyIndex propertyIndex)
            {
                ClearPropertyGenerated(xamlDirectObject.obj, propertyIndex);
            }
        }

        #endregion

        #region Collection Management

        public partial interface IXamlDirect
        {
            uint GetCollectionCount(XamlDirectObject xamlDirectObject);

            XamlDirectObject GetXamlDirectObjectFromCollectionAt(XamlDirectObject xamlDirectObject, uint index);

            void AddToCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value);

            void InsertIntoCollectionAt(XamlDirectObject xamlDirectObject, uint index, XamlDirectObject value);

            bool RemoveFromCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value);

            void RemoveFromCollectionAt(XamlDirectObject xamlDirectObject, uint index);

            void ClearCollection(XamlDirectObject xamlDirectObject);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public uint GetCollectionCount(XamlDirectObject xamlDirectObject)
            {
                return XD.GetCollectionCount((WUXCD.IXamlDirectObject)(xamlDirectObject.obj));
            }

            public XamlDirectObject GetXamlDirectObjectFromCollectionAt(XamlDirectObject xamlDirectObject, uint index)
            {
                var collectionValue = XD.GetXamlDirectObjectFromCollectionAt((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), index);
                if (collectionValue == null)
                {
                    return null;
                }

                return new XamlDirectObject(collectionValue);
            }

            public void AddToCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value)
            {
                XD.AddToCollection((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.IXamlDirectObject)(value?.obj));
            }

            public void InsertIntoCollectionAt(XamlDirectObject xamlDirectObject, uint index, XamlDirectObject value)
            {
                XD.InsertIntoCollectionAt((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), index, (WUXCD.IXamlDirectObject)(value?.obj));
            }

            public bool RemoveFromCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value)
            {
                return XD.RemoveFromCollection((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.IXamlDirectObject)(value?.obj));
            }

            public void RemoveFromCollectionAt(XamlDirectObject xamlDirectObject, uint index)
            {
                XD.RemoveFromCollectionAt((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), index);
            }

            public void ClearCollection(XamlDirectObject xamlDirectObject)
            {
                XD.ClearCollection((WUXCD.IXamlDirectObject)(xamlDirectObject.obj));
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public uint GetCollectionCount(XamlDirectObject xamlDirectObject)
            {
                dynamic collection = xamlDirectObject.obj;
                return (uint)collection.Count;
            }

            public XamlDirectObject GetXamlDirectObjectFromCollectionAt(XamlDirectObject xamlDirectObject, uint index)
            {
                dynamic collection = xamlDirectObject.obj;
                var collectionValue = collection[(int)index];
                if (collectionValue == null)
                {
                    return null;
                }

                return new XamlDirectObject(collectionValue);
            }

            public void AddToCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value)
            {
                dynamic collection = xamlDirectObject.obj;
                dynamic val = value?.obj;
                collection.Add(val);
            }

            public void InsertIntoCollectionAt(XamlDirectObject xamlDirectObject, uint index, XamlDirectObject value)
            {
                dynamic collection = xamlDirectObject.obj;
                dynamic val = value?.obj;
                collection.Insert((int)index, val);
            }

            public bool RemoveFromCollection(XamlDirectObject xamlDirectObject, XamlDirectObject value)
            {
                dynamic collection = xamlDirectObject.obj;
                dynamic val = value?.obj;
                return collection.Remove(val);
            }

            public void RemoveFromCollectionAt(XamlDirectObject xamlDirectObject, uint index)
            {
                dynamic collection = xamlDirectObject.obj;
                collection.RemoveAt((int)index);
            }

            public void ClearCollection(XamlDirectObject xamlDirectObject)
            {
                dynamic collection = xamlDirectObject.obj;
                collection.Clear();
            }
        }

        #endregion

        #region Event Management

        public partial interface IXamlDirect
        {
            void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler);

            void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler, bool handledEventsToo);

            void RemoveEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler);
        }

        partial class XamlDirectWrapper : IXamlDirect
        {
            public void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler)
            {
                XD.AddEventHandler((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlEventIndex)eventIndex, handler);
            }

            public void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler, bool handledEventsToo)
            {
                XD.AddEventHandler((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlEventIndex)eventIndex, handler, handledEventsToo);
            }

            public void RemoveEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler)
            {
                XD.RemoveEventHandler((WUXCD.IXamlDirectObject)(xamlDirectObject.obj), (WUXCD.XamlEventIndex)eventIndex, handler);
            }
        }

        partial class XamlDirectCompat : IXamlDirect
        {
            public void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler)
            {
                AddEventHandlerGenerated(xamlDirectObject.obj, eventIndex, handler);
            }

            public void AddEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler, bool handledEventsToo)
            {
                AddEventHandlerGenerated(xamlDirectObject.obj, eventIndex, handler, handledEventsToo);
            }

            public void RemoveEventHandler(XamlDirectObject xamlDirectObject, XamlEventIndex eventIndex, object handler)
            {
                RemoveEventHandlerGenerated(xamlDirectObject.obj, eventIndex, handler);
            }
        }

        #endregion
    }
}
