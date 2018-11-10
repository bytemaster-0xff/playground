using Confluent.Kafka;
using System;

namespace Producer
{
    class Program
    {
        static async void Send()
        {
            Console.WriteLine("Sending...");

            var config = new ProducerConfig { BootstrapServers = "kafka-dev.iothost.net:9092" };

            // A Producer for sending messages with null keys and UTF-8 encoded values.
            using (var p = new Producer<Null, string>(config))
            {
                try
                {
                    var dr = await p.ProduceAsync("test", new Message<Null, string> { Value = "Hello World" + Guid.NewGuid().ToString() });
                    Console.WriteLine($"Delivered '{dr.Value}' to '{dr.TopicPartitionOffset}'");
                }
                catch (KafkaException e)
                {
                    Console.WriteLine($"Delivery failed: {e.Error.Reason}");
                }
            }
        }

        static void Main(string[] args)
        {

            Console.Write("Running - X to Stop");

            bool running = true;
            while(running)
            {
               if( Console.ReadKey().Key == ConsoleKey.X)
                {
                    running = false;
                }
                else
                {
                    Send();
                }
            }
        }
    }
}
