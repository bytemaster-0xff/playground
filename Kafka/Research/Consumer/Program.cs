using Confluent.Kafka;
using System;

namespace Consumer
{
    class Program
    {
        private static void Consume()
        {

            var conf = new ConsumerConfig
            {
                GroupId = "test-consumer-group",
                BootstrapServers = "kafka-dev.iothost.net:9092",
                 BrokerAddressFamily = BrokerAddressFamilyType.V4,
                  Debug = "consumer",
                   
                // Note: The AutoOffsetReset property determines the start offset in the event:
                // there are not yet any committed offsets for the consumer group for the
                // topic/partitions of interest. By default, offsets are committed
                // automatically, so in this example, consumption will only start from the
                // eariest message in the topic 'my-topic' the first time you run the program.
                AutoOffsetReset = AutoOffsetResetType.Earliest
            };

            using (var c = new Consumer<Ignore, string>(conf))
            {
                c.Subscribe("test");
                Console.WriteLine("Starting Consumption");

                bool consuming = true;
                // The client will automatically recover from non-fatal errors. You typically
                // don't need to take any action unless an error is marked as fatal.
                c.OnError += (_, e) =>
                {
                    Console.WriteLine(e.Reason);
                    consuming = !e.IsFatal;
                };

                while (consuming)
                {
                    try
                    {
                        var cr = c.Consume();
                        Console.WriteLine($"Consumed message '{cr.Value}' at: '{cr.TopicPartitionOffset}'.");
                    }
                    catch (ConsumeException e)
                    {
                        Console.WriteLine($"Error occured: {e.Error.Reason}");
                    }
                }

                // Ensure the consumer leaves the group cleanly and final offsets are committed.
                c.Close();
            }
        }

        static void Main(string[] args)
        {
            Consume();
            Console.WriteLine("Hello World!");
        }
    }
}
