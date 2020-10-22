$invokePath = (Split-Path $MyInvocation.MyCommand.Path)
$invokePath
Set-Location $invokePath

cd .\pem-handler

Docker build -t bytemaster/pem-handler .
Docker push bytemaster/pem-handler
cd ..
faas-cli deploy --replace --update=false -f pem-handler.yml
