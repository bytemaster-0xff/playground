using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using System;
using System.Text;

namespace RabitMQTests
{
    class Program
    {
        static void Main(string[] args)
        {
            var factory = new ConnectionFactory
            {
                HostName = "138.197.78.130",
                Port = AmqpTcpEndpoint.UseDefaultPort,
            };
            using (var connection = factory.CreateConnection())
            {
                using (var channel = connection.CreateModel())
                {
                    channel.QueueDeclare("hello", false, false, false, null);

                    var consumer = new EventingBasicConsumer(channel);
                    consumer.Received += (model, ea) =>
                    {
                        var body = ea.Body;
                        var message = Encoding.UTF8.GetString(body);
                        Console.WriteLine(" [x] Received {0}", message);
                    };
                    channel.BasicConsume("hello", true, consumer);

                    Console.WriteLine("Press [enter] to exit.");
                    Console.ReadLine();
                }
            }
        }
    }
}
