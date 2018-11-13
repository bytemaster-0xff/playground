using System;
using System.Text;

namespace Function
{
    public class FunctionHandler
    {
        public string Handle(string input) {
            Console.WriteLine("Hi there - your input was: "+ input);
            return "Welcome to NuvIoT -> " + input;
        }
    }
}