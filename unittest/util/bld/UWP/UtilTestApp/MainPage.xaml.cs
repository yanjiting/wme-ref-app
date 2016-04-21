using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using UtilTestRTComponent;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace UtilTestApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            UtilTestRTC vUtilTest = new UtilTestRTC();
            int ret = vUtilTest.TestAllCases();
            TestInfo.Text = "First launch, UT return value is:" + ret.ToString();
        }
    }
}
