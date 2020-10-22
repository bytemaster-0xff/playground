using LagoVista.Core;
using LagoVista.Core.Validation;
using LagoVista.IoT.Runtime.Core.Models.PEM;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Threading.Tasks;

namespace Function
{
    public class FunctionHandler
    {
        public async Task<string> Handle(string json)
        {
            try
            {
                var pem = JsonConvert.DeserializeObject<LagoVista.IoT.Runtime.Core.Models.PEM.PipelineExecutionMessage>(json);

                pem.Log.Add(new LagoVista.IoT.Logging.Info()
                {
                    Message = "I have been processed by an external script.",
                    TimeStamp = DateTime.Now.ToJSONString()
                });

                await Task.Delay(500);

                return JsonConvert.SerializeObject(InvokeResult<PipelineExecutionMessage>.Create(pem),
                    new JsonSerializerSettings { ContractResolver = new CamelCasePropertyNamesContractResolver() });
            }
            catch(Exception ex)
            {
                return  JsonConvert.SerializeObject(InvokeResult<PipelineExecutionMessage>.FromError(ex.Message),
                    new JsonSerializerSettings { ContractResolver = new CamelCasePropertyNamesContractResolver() });
            }
        }
    }
}
