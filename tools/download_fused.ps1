# Download fused image from XIAO_Fusion access point
# Usage: Run this after connecting your PC to the XIAO_Fusion Wi-Fi AP.
param(
    [string]$out = "fused_download.rgb565",
    [string]$url = "http://192.168.4.1/"
)

Write-Host "Downloading fused image from $url to $out"
try{
    Invoke-WebRequest -Uri $url -OutFile $out -UseBasicParsing -TimeoutSec 30
    Write-Host "Downloaded to $out"
}catch{
    Write-Host "Failed to download: $_"
}
