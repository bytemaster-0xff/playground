Write-Host -NoNewline "Public (P = Production, D = Development, S = Staging, [ENTER] = None"
$key = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');

$end = Get-Date
$start = Get-Date "5/17/2017"

$today = Get-Date
$today = $today.ToShortDateString()
$today = Get-Date $today

$major = 4
$minor = 0

$minutes = New-TimeSpan -Start $today -End $end
$revisionNumber = (New-TimeSpan -Start $start -End $end).Days
$buildNumber = ("{0:00}" -f [math]::Round($minutes.Hours)) + ("{0:00}" -f ([math]::Round($minutes.Minutes)))
$version = "$major.$minor.$revisionNumber.$buildNumber"
$version

$fullTag = "nuviot/curb-energy-sender:v$version"
$fullTag

$now = [System.DateTime]::UtcNow

$isoDate = $now.toString("u").Replace(" ", "T");

$versionJson = "const version = {""version"":""$version"",""buildDate"":""$isoDate""}; exports.version = version;";
[System.IO.File]::WriteAllLines('.\version.js', $versionJson);

docker build -t $fullTag  .
docker push $fullTag

$yaml = (Get-Content k8.deployment.yaml) -replace 'nuviot/[a-z-]+:v[\d]+\.[\d]+\.[\d]+\.[\d]+', $fullTag
[System.IO.File]::WriteAllLines( 'k8.deployment.yaml', $yaml)

$key = $key.Character.ToString()

if(($key -eq "d") -or  ($key -eq "D")){
  kubectl config use-context do-nyc3-nuviot-dev
  kubectl apply -f k8.deployment.yaml
  "Publish Development"
}
elseif(($key -eq "s") -or  ($key -eq "S")){
  "Publish Staging"
}
elseif(($key -eq "P") -or  ($key -eq "P")){
  kubectl config use-context do-nyc3-nuviot-prod
  kubectl apply -f k8.deployment.yaml
  "Publish Production"
}
else {
  "Not Publishing"
}
