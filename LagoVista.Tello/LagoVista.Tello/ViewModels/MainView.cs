using LagoVista.Core.Commanding;
using LagoVista.Core.ViewModels;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.WiFi;

namespace LagoVista.Tello.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        public WiFiAdapter WiFiAdapter { get; private set; }

        UdpClient udpClient = new UdpClient(8889);

        RelayCommand _takeoffCommand;
        RelayCommand _landCommand;
        RelayCommand _refreshNetworks;

        public MainViewModel()
        {
            _landCommand = new RelayCommand(Land);
            _takeoffCommand = new RelayCommand(Takeoff);
            _refreshNetworks = new RelayCommand(RefreshNetworks);
        }


        public async Task InitializeFirstAdapter()
        {
            var access = await WiFiAdapter.RequestAccessAsync();
            if (access != WiFiAccessStatus.Allowed)
            {
                throw new Exception("WiFiAccessStatus not allowed");
            }
            else
            {
                var wifiAdapterResults = await DeviceInformation.FindAllAsync(WiFiAdapter.GetDeviceSelector());
                if (wifiAdapterResults.Count >= 1)
                {
                    this.WiFiAdapter = await WiFiAdapter.FromIdAsync(wifiAdapterResults[0].Id);
                    

                    this.WiFiAdapter.AvailableNetworksChanged += WiFiAdapter_AvailableNetworksChanged;
                    RefreshNetworks();
                }
                else
                {
                    throw new Exception("WiFi Adapter not found.");
                }
            }
        }

        private void StartListener()
        {
            Task.Run(async () =>
            {
                while (true)
                {
                    UdpClient udpClient = new UdpClient(8888);
                    var result = await udpClient.ReceiveAsync();
                    Debug.WriteLine(System.Text.ASCIIEncoding.ASCII.GetString(result.Buffer));
                }
            });
        }



        public async void Land()
        {
            string request = "land";
            var msg = System.Text.ASCIIEncoding.ASCII.GetBytes(request);
            await udpClient.SendAsync(msg, msg.Length, "192.168.10.1", 8889);

        }

        public async void Takeoff()
        {
            string request = "takeoff";
            var msg = System.Text.ASCIIEncoding.ASCII.GetBytes(request);
            await udpClient.SendAsync(msg, msg.Length, "192.168.10.1", 8889);
        }

        bool _connected = false;

        private async void RefreshNetworks()
        {
            if (_connected)
            {
                await this.WiFiAdapter.ScanAsync();
                foreach (var networks in WiFiAdapter.NetworkReport.AvailableNetworks)
                {
                    if (networks.Ssid.StartsWith("TELLO"))
                    {
                        await this.WiFiAdapter.ConnectAsync(networks, WiFiReconnectionKind.Automatic);
                        Debug.WriteLine("CONNECTED!");
                        _connected = true;
                        DispatcherServices.Invoke(() => IsConnected = true);

                        return;
                    }
                }

                DispatcherServices.Invoke(() => IsConnected = false);
            }
        }

        private void WiFiAdapter_AvailableNetworksChanged(WiFiAdapter sender, object args)
        {
            RefreshNetworks();
        }

        private bool _isConnected;
        public bool IsConnected
        {
            get => _isConnected;
            set => Set(ref _isConnected, value);
        }

        public RelayCommand TakeoffCommand => _takeoffCommand;
        public RelayCommand LandCommand => _landCommand;
        public RelayCommand RefreshNetworksCommand => _refreshNetworks;
    }
}
