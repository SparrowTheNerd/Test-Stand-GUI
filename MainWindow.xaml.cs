﻿using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO.Ports;
using System.Windows.Threading;
using System.ComponentModel;
using System.Threading;
using System.Threading.Tasks;
using InteractiveDataDisplay.WPF;
using System.Reflection.Metadata;


namespace Test_Station_Mk1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        SerialPort sp = new SerialPort();
        public MainWindow()
        {
            InitializeComponent();
            sp.DataReceived += DataReceived;
        }

        private void DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var sp = (SerialPort)sender;
            string data = sp.ReadLine();    //read new serial data until newline character. SERIAL DATA MUST INCLUDE NEWLINE
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action<string>)DataPlot, data);
        }

        Queue<string> displayTank = new Queue<string>();
        Queue<string> displayComb = new Queue<string>();
        Queue<string> displayForce = new Queue<string>();
        Queue<string> displayTime = new Queue<string>();
        int maxQueueLen = 300;
        

        private void DataPlot(string data)
        {
            data.Replace("\n", "");
            string[] content = data.Split(",");     //split the serial string on each comma (individual data) and write to corresponding content fields

            tankPrs.Content = content[0];
            combPrs.Content = content[1];
            loadCel.Content = content[2];

            displayTank.Enqueue(content[0]);
            displayComb.Enqueue(content[1]);
            displayForce.Enqueue(content[2]);
            displayTime.Enqueue(content[3]);

            if (displayTime.Count == maxQueueLen)       //remove first entry when the FIFO is full
            {
                displayTime.Dequeue();
                displayTank.Dequeue();
                displayComb.Dequeue();
                displayForce.Dequeue();
            }

            var x = Array.ConvertAll(displayTime.ToArray(),double.Parse);   //convert string arrays to double arrays for plotting
            var y = Array.ConvertAll(displayTank.ToArray(),double.Parse);

            linegraph.PlotOriginX = x[0];
            linegraph.PlotWidth = x.Last() - x[0];
            linegraph.Plot(x, y); // x and y are IEnumerable<double>
        }

        private void Connect_Click(object sender, RoutedEventArgs e)    //COM Port connection handling
        {
            sp.Close();
            try
            {
                String portName = comPort.Text;
                sp.PortName = portName;
                sp.BaudRate = 115200;
                sp.DtrEnable = true;
                sp.RtsEnable = true;
                sp.DataBits = 8;
                sp.StopBits = StopBits.One;
                sp.NewLine = "\n";
                sp.Open();
                comStatus.Foreground = Brushes.Green;
                comStatus.Content = "Connected";
            }
            catch (Exception)
            {
                comStatus.Foreground = Brushes.Red;
                comStatus.Content = "Failed to connect";
            }
        }
    }
}