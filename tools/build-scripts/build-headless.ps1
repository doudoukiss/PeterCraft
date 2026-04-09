param(
  [string]$ConfigurePreset = 'windows-vs2022-headless',
  [string]$BuildPreset = 'windows-vs2022-headless-debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  cmake --preset $ConfigurePreset
  Assert-LastExitCode 'Configure headless build'
  cmake --build --preset $BuildPreset
  Assert-LastExitCode 'Build headless runtime'
} finally {
  Pop-Location
}
