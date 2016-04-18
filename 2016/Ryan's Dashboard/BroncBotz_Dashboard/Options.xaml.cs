using BroncBotz_Dashboard.Properties;
using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.Net;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace BroncBotz_Dashboard
{
    /// <summary>
    /// Interaction logic for Options.xaml
    /// </summary>
    public partial class Options : Window
    {
        private static bool allowNetwork = false;
        private static int _index_;
        private MainWindow parent;
        private static string _source_;

        public Options(MainWindow parent)
        {
            InitializeComponent();
            this.parent = parent;
            browse.Click += Browse_Click;
            captureCombo.Items.Add("USB");
            captureCombo.Items.Add("Stream");
            captureCombo.Items.Add("Local File");
            captureCombo.Items.Add("Test Image");

            captureCombo.SelectionChanged += CaptureCombo_SelectionChanged;
            captureCombo.SelectedIndex = Settings.Default.CaptureType;
            source.Text = Settings.Default.CaptureSource;
            saveButton.Click += SaveButton_Click;
        }

        private void Browse_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
                source.Text = openFileDialog.FileName;
        }

        public void NetworkOnStartup(string source)
        {
            _index_ = 1;
            _source_ = source;
            BackgroundWorker bw = new BackgroundWorker();
            bw.WorkerSupportsCancellation = true;
            bw.DoWork += Bw_DoWork;
            bw.RunWorkerCompleted += Bw_RunWorkerCompleted;
            bw.RunWorkerAsync();
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            _index_ = captureCombo.SelectedIndex;
            _source_ = source.Text;
            BackgroundWorker bw = new BackgroundWorker();
            bw.WorkerSupportsCancellation = true;
            bw.DoWork += Bw_DoWork;
            bw.RunWorkerCompleted += Bw_RunWorkerCompleted;
            bw.RunWorkerAsync();
            Close();
        }

        private void Bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (_index_ == 3)
            {
                parent.Dispatcher.BeginInvoke((Action)delegate () { parent.UpdateCapture(CaptureType.Test, ""); });
                Settings.Default.CaptureType = _index_;
                Settings.Default.CaptureSource = "";
                Close();
                return;
            }
            else if (_index_ != 1)
            {
                parent.Dispatcher.BeginInvoke((Action)delegate () { parent.UpdateCapture((CaptureType)_index_, _source_); });
                Settings.Default.CaptureType = _index_;
                Settings.Default.CaptureSource = _source_;
            }

            if (_index_ == 0)
                Settings.Default.prevUSB = _source_;
            else if (_index_ == 1)
                Settings.Default.prevNetwork = _source_;
            else if (_index_ == 2)
                Settings.Default.prevFile = _source_;

            if (_index_ == 1 && allowNetwork == true)
            {
                parent.UpdateCapture((CaptureType)_index_, _source_);
            }

            if (_index_ == 1 && allowNetwork == false)
            {
                parent.UpdateCapture(CaptureType.Test, null);
            }
        }

        private void Bw_DoWork(object sender, DoWorkEventArgs e)
        {
            if (_index_ == 1)
            {
                var match = Regex.Match(_source_, @"\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b");
                ConsoleManager.Instance.AppendInfo("Attempting to contact http://" + match.Captures[0]);

                if (match.Success)
                {
                A:
                    HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://" + match.Captures[0]);
                    request.Timeout = 1500;
                    request.Method = "HEAD";
                    try
                    {
                        HttpWebResponse response = request.GetResponse() as HttpWebResponse;
                        response.Close();

                        if (response.StatusCode == HttpStatusCode.OK)
                        {
                            ConsoleManager.Instance.AppendInfo("Connection to http://" + match.Captures[0] + " secured.");
                            allowNetwork = true;
                            Settings.Default.CaptureType = _index_;
                            Settings.Default.CaptureSource = _source_;
                        }
                    }
                    catch (WebException ex)
                    {
                        allowNetwork = false;
                        ConsoleManager.Instance.AppendError(ex);
                        ConsoleManager.Instance.AppendError("There was an error connecting to the server, trying again...");
                        goto A;
                    }
                }
            }
        }

        #region UI_Calls

        private void CaptureCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (captureCombo.SelectedIndex == 0)
            {
                source.Text = Settings.Default.prevUSB;
                source.IsEnabled = true;
                browse.IsEnabled = false;
            }
            else if (captureCombo.SelectedIndex == 1)
            {
                source.Text = Settings.Default.prevNetwork;
                source.IsEnabled = true;
                browse.IsEnabled = false;
            }
            else if (captureCombo.SelectedIndex == 2)
            {
                source.Text = Settings.Default.prevFile;
                source.IsEnabled = true;
                browse.IsEnabled = true;
            }
            else
            {
                source.Text = "";
                source.IsEnabled = false;
                browse.IsEnabled = false;
            }
        }

        #endregion UI_Calls

        public string GetSource()
        {
            return source.Text;
        }

        #region WindowFunctions

        private void PART_TITLEBAR_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void PART_CLOSE_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion WindowFunctions
    }
}