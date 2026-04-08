param(
  [string]$ConfigurePreset = 'windows-vs2022-dev',
  [string]$BuildPreset = 'windows-vs2022-dev-debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  cmake --preset $ConfigurePreset
  cmake --build --preset $BuildPreset
} finally {
  Pop-Location
}
