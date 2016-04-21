using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using TPTestComp;
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace TPTestApp
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
            TPTest t = new TPTest();
            int ret = t.TestAllCases();
            TestInfo.Text = "First launch, UT return value is:" + ret.ToString();
        }
    }
}
