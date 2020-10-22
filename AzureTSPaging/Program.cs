using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Newtonsoft.Json;
using System;

namespace AzureTSPaging
{
    class Program
    {
        private const string CONNNSTR = "DefaultEndpointsProtocol=https;AccountName=nuviotdev;AccountKey=Za6PpxUbXjXic8rhK3lbcWyUQyVY2NVsgXRRD1rVj2LAjXUnji5/ooJx7u0ob9cPKTkPu/woa74DBE6IVKsLQA==;EndpointSuffix=core.windows.net";

        private static async void QueryTable()
        {
        

            var storageAccount = CloudStorageAccount.Parse(CONNNSTR);
            var tableClient = storageAccount.CreateCloudTableClient();
            var table = tableClient.GetTableReference("UsageMetrics");

            // Initialize the continuation token to null to start from the beginning of the table.
            TableContinuationToken continuationToken = null;

            var tableQuery = new TableQuery<UsageMetrics>().Where(TableQuery.GenerateFilterCondition("PartitionKey", QueryComparisons.Equal, "B23F252329484CB39B5E6F6FA033D497"));
            tableQuery.Take(250);

            //do
            {
                // Retrieve a segment (up to 1,000 entities).
                var tableQueryResult = await table.ExecuteQuerySegmentedAsync(tableQuery, continuationToken);

                // Assign the new continuation token to tell the service where to
                // continue on the next iteration (or null if it has reached the end).
                continuationToken = tableQueryResult.ContinuationToken;

                if (continuationToken != null)
                {
                    Console.WriteLine($"Rows retrieved {tableQueryResult.Results.Count} PK=[{continuationToken.NextPartitionKey}]  RK=[{continuationToken.NextRowKey}] NT=[{continuationToken.NextTableName}] CT=[{continuationToken.TargetLocation.Value}]");
                }
                else
                {
                    Console.WriteLine("Rows retrieved {0}", tableQueryResult.Results.Count);
                }               

                // Loop until a null continuation token is received, indicating the end of the table.
            }// while (continuationToken != null);

        }


        static void Main(string[] args)
        {
            QueryTable();
            Console.ReadKey();
        }
    }

    public class UsageMetrics : Microsoft.WindowsAzure.Storage.Table.TableEntity
    {        
        [JsonProperty("startTimeStamp")]
        public String StartTimeStamp { get; set; }
        [JsonProperty("endTimeStamp")]
        public String EndTimeStamp { get; set; }
        [JsonProperty("elapsedMS")]
        public double ElapsedMS { get; set; }
        [JsonProperty("messagesPerSecond")]
        public double MessagesPerSecond { get; set; }
        [JsonProperty("averageProcessingMS")]
        public double AvergeProcessingMs { get; set; }
        [JsonProperty("version")]
        public String Version { get; set; }
        [JsonProperty("instanceId")]
        public String InstanceId { get; set; }

        [JsonProperty("hostId")]
        public String HostId { get; set; }
        [JsonProperty("status")]
        public string Status { get; set; }
        [JsonProperty("pipelineModuleId")]
        public String PipelineModuleId { get; set; }
        [JsonProperty("messagesProcessed")]
        public int MessagesProcessed { get; set; }
        [JsonProperty("deadLetterCount")]
        public int DeadLetterCount { get; set; }
        [JsonProperty("bytesProccessed")]
        public long BytesProcessed { get; set; }
        [JsonProperty("errorCount")]
        public int ErrorCount { get; set; }
        [JsonProperty("warningCount")]
        public int WarningCount { get; set; }
        [JsonProperty("activeCount")]
        public int ActiveCount { get; set; }
        [JsonProperty("processingMS")]
        public double ProcessingMS { get; set; }
    }
}