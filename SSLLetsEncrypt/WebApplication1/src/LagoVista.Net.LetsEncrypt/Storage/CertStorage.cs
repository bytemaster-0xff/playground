using LagoVista.Net.LetsEncrypt.Interfaces;
using System.Threading.Tasks;
using System;

namespace LagoVista.Net.LetsEncrypt.Storage
{
    public class CertStorage : IStorage
    {
        IAcmeSettings _settings;

        private static string _response;

        public CertStorage(IAcmeSettings settings)
        {
            _settings = settings;
        }

        public Task<byte[]> GetCertAsync(string domainName)
        {
            var fullPath = $@"X:\Temp\{domainName}.cer";
            if(System.IO.File.Exists(fullPath))
            {
                return Task.FromResult( System.IO.File.ReadAllBytes(fullPath));
            }

            return Task.FromResult(default(byte[]));         
        }

        public Task<string> GetResponseAsync(string challenge)
        {
            return Task.FromResult( _response);
        }

        public Task SetChallengeAndResponseAsync(string challenge, string response)
        {
            _response = response;
            return Task.FromResult(default(object));
        }

        public Task StoreCertAsync(string domainName, byte[] bytes)
        {
            var fullPath = $@"X:\Temp\{domainName}.cer";

            System.IO.File.WriteAllBytes(fullPath, bytes);

            return Task.FromResult(default(object));
        }
    }
}
