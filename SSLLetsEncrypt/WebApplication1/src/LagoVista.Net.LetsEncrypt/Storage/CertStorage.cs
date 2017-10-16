using LagoVista.Net.LetsEncrypt.Interfaces;
using System.Threading.Tasks;
using System;

namespace LagoVista.Net.LetsEncrypt.Storage
{
    public class CertStorage : IStorage
    {
        IAcmeSettings _settings;

        public CertStorage(IAcmeSettings settings)
        {
            _settings = settings;
        }

        public Task<byte[]> GetCertAsync(string domainName)
        {
            throw new NotImplementedException();
        }

        public Task<string> GetResponseAsync(string challenge)
        {
            throw new NotImplementedException();
        }

        public Task SetChallengeAndResponseAsync(string challenge, string response)
        {
            throw new NotImplementedException();
        }

        public Task StoreCertAsync(string domainName, byte[] bytes)
        {
            throw new NotImplementedException();
        }
    }
}
