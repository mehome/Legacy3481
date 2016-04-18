using System;
using System.IO;
using System.Text;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;

namespace BroncBotz_Dashboard
{
    public class ConsoleManager : TextWriter
    {
        private ConsoleManager()
        {
            System.Console.SetOut(this);
        }

        private static ConsoleManager instance;

        public RichTextBox Console { private get; set; }

        public void AppendError(Exception ex)
        {
            Console.Dispatcher.BeginInvoke((Action)delegate ()
            {
                TextRange tr = new TextRange(Console.Document.ContentEnd, Console.Document.ContentEnd);
                tr.Text = ex.ToString() + "\n";
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Red);
                Console.ScrollToEnd();
            });
        }

        public void AppendError(string ex)
        {
            Console.Dispatcher.BeginInvoke((Action)delegate ()
            {
                TextRange tr = new TextRange(Console.Document.ContentEnd, Console.Document.ContentEnd);
                tr.Text = ex + "\n";
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Red);
                Console.ScrollToEnd();
            });
        }

        public void AppendInfo(string info)
        {
            Console.Dispatcher.BeginInvoke((Action)delegate ()
            {
                TextRange tr = new TextRange(Console.Document.ContentEnd, Console.Document.ContentEnd);
                tr.Text = info + "\n";
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.GreenYellow);
                Console.ScrollToEnd();
            });
        }

        public override void Write(char value)
        {
            base.Write(value);

            Console.Dispatcher.BeginInvoke((Action)delegate ()
            {
                TextRange tr = new TextRange(Console.Document.ContentEnd, Console.Document.ContentEnd);
                tr.Text = value.ToString();
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.GreenYellow);
                Console.ScrollToEnd();
            });
        }

        public override Encoding Encoding { get { return Encoding.UTF8; } }

        public static ConsoleManager Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new ConsoleManager();
                }
                return instance;
            }
        }
    }
}