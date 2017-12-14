using LagoVista.Net.LetsEncrypt.Interfaces;
using System;

namespace LagoVista.Net.LetsEncrypt.Models
{
    public class AcmeSettings : IAcmeSettings
    {
        public string EmailAddress { get; set; }
        public string PfxPassword { get; set; }

        public Uri AcmeUri
        {
            get{return Development ? new Uri("https://acme-staging.api.letsencrypt.org/directory") : new Uri("https://acme-v01.api.letsencrypt.org/directory");}
        }

        public bool Diagnostics { get; set; } = false;
        public bool Development { get; set; } = false;

        public string StorageAccountName { get; set; }
        public string StorageKey { get; set; }

        public TimeSpan RenewalPeriod { get; set; }

        public StorageLocation StorageLocation { get; set; }

        public string StorageContainerName { get; set; }

        public string StoragePath { get; set; }

        public AcmeSettings()
        {
            RenewalPeriod = TimeSpan.FromDays(14);
        }
    }
}
