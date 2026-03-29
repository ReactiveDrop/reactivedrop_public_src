[CmdletBinding()]
param (
    [Parameter(Mandatory=$true, ValueFromPipeline=$true)][System.IO.FileInfo]$File,
    [Parameter(Mandatory=$true)][string]$Version,
    [Parameter(Mandatory=$false)][switch]$Force,
    [Parameter(Mandatory=$false)][switch]$Dynamic,
    [Parameter(Mandatory=$false)][int]$Threads = 0,
    [Parameter(Mandatory=$false)][int]$Optimize = 3
)

if ($Version -notin @("20b","30","40","41","50","51")) { return }

$ShaderStart = Get-Date
foreach ($line in Get-Content $File) {
    if ($line -match '^\s*$' -or $line -match '^\s*//') { continue }

    $args = @()

    if ($Force)   { $args += "-force" }
    $args += "-force"

    if ($Threads -gt 0) { $args += "-threads", $Threads }

    $args += "-ver", $Version
    $args += "-shaderpath", $File.DirectoryName

    if ($Dynamic) {
        $args += "-dynamic"
    } else {
        $args += "-optimize", $Optimize
    }

    $args += $line
    $ShaderStart = Get-Date
    & "$PSScriptRoot\ShaderCompile" $args
}
$TotalTime = (Get-Date) - $ShaderStart

if ($Force) {
    $modeText  = "FORCE (full recompilation)"
    $modeColor = "Magenta"
} else {
    $modeText  = "ON-DEMAND (recompile changed shaders only)"
    $modeColor = "Cyan"
}
Write-Host ""
Write-Host "Shader compilation pass for '$($File.Name)' finished." -ForegroundColor Green
Write-Host "Mode: $modeText" -ForegroundColor $modeColor
Write-Host ("Time elapsed: {0}h:{1:mm}m:{1:ss}s:{1:ff}ms" -f [int]$TotalTime.TotalHours, $TotalTime) -ForegroundColor Cyan
Write-Host ""
