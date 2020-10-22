using LagoVista.Tello.Services;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace LagoVista.Tello
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        TelloUDPClient _client;

        ViewModels.MainViewModel _vm;

        public MainPage()
        {
            this.InitializeComponent();
            _vm = new ViewModels.MainViewModel();
            this.DataContext = _vm;
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            await _vm.InitializeFirstAdapter();
            base.OnNavigatedTo(e);
        }
    }
}
