using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SensorService.Models
{
    public class CurrentTemperature
    {
        public Observation current_observation { get; set; }

        public static CurrentTemperature Create(String json)
        {
            return JsonConvert.DeserializeObject<CurrentTemperature>(json);
        }

    }

    public class Observation
    {
        public double temp_f { get; set; }
        public double temp_c { get; set; }

        public string relative_humidity { get; set; }

    }
}
