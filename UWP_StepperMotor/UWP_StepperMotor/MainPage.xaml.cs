using Stepper_Motor;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace UWP_StepperMotor
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        Uln2003Driver _driver;

        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }

        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            _driver = new Uln2003Driver(26, 13, 6, 5);
            await _driver.Init();
        }

        const double  delay = 0.5;
        /*        private void SpinUp(int loops)
        {
            Task.Run(async () =>
            {
                Debug.WriteLine("MOVE UP");

                for (var idx = 0; idx < loops; ++idx)
                {
                    _driver.Phase(0);
                    await Task.Delay(delay);
                    _driver.Phase(1);
                    await Task.Delay(delay);
                    _driver.Phase(2);
                    await Task.Delay(delay);
                    _driver.Phase(3);
                    await Task.Delay(delay);
                }

                Debug.WriteLine("MOVED UP");

                _driver.Stop();
                
            });
        }

        private void SpinDown(int loops)
        {
            Task.Run(async () =>
            {
                Debug.WriteLine("MOVE DOWN");

                for (var idx = 0; idx < loops; ++idx)
                {
                    _driver.Phase(3);
                    await Task.Delay(delay);
                    _driver.Phase(2);
                    await Task.Delay(delay);
                    _driver.Phase(1);
                    await Task.Delay(delay);
                    _driver.Phase(0);
                    await Task.Delay(delay);
                }

                Debug.WriteLine("MOVED DOWN");

                _driver.Stop();

            });
        }*/


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var btn = sender as Button;
            if (btn.Tag == null)
            {
                //  

                //SpinIt(100);
            }
            else
            {
                switch(btn.Tag.ToString())
                {
                    case "up": _driver.TurnAsync(100, TurnDirection.Left); break;
                    case "down": _driver.TurnAsync(100, TurnDirection.Right); break;

                    case "2": _driver.Phase(1); break;
                    case "3": _driver.Phase(2); break;
                    case "4": _driver.Phase(3); break;
                }
            }
        }
    }
}
