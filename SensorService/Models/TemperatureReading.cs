using LagoVista.Core.Models;
using LagoVista.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SensorService.Models
{
    public class TemperatureReading : TableStorageEntity
    {
        public TemperatureReading(String deviceId)
        {
            PartitionKey = deviceId;
            DateStamp = DateTime.Now.ToJSONString();
            RowKey = DateStamp + deviceId;


        }

        public double OutsideTemperature { get; set; }
        public double OutsideHumidity { get; set; }

        public double Temperature { get; set; }

        public double Humidity { get; set; }

        public String DateStamp { get; set; }
    }
}
