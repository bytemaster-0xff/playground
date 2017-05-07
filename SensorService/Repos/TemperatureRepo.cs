using SensorService.Models;
using LagoVista.Core.PlatformSupport;

namespace SensorService.Repos
{
    public class TemperatureRepo : LagoVista.CloudStorage.Storage.TableStorageBase<TemperatureReading>
    {
        public TemperatureRepo(string accountName, string accountKey, ILogger logger) : base(accountName, accountKey, logger)
        {
        }

        
    }
}
