using LagoVista.Core.Validation;
using LagoVista.IoT.Runtime.Core.Models.PEM;
using Newtonsoft.Json;
using System;
using System.Text;

namespace Function
{
    public class FunctionHandler
    {
        public string Handle(string json) {
            var pem = JsonConvert.DeserializeObject<LagoVista.IoT.Runtime.Core.Models.PEM.PipelineExecutionMessage>(json);

            return  JsonConvert.SerializeObject(InvokeResult<PipelineExecutionMessage>.Create(pem));
        }
    }
}
