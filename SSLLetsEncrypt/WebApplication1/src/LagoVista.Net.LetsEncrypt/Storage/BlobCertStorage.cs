using LagoVista.Net.LetsEncrypt.Interfaces;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System.Threading.Tasks;
using System.IO;

namespace LagoVista.Net.LetsEncrypt.Storage
{
    public class BlobCertStorage : IStorage
    {
        IAcmeSettings _settings;

        private static string _response;

        public BlobCertStorage(IAcmeSettings settings)
        {
            _settings = settings;
        }

        private async Task<CloudBlobContainer> GetContainerAsync()
        {
            var connectionString = $"DefaultEndpointsProtocol=https;AccountName={_settings.StorageAccountName};AccountKey={_settings.StorageKey}";
            var storageAccount = CloudStorageAccount.Parse(connectionString);
            var blobClient = storageAccount.CreateCloudBlobClient();
            var container = blobClient.GetContainerReference(_settings.StorageContainerName);
            await container.CreateIfNotExistsAsync();

            return container;
        }

        public async Task<byte[]> GetCertAsync(string domainName)
        {
            using (var ms = new MemoryStream())
            {
                var container = await GetContainerAsync();
                var blob = container.GetBlockBlobReference(domainName);
                if(!await blob.ExistsAsync())
                {
                    return null;
                }

                await blob.DownloadToStreamAsync(ms);
                return ms.ToArray();
            }
        }

        public Task<string> GetResponseAsync(string challenge)
        {
            return Task.FromResult(_response);
        }

        public Task SetChallengeAndResponseAsync(string challenge, string response)
        {
            _response = response;
            return Task.FromResult(default(object));
        }

        public async Task StoreCertAsync(string domainName, byte[] bytes)
        {
            using (var ms = new MemoryStream(bytes))
            {
                var container = await GetContainerAsync();
                var blob = container.GetBlockBlobReference(domainName);
                await blob.DeleteIfExistsAsync();
                await blob.UploadFromStreamAsync(ms);
            }
        }
    }
}
