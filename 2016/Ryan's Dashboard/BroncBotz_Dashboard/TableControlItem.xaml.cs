using System;
using System.Windows.Controls;

namespace BroncBotz_Dashboard
{
    /// <summary>
    /// Interaction logic for TableControlItem.xaml
    /// </summary>
    public partial class TableControlItem : UserControl
    {
        public string Key { get; private set; }

        public TableControlItem(string key, object value)
        {
            InitializeComponent();
            Key = key;
            SetKey(key);
            Update(value);
        }

        private void SetKey(string key)
        {
            this.Dispatcher.BeginInvoke((Action)delegate () { this.key.Content = key; });
        }

        public void Update(object value)
        {
            if (value is double)
                this.Dispatcher.BeginInvoke((Action)delegate () { this.value.Content = ((int)((double)value)).ToString(); });
            else
                this.Dispatcher.BeginInvoke((Action)delegate () { this.value.Content = value.ToString(); });
        }
    }
}