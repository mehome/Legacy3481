using BroncBotz_Dashboard.Properties;
using Emgu.CV;
using Emgu.CV.UI;
using NetworkTables;
using NetworkTables.Tables;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Forms.Integration;
using System.Windows.Input;

namespace BroncBotz_Dashboard
{
    public enum CaptureType
    {
        USB,
        Network,
        Local,
        Test = -1
    }

    #region NetworkListeners
    public class NetworkTableLister : IRemoteConnectionListener
    {
        public void Connected(IRemote remote, ConnectionInfo info)
        {
            ConsoleManager.Instance.AppendInfo("Network tables connected at " + info.RemoteName + ". Protocol version: " + info.ProtocolVersion);
        }

        public void Disconnected(IRemote remote, ConnectionInfo info)
        {
            ConsoleManager.Instance.AppendError("Network tables disconnected from " + info.RemoteName + ". Protocol version: " + info.ProtocolVersion);
        }
    }

    public class TableActivityListener : ITableListener
    {
        private MainWindow parent;
        private Dictionary<string, TableControlItem> controlsDictionary = new Dictionary<string, TableControlItem>();

        public TableActivityListener(MainWindow parent)
        {
            this.parent = parent;
        }

        public void ValueChanged(ITable source, string key, object value, NotifyFlags flags)
        {
            if (key == "AUTONS")
                parent.UpdateAutonList(source.GetStringArray(key));

            if (key.Split('_')[0] == "AUTON")
            {
                if (!controlsDictionary.ContainsKey(key))
                {
                    parent.Dispatcher.BeginInvoke((Action)delegate ()
                    {
                        var control = new TableControlItem(key.Substring(5), value);
                        parent.AutonTab.Children.Add(control);
                        controlsDictionary.Add(key, control);
                    });
                }
                else
                    controlsDictionary[key].Update(value);
            }
            else if (key.Split('_')[0] == "TELEOP")
            {
                if (!controlsDictionary.ContainsKey(key))
                {
                    parent.Dispatcher.BeginInvoke((Action)delegate ()
                    {
                        var control = new TableControlItem(key.Substring(7), value);
                        parent.TeleopTab.Children.Add(control);
                        controlsDictionary.Add(key, control);
                    });
                }
                else
                    controlsDictionary[key].Update(value);
            }
        }
    }
    #endregion

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private bool firstRun = true;
        private FeedHandler feedHandler;
        private ImageBox CV_CompositImage = new ImageBox();
        private ImageBox CV_OutputImage = new ImageBox();

        public MainWindow()
        {
            InitializeComponent();

            string[] args = Environment.GetCommandLineArgs();

            //reset to all default parameters
            if (args.Length > 1 && args[1] == "--reset")
                Settings.Default.CaptureType = -1;

            Setup();
        }

        /// <summary>
        /// Setup UI and child components
        /// </summary>
        private void Setup()
        {
            record.IsChecked = false;
            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;
            autonComboBox.SelectedIndex = 0;
            ConsoleManager.Instance.Console = Console;
            VideoWriterManager.Instance.Record = false;// do not record by default

            #region FormsHostSetup

            WindowsFormsHost mainHost = new WindowsFormsHost();
            WindowsFormsHost compositHost = new WindowsFormsHost();
            CV_OutputImage.Dock = DockStyle.Fill;
            CV_CompositImage.Dock = DockStyle.Fill;
            CV_CompositImage.FunctionalMode = ImageBox.FunctionalModeOption.Minimum;
            CV_OutputImage.FunctionalMode = ImageBox.FunctionalModeOption.Minimum;
            CV_OutputImage.SizeMode = PictureBoxSizeMode.StretchImage;
            CV_CompositImage.SizeMode = PictureBoxSizeMode.StretchImage;
            CV_OutputImage.Image = CvInvoke.Imread(@"defaultFeed.jpg", Emgu.CV.CvEnum.LoadImageType.Color);
            CV_CompositImage.Image = CvInvoke.Imread(@"defaultFeed.jpg", Emgu.CV.CvEnum.LoadImageType.Color);

            mainHost.Child = CV_OutputImage;
            compositHost.Child = CV_CompositImage;
            mainHost.HorizontalAlignment = System.Windows.HorizontalAlignment.Stretch;
            mainHost.VerticalAlignment = VerticalAlignment.Stretch;
            compositHost.HorizontalAlignment = System.Windows.HorizontalAlignment.Stretch;
            compositHost.VerticalAlignment = VerticalAlignment.Stretch;
            AutonTab.FlowDirection = System.Windows.FlowDirection.LeftToRight;
            AutonTab.Orientation = System.Windows.Controls.Orientation.Vertical;
            MainTab.Children.Add(mainHost);
            CompositTab.Children.Add(compositHost);

            #endregion

            #region SlideBar setup

            lowerHue.Minimum = 0;
            lowerHue.Maximum = 255;
            lowerSaturation.Minimum = 0;
            lowerSaturation.Maximum = 255;
            lowerBrightness.Minimum = 0;
            lowerBrightness.Maximum = 255;
            upperHue.Minimum = 0;
            upperHue.Maximum = 255;
            upperSaturation.Minimum = 0;
            upperSaturation.Maximum = 255;
            upperBrightness.Minimum = 0;
            upperBrightness.Maximum = 255;
            lowerHue.ValueChanged += LowerHue_ValueChanged;
            lowerSaturation.ValueChanged += LowerSaturation_ValueChanged;
            lowerBrightness.ValueChanged += LowerBrightness_ValueChanged;
            upperHue.ValueChanged += UpperHue_ValueChanged;
            upperSaturation.ValueChanged += UpperSaturation_ValueChanged;
            upperBrightness.ValueChanged += UpperBrightness_ValueChanged;

            upperBrightness.Value = Properties.Settings.Default.upperBrightness;
            upperSaturation.Value = Properties.Settings.Default.upperSaturation;
            upperHue.Value = Properties.Settings.Default.upperHue;
            lowerBrightness.Value = Properties.Settings.Default.lowerBrightness;
            lowerSaturation.Value = Properties.Settings.Default.lowerSaturation;
            lowerHue.Value = Properties.Settings.Default.lowerHue;

            #endregion SlideBar setup

            ConsoleManager.Instance.AppendInfo("System Ready.");
        }

        /// <summary>
        /// Setup network tables
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            TeamOptions.NetworkTablesInit();
            TableWrapper.Instance.Table.AddConnectionListener(new NetworkTableLister(), true);
            TableWrapper.Instance.Table.AddTableListener(new TableActivityListener(this), true);
            UpdateCapture((CaptureType)Settings.Default.CaptureType, Settings.Default.CaptureSource);
        }

        /// <summary>
        /// Returns the string value of a capture type enum
        /// </summary>
        /// <param name="type">Capturtype</param>
        /// <returns></returns>
        public string GetEnumString(CaptureType type)
        {
            switch (type)
            {
                case CaptureType.Local:
                    return "File";

                case CaptureType.Network:
                    return "Network Stream";

                case CaptureType.Test:
                    return "Test Image";

                default:
                    return "USB";
            }
        }

        /// <summary>
        /// Destroys the old FeedHandler and creates a new one.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="source"></param>
        public void UpdateCapture(CaptureType type, string source)
        {
            if (feedHandler != null)
                feedHandler.Dispose();

            ConsoleManager.Instance.AppendInfo("New FeedHandeler created for source at " + GetEnumString(type) + ":" + source);

            try
            {
                if (type == CaptureType.USB)
                    feedHandler = new FeedHandler(new Capture(Convert.ToInt32(source)), CV_OutputImage, CV_CompositImage, this);
                else if (type == CaptureType.Network && firstRun)
                {
                    firstRun = false;
                    new Options(this).NetworkOnStartup(source);
                }
                else if (type != CaptureType.Test)
                    feedHandler = new FeedHandler(new Capture(source), CV_OutputImage, CV_CompositImage, this);
                else
                    feedHandler = new FeedHandler(CV_OutputImage, CV_CompositImage);
            }
            catch (Exception) { }
            TARGETING_ON_Click(null, new RoutedEventArgs());
        }

        #region WindowFunctions

        private void PART_TITLEBAR_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void PART_MAXIMIZE_RESTORE_Click(object sender, RoutedEventArgs e)
        {
            if (this.WindowState == System.Windows.WindowState.Normal)
            {
                this.WindowState = System.Windows.WindowState.Maximized;
            }
            else
            {
                this.WindowState = System.Windows.WindowState.Normal;
            }
        }

        private void PART_CLOSE_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void PART_MINIMIZE_Click(object sender, RoutedEventArgs e)
        {
            this.WindowState = System.Windows.WindowState.Minimized;
        }

        #endregion WindowFunctions

        #region UIEvents

        private void button_Click(object sender, RoutedEventArgs e)
        {
            lowerHue.Value = 149;
            lowerSaturation.Value = 230;
            lowerBrightness.Value = 0;
            upperHue.Value = 255;
            upperSaturation.Value = 255;
            upperBrightness.Value = 218;

            Properties.Settings.Default.upperBrightness = (int)upperBrightness.Value;
            Properties.Settings.Default.upperSaturation = (int)upperSaturation.Value;
            Properties.Settings.Default.upperHue = (int)upperHue.Value;
            Properties.Settings.Default.lowerBrightness = (int)lowerBrightness.Value;
            Properties.Settings.Default.lowerSaturation = (int)lowerSaturation.Value;
            Properties.Settings.Default.lowerHue = (int)lowerHue.Value;

            Properties.Settings.Default.Save();
        }

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Properties.Settings.Default.Save();
            System.Windows.Application.Current.Shutdown();
        }

        private void UpperBrightness_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            upperBrightnessLabel.Content = ((int)upperBrightness.Value).ToString();
            Properties.Settings.Default.upperBrightness = (int)upperBrightness.Value;
        }

        private void UpperSaturation_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            upperSaturationLabel.Content = ((int)upperSaturation.Value).ToString();
            Properties.Settings.Default.upperSaturation = (int)upperSaturation.Value;
        }

        private void UpperHue_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            upperHueLabel.Content = ((int)upperHue.Value).ToString();
            Properties.Settings.Default.upperHue = (int)upperHue.Value;
        }

        private void LowerBrightness_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            lowerBrightnessLabel.Content = ((int)lowerBrightness.Value).ToString();
            Properties.Settings.Default.lowerBrightness = (int)lowerBrightness.Value;
        }

        private void LowerSaturation_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            lowerSaturationLabel.Content = ((int)lowerSaturation.Value).ToString();
            Properties.Settings.Default.lowerSaturation = (int)lowerSaturation.Value;
        }

        private void LowerHue_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            lowerHueLabel.Content = ((int)lowerHue.Value).ToString();
            Properties.Settings.Default.lowerHue = (int)lowerHue.Value;
        }

        private void MenuItem_Options(object sender, RoutedEventArgs e)
        {
            new Options(this).ShowDialog();
        }

        private void TARGETING_ON_Click(object sender, RoutedEventArgs e)
        {
            if (feedHandler != null)
            {
                TargetingStatusLabel.Content = "Targeting is on";
                feedHandler.Targeting = true;
                LEDStatusLabel.Content = "The LED rings are on";
                TableWrapper.Instance.Table.PutBoolean("LED", true);
                CemeraStatusLabel.Content = "The Camera is on";
                TableWrapper.Instance.Table.PutBoolean("CAMERA", true);
            }
        }

        private void TARGETING_OFF_Click(object sender, RoutedEventArgs e)
        {
            if (feedHandler != null)
            {
                TargetingStatusLabel.Content = "Targeting is off";
                feedHandler.Targeting = false;
                LEDStatusLabel.Content = "The LED rings are off";
                TableWrapper.Instance.Table.PutBoolean("LED", false);
            }
        }

        private void LED_ON_Click(object sender, RoutedEventArgs e)
        {
            LEDStatusLabel.Content = "The LED rings are on";
            TableWrapper.Instance.Table.PutBoolean("LED", true);
        }

        private void LED_OFF_Click(object sender, RoutedEventArgs e)
        {
            LEDStatusLabel.Content = "The LED rings are off";
            TableWrapper.Instance.Table.PutBoolean("LED", false);
        }

        private void CAMERA_ON_Click(object sender, RoutedEventArgs e)
        {
            TargetingStatusLabel.Content = "Targeting is on";
            feedHandler.Targeting = true;
            LEDStatusLabel.Content = "The LED rings are on";
            TableWrapper.Instance.Table.PutBoolean("LED", true);
            CemeraStatusLabel.Content = "The Camera is on";
            TableWrapper.Instance.Table.PutBoolean("CAMERA", true);
            UpdateCapture(CaptureType.Network, Settings.Default.prevNetwork);
        }

        private void CAMERA_OFF_Click(object sender, RoutedEventArgs e)
        {
            TargetingStatusLabel.Content = "Targeting is off";
            feedHandler.Targeting = false;
            LEDStatusLabel.Content = "The LED rings are off";
            TableWrapper.Instance.Table.PutBoolean("LED", false);
            CemeraStatusLabel.Content = "The Camera is off";
            TableWrapper.Instance.Table.PutBoolean("CAMERA", false);
            UpdateCapture(CaptureType.Test, null);
        }

        public void UpdateAutonList(string[] autons)
        {
            autonComboBox.Dispatcher.BeginInvoke((Action)delegate ()
            {
                autonComboBox.Items.Clear();
                foreach (string str in autons)
                    autonComboBox.Items.Add(str);
            });
        }

        public void AddAutonChild(TableControlItem control)
        {
            AutonTab.Dispatcher.BeginInvoke((Action)delegate ()
            {
                AutonTab.Children.Add(control);
                AutonTab.InvalidateArrange();
            });
        }

        public void AddTeleopChild(TableControlItem control)
        {
            TeleopTab.Dispatcher.BeginInvoke((Action)delegate ()
            {
                TeleopTab.Children.Add(control);
                TeleopTab.InvalidateArrange();
            });
        }

        public void UpdateOffsetLabel(int offset, System.Windows.Media.Brush brush, bool isNAN = false)
        {
            offsetLabel.Dispatcher.BeginInvoke((Action)delegate ()
            {
                if (isNAN)
                {
                    offsetLabel.Content = "N/A";
                    offsetLabel.Background = System.Windows.Media.Brushes.DimGray;
                    offsetLabel.IsEnabled = false;
                    return;
                }
                offsetLabel.Content = offset;
                offsetLabel.IsEnabled = true;
                offsetLabel.Background = brush;
            });
        }

        public void UpdateRaduisLabel(int offset, System.Windows.Media.Brush brush, bool isNAN = false)
        {
            radiusLabel.Dispatcher.BeginInvoke((Action)delegate ()
            {
                if (isNAN)
                {
                    radiusLabel.Content = "N/A";
                    radiusLabel.Background = System.Windows.Media.Brushes.DimGray;
                    radiusLabel.IsEnabled = false;
                    return;
                }
                radiusLabel.Content = offset;
                radiusLabel.IsEnabled = true;
                radiusLabel.Background = brush;
            });
        }

        private void Team_Options_Click(object sender, RoutedEventArgs e)
        {
            new TeamOptions().ShowDialog();
        }

        private void autonComboBox_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            try
            {
                ConsoleManager.Instance.WriteLine("SELECTED_AUTON: " + autonComboBox.SelectedItem.ToString());
                TableWrapper.Instance.Table.PutString("SELECTED_AUTON", autonComboBox.SelectedItem.ToString());
            }
            catch (Exception) { }
        }

        private void record_Checked(object sender, RoutedEventArgs e)
        {
            VideoWriterManager.Instance.Record = true;
        }

        private void record_Unchecked(object sender, RoutedEventArgs e)
        {
            VideoWriterManager.Instance.Record = false;
        }

        #endregion UIEvents
    }
}