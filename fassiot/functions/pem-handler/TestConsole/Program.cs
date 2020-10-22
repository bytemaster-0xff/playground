using System;
using System.Threading.Tasks;

namespace TestConsole
{
    class Program
    {
        static async Task Main(string[] args)
        {
            var func = new Function.FunctionHandler();
            var result = await func.Handle("{'hello':'teset'}");
            
            
        }
    }
}
