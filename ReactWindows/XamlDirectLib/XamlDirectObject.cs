using System.Collections.Generic;

namespace Microsoft.UI.Xaml.Core.Direct
{
    public class XamlDirectObject
    {
        internal object obj;
        internal XamlDirectObject(object obj)
        {
            this.obj = obj;
        }

        public override int GetHashCode()
        {
            return EqualityComparer<object>.Default.GetHashCode(this.obj);
        }

        public override bool Equals(object obj)
        {
            if (obj is XamlDirectObject xdo)
            {
                return EqualityComparer<object>.Default.Equals(this.obj, xdo.obj);
            }

            return false;
        }
    }
}
