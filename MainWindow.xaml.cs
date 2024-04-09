using System.Text;
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

        Queue<float> displayTank = new Queue<float>();
        Queue<float> displayComb = new Queue<float>();
        Queue<float> displayForce = new Queue<float>();
        Queue<float> displayTime = new Queue<float>();
        int maxQueueLen = 250;
        float[] time, tank, comb, force;
        
        private void DataPlot(string data)
        {
            data.Replace("\n", "");
            string[] dataStrings = data.Split(",");     //split the serial string on each comma (individual data) and write to corresponding content fields

            tankPrs.Content = dataStrings[1];
            combPrs.Content = dataStrings[2];
            loadCel.Content = dataStrings[3];

            float[] content = Array.ConvertAll(dataStrings, float.Parse);

            displayTime.Enqueue(content[0]);
            displayTank.Enqueue(content[1]);
            displayComb.Enqueue(content[2]);
            displayForce.Enqueue(content[3]);

            if (displayTime.Count >= maxQueueLen)       //remove first entry when the FIFO is full
            {
                displayTime.Dequeue();
                displayTank.Dequeue();
                displayComb.Dequeue();
                displayForce.Dequeue();
            }
            time = displayTime.ToArray();   //convert string arrays to double arrays for plotting
            tank = displayTank.ToArray();
            comb = displayComb.ToArray();
            force = displayForce.ToArray();

            tankpres.PlotOriginX = time[0];
            tankpres.PlotWidth = time.Last() - time[0];
            tankpres.Plot(time, tank); // x and y are IEnumerable<double>
            combpres.PlotOriginX = time[0];
            combpres.PlotWidth = time.Last() - time[0];
            combpres.Plot(time, comb); // x and y are IEnumerable<double>
            forceread.PlotOriginX = time[0];
            forceread.PlotWidth = time.Last() - time[0];
            forceread.Plot(time, force); // x and y are IEnumerable<double>

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