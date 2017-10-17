using System;
using System.IO;
using Microsoft.AspNetCore.Hosting;
using LagoVista.Net.LetsEncrypt.AcmeServices.Interfaces;
using LagoVista.Net.LetsEncrypt.Models;
using Microsoft.Extensions.Logging;


namespace LagoVisata.Net.LetsEncrypt.Sample
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var settings = new AcmeSettings()
            {
                EmailAddress = "kevinw@software-logistics.com",
                Development = true,
                Diagnostics = true,
                PfxPassword = "Test1234"
            };

            var acmeHost = new WebHostBuilder()
                    .ConfigureLogging((factory) =>
                    {
                        factory.AddConsole();

                    })
                   .ConfigureServices(services => services.AddAcmeCertificateManager(settings))
                   .UseUrls("http://*:5000/.well-known/acme-challenge/")
                   .Configure(app => app.UseAcmeResponse())
                   .UseKestrel()
                   .Build();

            acmeHost.Start();

            var host = new WebHostBuilder()
                .ConfigureLogging((factory) =>
                {
                    factory.AddConsole();

                })
                .UseStartup<Startup>()
                .ConfigureServices(services => services.AddAcmeCertificateManager(settings))
                .UseKestrel(async (options) =>
                {
                    // Request a new certificate with Let's Encrypt and store it for next time
                    var certificateManager = options.ApplicationServices.GetService(typeof(ICertificateManager)) as ICertificateManager;
                    var certificate = await certificateManager.GetCertificate("cdf8ac12.ngrok.io");
                    if (certificate != null)
                    {
                        options.UseHttps(certificate);
                        Console.WriteLine("GOT CERT!");
                    }
                })
                .UseContentRoot(Directory.GetCurrentDirectory())
                .UseUrls("https://849fbb6a.ngrok.io")
                .UseApplicationInsights()
                .Build();

            host.Run();
        }
    }
}
