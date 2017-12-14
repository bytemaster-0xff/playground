using System;
using System.Threading.Tasks;
using Certes;
using Certes.Acme;
using System.Linq;
using Certes.Pkcs;
using System.Security.Cryptography.X509Certificates;
using Newtonsoft.Json;
using LagoVista.Net.LetsEncrypt.AcmeServices.Interfaces;
using LagoVista.Net.LetsEncrypt.Models;
using LagoVista.Net.LetsEncrypt.Interfaces;
using Microsoft.Extensions.Logging;

namespace LagoVista.Net.LetsEncrypt.AcmeServices
{
    public class AcmeCertificateManager : ICertificateManager
    {
        readonly IAcmeSettings _settings;
        readonly IStorage _storage;
        readonly ILogger _logger;

        public AcmeCertificateManager(IStorage storage, IAcmeSettings settings)
        {
            _storage = storage;
            _settings = settings;
         }

        public async Task<X509Certificate2> GetCertificate(string domainName)
        {
            var pfx = await _storage.GetCertAsync(domainName);
            if (pfx != null)
            {
                var cert = new X509Certificate2(pfx, _settings.PfxPassword);
                if (cert.NotAfter - DateTime.UtcNow > _settings.RenewalPeriod)
                {
                    return cert;
                }
            }

            pfx = await RequestNewCertificate(domainName);
            await _storage.StoreCertAsync(domainName, pfx);
            return new X509Certificate2(pfx, _settings.PfxPassword);
        }

        private async Task<AcmeResult<Authorization>> PollResultAsync(AcmeClient client, Uri uri)
        {
            int attempt = 0;
            do
            {
                await Task.Delay(5000 * attempt);
                var auth = await client.GetAuthorization(uri);
                if (auth.Data.Status != EntityStatus.Pending)
                {
                    return auth;
                }
            }
            while (++attempt < 5);

            return null;
        }

        private async Task<AcmeResult<Authorization>> GetAuthorizationAsync(AcmeClient client, string domainName)
        {
            var account = await client.NewRegistraton($"mailto:{_settings.EmailAddress}");
            account.Data.Agreement = account.GetTermsOfServiceUri();
            account = await client.UpdateRegistration(account);

            var auth = await client.NewAuthorization(new AuthorizationIdentifier
            {
                Type = AuthorizationIdentifierTypes.Dns,
                Value = domainName
            });

            var challenge = auth.Data.Challenges.Where(c => c.Type == ChallengeTypes.Http01).First();
            var response = client.ComputeKeyAuthorization(challenge);
            await _storage.SetChallengeAndResponseAsync(challenge.Token, response);
            var httpChallenge = await client.CompleteChallenge(challenge);
            return await PollResultAsync(client, httpChallenge.Location);
        }

        private async Task<byte[]> RequestNewCertificate(String domainName)
        {
            using (var client = new AcmeClient(_settings.AcmeUri))
            {
                var result = await GetAuthorizationAsync(client, domainName);

                if (result.Data.Status != EntityStatus.Valid)
                {
                    var acmeResponse = JsonConvert.DeserializeObject<AcmeResponseModel>(result.Json);
                    return null;
                }

                var csr = new CertificationRequestBuilder();
                csr.AddName("CN", domainName);

                var cert = await client.NewCertificate(csr);
                return cert.ToPfx().Build(domainName, _settings.PfxPassword);
            }
        }
    }
}