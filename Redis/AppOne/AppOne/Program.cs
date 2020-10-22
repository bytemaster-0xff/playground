using StackExchange.Redis;
using System;
using System.Threading.Tasks;

namespace AppOne
{
    class Program
    {
        static async Task Main(string[] args)
        {
            var pwd = "[PASSWORD]";

            var redis = ConnectionMultiplexer.Connect($"redis-dev.iothost.net,password={pwd}");
            var db = redis.GetDatabase();
            
            await db.StringSetAsync("FirstKey", "Value");

            db.Set

            Console.WriteLine(await db.StringGetAsync("FirstKey"));

            Console.ReadKey();
        }
    }
}
