[CmdletBinding()]
param (
    [Parameter(Mandatory=$true, ValueFromPipeline=$true)][System.IO.FileInfo]$File,
    [Parameter(Mandatory=$true)][string]$Version,
    [Parameter(Mandatory=$false)][switch]$Force,
    [Parameter(Mandatory=$false)][switch]$Dynamic,
    [Parameter(Mandatory=$false)][int]$Threads = 0,
    [Parameter(Mandatory=$false)][int]$Optimize = 3
)

# =====================================
# Validate input
# =====================================
if ($Version -notin @("20b","30","40","41","50","51")) {
    return
}

# =====================================
# Resolve compilation parameters
# =====================================
$argsBase = @()
if ($Force) { $argsBase += "-force" }

if ($Threads -is [int] -and $Threads -gt 0) {
    $ThreadsToUse = [Math]::Min($Threads, [int]$env:NUMBER_OF_PROCESSORS)
    $argsBase += "-threads", $ThreadsToUse
}

$argsBase += "-ver", $Version
$argsBase += "-shaderpath", $File.DirectoryName

if ($Dynamic) {
    $argsBase += "-dynamic"
} elseif ($Optimize -is [int]) {
    $optVal = [Math]::Min([Math]::Max($Optimize, 0), 3)
    $argsBase += "-optimize", $optVal
}

# =====================================
# Logging 1
# =====================================
$modeParts = @()
if ($Force) { $modeParts += "FORCE" }
if ($Dynamic) { $modeParts += "DYNAMIC" }
if (-not $Force -and -not $Dynamic) { $modeParts += "ON-DEMAND" }

$modeText  = $modeParts -join " + "
$modeColor = if ($Force -or $Dynamic) { "Magenta" } else { "White" }

Write-Host "`nMode: $modeText`n" -ForegroundColor $modeColor

# =====================================
# Main shader compilation loop
# =====================================
$ShaderStart = Get-Date
foreach ($line in Get-Content $File) {
    if ($line -match '^\s*$' -or $line -match '^\s*//') { continue }

    # Build full args for this shader line
    $args = @($argsBase)
    $args += $line

    # Invoke ShaderCompile
    & "$PSScriptRoot\ShaderCompile" $args
}
$TotalTime = (Get-Date) - $ShaderStart

# =====================================
# Logging 2
# =====================================
Write-Host "`nShader compilation pass for $($File.Name) finished."
Write-Host ("Time elapsed: {0}h:{1:mm}m:{1:ss}s:{1:ff}ms`n" -f [int]$TotalTime.TotalHours, $TotalTime) -ForegroundColor Cyan
