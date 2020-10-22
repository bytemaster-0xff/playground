using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking.Connectivity;

namespace LagoVista.Tello.Services
{
    public class TelloUDPClient
    {
        public TelloUDPClient(NetworkAdapter adapter)
        {
            _networkAdapter = adapter;
        }

        private NetworkAdapter _networkAdapter;
        

        public async void StartServer()
        {
            try
            {
                while(true)
                {
                    UdpClient udpClient = new UdpClient(8888);
                    var result = await udpClient.ReceiveAsync();
                    Debug.WriteLine(System.Text.ASCIIEncoding.ASCII.GetString(result.Buffer));
                }

            }
            catch (Exception ex)
            {
                Windows.Networking.Sockets.SocketErrorStatus webErrorStatus = Windows.Networking.Sockets.SocketError.GetStatus(ex.GetBaseException().HResult);
                Debug.WriteLine("Open listener error: " + ex.Message);
            }
        }

        private async void StreamSocketListener_ConnectionReceived(Windows.Networking.Sockets.StreamSocketListener sender, Windows.Networking.Sockets.StreamSocketListenerConnectionReceivedEventArgs args)
        {


            string request;
            using (var streamReader = new StreamReader(args.Socket.InputStream.AsStreamForRead()))
            {
                request = await streamReader.ReadLineAsync();
                Debug.WriteLine(request);
            }


            // Echo the request back as the response.
            using (Stream outputStream = args.Socket.OutputStream.AsStreamForWrite())
            {
                using (var streamWriter = new StreamWriter(outputStream))
                {
                    await streamWriter.WriteLineAsync(request);
                    await streamWriter.FlushAsync();
                }
            }

            sender.Dispose();
        }

                
    }
}
