
using Microsoft.IoT.Lightning.Providers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace Stepper_Motor
{
    public class Uln2003Driver : IDisposable
    {
        private readonly GpioPin[] _gpioPins = new GpioPin[4];


        /* Pin Mapping
         *  OUT1 => MOT1A
         *  OUT2 => MOT1B
         *  OUT3 => MOT2A
         *  OUT4 => MOT2B
         *  
         *  BLUE2=>PURPLE IN4 GPIO5
         *  WHITE=>GRAY IN3 GPIO6
         *  BLUE=>BROWN IN2 GPIO13
         *  GREEN => ORANGE IN1 GPIO26
         *  
         *  GPIO26=> OUT1 => MOT1A
         *  GPIO13=> OUT2 => MOT1B
         *  GPIO6 => OUT3 => MOT2A
         *  GPIO5 => OUT4 => MOT2B
         *  
         *  Motors colors
         *  PURPLE GRAY GREEN BLUE
         */

        private readonly GpioPinValue[][] _waveDriveSequence =
        {
            new[] {GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High}
        };

        private readonly GpioPinValue[][] _fullStepSequence =
        {
            new[] {GpioPinValue.High, GpioPinValue.Low, GpioPinValue.High, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.High, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.Low, GpioPinValue.High},
            new[] {GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High }

        };

        private readonly GpioPinValue[][] _dvdStepSequence =
{
            new[] {GpioPinValue.High, GpioPinValue.Low, GpioPinValue.High,  GpioPinValue.Low},
            new[] {GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low,  GpioPinValue.High},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.Low,  GpioPinValue.High},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.High,  GpioPinValue.Low}

        };


        private readonly GpioPinValue[][] _haveStepSequence =
        {
            new[] {GpioPinValue.High, GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High},
            new[] {GpioPinValue.Low, GpioPinValue.High, GpioPinValue.High, GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High, GpioPinValue.High, GpioPinValue.High, GpioPinValue.Low, GpioPinValue.Low},
            new[] {GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.Low, GpioPinValue.High, GpioPinValue.High, GpioPinValue.High }
        };



        private int _mot1a, _mot1b, _mot2a, _mot2b;

        public Uln2003Driver(int mot1a, int mot1b, int mot2a, int mot2b)
        {
            _mot1a = mot1a;
            _mot1b = mot1b;
            _mot2a = mot2a;
            _mot2b = mot2b;
        }

        public async Task Init()
        {
            if (LightningProvider.IsLightningEnabled)
            {
                GpioController gpioController = (await GpioController.GetControllersAsync(LightningGpioProvider.GetGpioProvider()))[0];

                _gpioPins[0] = gpioController.OpenPin(_mot1a);
                _gpioPins[1] = gpioController.OpenPin(_mot1b);
                _gpioPins[2] = gpioController.OpenPin(_mot2a);
                _gpioPins[3] = gpioController.OpenPin(_mot2b);
            }

            var gpio = GpioController.GetDefault();

            foreach (var gpioPin in _gpioPins)
            {
                gpioPin.Write(GpioPinValue.Low);
                gpioPin.SetDriveMode(GpioPinDriveMode.Output);
            }
        }

        public void Phase(int phase)
        {
            var steps = _dvdStepSequence[phase];
            for(var idx = 0; idx < steps.Length;++idx)
            {
                _gpioPins[idx].Write(steps[idx]);
            }
        }

        public void TurnAsync(int steps, TurnDirection direction, DrivingMethod drivingMethod = DrivingMethod.FullStep)
        {
            Task.Run(() =>
            {
                //var steps = 0;
                GpioPinValue[][] methodSequence = _dvdStepSequence;
//                steps = (int)Math.Ceiling(degree / 0.1767478397486253);
                /*steps = (int)Math.Ceiling(degree / 0.1767478397486253);
                switch (drivingMethod)
                {
                    case DrivingMethod.WaveDrive:
                        methodSequence = _waveDriveSequence;
                        steps = (int)Math.Ceiling(degree / 0.1767478397486253);
                        break;
                    case DrivingMethod.FullStep:
                        methodSequence = _fullStepSequence;
                        steps = (int)Math.Ceiling(degree / 0.1767478397486253);
                        break;
                    case DrivingMethod.HalfStep:
                        methodSequence = _haveStepSequence;
                        steps = (int)Math.Ceiling(degree / 0.0883739198743126);
                        break;
                    default:
                        throw new ArgumentOutOfRangeException(nameof(drivingMethod), drivingMethod, null);
                }*/
                var counter = 0;
                System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();

                while (counter < steps)
                {
                    for (var j = 0; j < methodSequence[0].Length; j++)
                    {
                        Phase(0);
                        sw.Start(); while ((sw.Elapsed).TotalMilliseconds < 10) { } sw.Reset();
                        Phase(1);
                        sw.Start(); while ((sw.Elapsed).TotalMilliseconds < 10) { } sw.Reset();
                        Phase(2);
                        sw.Start(); while ((sw.Elapsed).TotalMilliseconds < 10) { } sw.Reset();
                        Phase(3);
                        sw.Start(); while ((sw.Elapsed).TotalMilliseconds < 10) { } sw.Reset();

                        //                        if (counter % 20 == 0)
                        //                          Debug.WriteLine(sw.Elapsed.TotalMilliseconds);




                        counter++;
                        if (counter == steps)
                            break;
                    }
                }

                Stop();
            });
        }

        public void Stop()
        {
            foreach (var gpioPin in _gpioPins)
            {
                gpioPin.Write(GpioPinValue.Low);
            }
        }

        public void Dispose()
        {
            foreach (var gpioPin in _gpioPins)
            {
                gpioPin.Write(GpioPinValue.Low);
                gpioPin.Dispose();
            }
        }
    }

    public enum DrivingMethod
    {
        WaveDrive,
        FullStep,
        HalfStep
    }

    public enum TurnDirection
    {
        Left,
        Right
    }
}
