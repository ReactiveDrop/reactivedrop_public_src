[CmdletBinding()]
param (
    [Parameter(Mandatory=$true, ValueFromPipeline=$true)][System.IO.FileInfo]$File,
    [Parameter(Mandatory=$true)][string]$Version,
    [Parameter(Mandatory=$false)][switch]$Dynamic,
    [Parameter(Mandatory=$false)][System.UInt32]$Threads = 0,
    [Parameter(Mandatory=$false)][System.UInt32]$Optimize = 3
)

if ($Version -notin @("20b","30","40","41","50","51")) { return }

foreach ($line in Get-Content $File) {
    if ($line -match '^\s*$' -or $line -match '^\s*//') { continue }

    $args = @()
    if ($Dynamic) { $args += "-dynamic" }
    if ($Threads -ne 0) { $args += "-threads"; $args += $Threads }
    $args += "-ver"; $args += $Version
    $args += "-shaderpath"; $args += $File.DirectoryName
    if (-not $Dynamic) { $args += "-optimize"; $args += $Optimize }
    $args += $line

    & "$PSScriptRoot\ShaderCompile" $args
}
