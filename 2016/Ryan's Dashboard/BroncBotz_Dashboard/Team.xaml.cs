using BroncBotz_Dashboard.Properties;
using NetworkTables;
using System;
using System.ComponentModel;
using System.Net;
using System.Windows;
using System.Windows.Input;

namespace BroncBotz_Dashboard
{
    /// <summary>
    /// Interaction logic for Options.xaml
    /// </summary>
    public partial class TeamOptions : Window
    {
        private static string team_Number = "";

        public TeamOptions()
        {
            InitializeComponent();

            teamNumber.Text = Settings.Default.teamNumber.ToString();
            saveButton.Click += SaveButton_Click;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            team_Number = teamNumber.Text;
            BackgroundWorker bw = new BackgroundWorker();
            bw.DoWork += Bw_DoWork;
            bw.RunWorkerAsync();

            Settings.Default.teamNumber = Convert.ToInt32(teamNumber.Text);
            Close();
        }

        public static void NetworkTablesInit()
        {
            NetworkTable.SetClientMode();
            NetworkTable.SetIPAddress("10." + Settings.Default.teamNumber.ToString().Substring(0, 2) + "."
                + Settings.Default.teamNumber.ToString().Substring(2, 2) + ".2");
            TableWrapper.Instance.Table = NetworkTable.GetTable("DADHBOARD_2016");
        }

        private void Bw_DoWork(object sender, DoWorkEventArgs e)
        {
            NetworkTable.SetClientMode();

            if (team_Number != "")
            {
                HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://roborio-" + team_Number + "-frc.local");
                request.Timeout = 15000;
                request.Method = "HEAD";
                try
                {
                    using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                        ConsoleManager.Instance.AppendInfo("Connection to http://roborio-" + team_Number + "-frc.local" + " secured.");
                    var entry = Dns.GetHostEntry("roborio-" + team_Number + "-frc.local");
                    NetworkTable.SetIPAddress(entry.AddressList.ToString());
                    TableWrapper.Instance.Table = NetworkTable.GetTable("DADHBOARD_2016");
                }
                catch (WebException ex)
                {
                    ConsoleManager.Instance.AppendError(ex);
                    ConsoleManager.Instance.AppendError("There was an error connecting to the server, trying again...");
                }

                request = (HttpWebRequest)WebRequest.Create("http://10." + team_Number.Substring(0, 2) + "." + team_Number.Substring(2, 2) + ".2");
                try
                {
                    using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                        ConsoleManager.Instance.AppendInfo("Connection to 10." + team_Number.Substring(0, 2) + "." + team_Number.Substring(2, 2) + ".2 secured.");
                    NetworkTable.SetIPAddress("10." + team_Number.Substring(0, 2) + "." + team_Number.Substring(2, 2) + ".2");
                    TableWrapper.Instance.Table = NetworkTable.GetTable("DADHBOARD_2016");
                }
                catch (WebException ex)
                {
                    ConsoleManager.Instance.AppendError(ex);
                    ConsoleManager.Instance.AppendError("Was unble to connect network table.");
                }
            }
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