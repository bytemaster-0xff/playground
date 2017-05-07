using Microsoft.AspNetCore.Mvc;
using SensorService.Models;
using SensorService.Repos;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;

namespace SensorService.Controllers
{
    public class TempService : Controller
    {

        CachedObservation _cachedObservation;

        public TempService(CachedObservation cachedObservation)
        {
            _cachedObservation = cachedObservation;
        }

        [HttpPost("/api/temperatures/F639AC1B-965F-4184-8E3C-D9BD9456949D/{deviceid}")]
        public async Task PostTemperature(String deviceid,[FromBody] Observation reading)
        {
            if (_cachedObservation.Current == null || (DateTime.Now - _cachedObservation.CacheDateStamp).Minutes > 10)
            {
                var client = new HttpClient();
                var json = await client.GetStringAsync("http://api.wunderground.com/api/238e058c81443466/conditions/q/FL/Palm_Harbor.json");
                var current = Models.CurrentTemperature.Create(json);

                _cachedObservation.Current = current.current_observation;
                _cachedObservation.CacheDateStamp = DateTime.Now;
            }

            var repo = new TemperatureRepo("bytemaster", "vSz9ZXAnd1RqavJppFHUlv6O0fMt+3jJfSPdXyD6BAxBX8WPMmhjANewbtZCkZBNIjYDlAZTvGpg9xzaOckymA==", new Loggers.Logger());

            await repo.InsertAsync(new Models.TemperatureReading(deviceid)
            {
                Temperature = reading.temp_f,
                Humidity = Convert.ToDouble(reading.relative_humidity.Replace("%", "")),
                OutsideHumidity = Convert.ToDouble(_cachedObservation.Current.relative_humidity.Replace("%", "")),
                OutsideTemperature = _cachedObservation.Current.temp_f
            });
        }
    }

    public class CachedObservation
    {
        public DateTime CacheDateStamp { get; set; }

        public Models.Observation Current { get; set; }

    }
}
