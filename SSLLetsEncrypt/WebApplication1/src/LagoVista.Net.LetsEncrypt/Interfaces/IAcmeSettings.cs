using System;
using System.Collections.Generic;
using System.Text;

namespace LagoVista.Net.LetsEncrypt.Interfaces
{
    public interface IAcmeSettings
    {
        Uri AcmeUri { get; }
        string EmailAddress { get; }
        string PfxPassword { get; }
        TimeSpan RenewalPeriod { get; }
        bool Diagnostics { get;  }
    }
}
