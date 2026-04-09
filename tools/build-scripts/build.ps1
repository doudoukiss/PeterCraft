param(
  [string]$ConfigurePreset = 'windows-vs2022-headless',
  [string]$BuildPreset = 'windows-vs2022-headless-debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  & (Join-Path $PSScriptRoot 'build-headless.ps1') -ConfigurePreset $ConfigurePreset -BuildPreset $BuildPreset
} finally {
  Pop-Location
}
