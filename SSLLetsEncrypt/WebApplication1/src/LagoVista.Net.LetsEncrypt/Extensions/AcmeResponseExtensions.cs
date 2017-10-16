using Microsoft.AspNetCore.Builder;
using LagoVista.Net.LetsEncrypt.AcmeServices.Middleware;

namespace Microsoft.AspNetCore.Hosting
{
    public static class AcmeResponseExtensions
    {
        public static IApplicationBuilder UseAcmeResponse(this IApplicationBuilder builder)
        {
            return builder.UseMiddleware<AcmeResponseMiddleware>();
        }
    }
}