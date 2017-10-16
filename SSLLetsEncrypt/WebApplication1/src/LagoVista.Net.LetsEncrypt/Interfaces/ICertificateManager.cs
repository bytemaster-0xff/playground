using System.Threading.Tasks;
using System.Security.Cryptography.X509Certificates;

namespace LagoVista.Net.LetsEncrypt.AcmeServices.Interfaces
{
    public interface ICertificateManager
    {
        Task<X509Certificate2> GetCertificate(string domainNames);
    }
}