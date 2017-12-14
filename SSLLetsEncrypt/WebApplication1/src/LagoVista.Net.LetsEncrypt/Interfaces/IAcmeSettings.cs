using System;
using System.Collections.Generic;
using System.Text;

namespace LagoVista.Net.LetsEncrypt.Interfaces
{
    public enum StorageLocation
    {
        BlobStorage,
        FileSystem
    }

    public interface IAcmeSettings
    {
        /// <summary>
        /// Selected option to store cert locally or in azure blob storage
        /// </summary>
        StorageLocation StorageLocation { get; }


        /// <summary>
        /// URI that should be used to get cert, readonly, staging URI will be returned development flag is set.
        /// </summary>
        Uri AcmeUri { get; }

        /// <summary>
        /// Email on 
        /// </summary>
        string EmailAddress { get; }

        /// <summary>
        /// Password associated with certificate
        /// </summary>
        string PfxPassword { get; }

        /// <summary>
        /// Number of days prior to 3 month expiration that the cert will be renewed rather than loaded
        /// </summary>
        TimeSpan RenewalPeriod { get; }

        ///<summary>
        /// While in diagnostics mode, the logger will receive diagnostics events
        /// </summary>
        bool Diagnostics { get;  }

        /// <summary>
        /// Account name of Azure Storage Account, use to store cert in blob
        /// </summary>
        string StorageAccountName { get; }

        /// <summary>
        /// Account key of Azure Storage Account, used to store cert in blob
        /// </summary>
        string StorageKey { get; }

        /// <summary>
        /// Name of container where cert will be stored in blob storage
        /// </summary>
        string StorageContainerName { get; }

        /// <summary>
        /// Path to store key if storing locally.
        /// </summary>
        string StoragePath { get; }

        /// <summary>
        /// If Development is true, use staging server to obtain certificate (invalid CSA but no rate limiting) 
        /// </summary>
        bool Development { get; set; }

    }
}
