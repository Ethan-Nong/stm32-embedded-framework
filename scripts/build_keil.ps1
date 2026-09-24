param(
    [Parameter(Mandatory = $true)]
    [string]$Project,
    [string]$Uv4Path = "D:\Keil_v5\UV4\UV4.exe"
)

$root = Split-Path -Parent $PSScriptRoot
python "$PSScriptRoot\sync_keil.py" $Project
if ($LASTEXITCODE -ne 0) {
    throw "Failed to synchronize project: $Project"
}

$manifest = Get-Content -Raw (Join-Path $root "projects\$Project\project.json") | ConvertFrom-Json
$projectFile = Join-Path $root "platform\$($manifest.platform)\cube\MDK-ARM\$Project.uvprojx"
$logFile = Join-Path $root "build\keil\$Project.log"
New-Item -ItemType Directory -Path (Split-Path -Parent $logFile) -Force | Out-Null

$process = Start-Process -FilePath $Uv4Path `
    -ArgumentList @("-b", $projectFile, "-o", $logFile) `
    -WindowStyle Hidden `
    -Wait `
    -PassThru

Get-Content -LiteralPath $logFile -Tail 20
exit $process.ExitCode
