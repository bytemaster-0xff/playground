using LagoVista.Core.PlatformSupport;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SensorService.Loggers
{
    public class Logger : ILogger
    {
        public void Log(LogLevel level, string area, string message, params KeyValuePair<string, string>[] args)
        {
            
        }

        public void LogException(string area, Exception ex, params KeyValuePair<string, string>[] args)
        {
            
        }

        public void SetKeys(params string[] args)
        {
            
        }

        public void SetUserId(string userId)
        {
            
        }

        public void TrackEvent(string message, Dictionary<string, string> parameters)
        {
            
        }
    }
}
