using RabbitMQ.Client;
using System;
using System.Text;

namespace Device
{
    class Program
    {
        static void Main(string[] args)
        {
            var factory = new ConnectionFactory
            {
                HostName = "rabbitmq.nuviot.com",
                Port = AmqpTcpEndpoint.UseDefaultPort,
                UserName = "fred",
                Password = "Test1234"
                
            };
            using (var connection = factory.CreateConnection())
            {
                using (var channel = connection.CreateModel())
                {
                    channel.QueueDeclare("hello", false, false, false, null);

                    var message = "Hello World!";
                    var body = Encoding.UTF8.GetBytes(message);

                    do
                    {
                        channel.BasicPublish(String.Empty, "hello", null, body);
                        Console.WriteLine(" [x] Sent {0}", message);
                    }
                    while (Console.ReadKey().Key != ConsoleKey.X);
                }
            }
            Console.ReadLine();
        }
    }
}
