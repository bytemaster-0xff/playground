using LagoVista.Core.Validation;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;

namespace TestConsole
{
    class Program
    {

        private static string getStdin()
        {
            if(Debugger.IsAttached)
            {
                return "{foo";
            }

            StringBuilder buffer = new StringBuilder();
            string s;
            while ((s = Console.ReadLine()) != null)
            {
                buffer.AppendLine(s);
            }
            return buffer.ToString();
        }

        static async Task Main(string[] args)
        {
            var func = new Function.FunctionHandler();
            var result = await func.Handle(getStdin());
            if (result != null)
            {
                Console.Write(result);
            }
            else
            {

            }

            Console.WriteLine(JsonConvert.SerializeObject(InvokeResult.FromError("Null response."), 
                new JsonSerializerSettings { ContractResolver = new CamelCasePropertyNamesContractResolver() }));

            if(Debugger.IsAttached)
            {
                Console.ReadKey();
            }
        }
    }
}
