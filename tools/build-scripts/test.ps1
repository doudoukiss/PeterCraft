param(
  [string]$ConfigurePreset = 'windows-vs2022-dev',
  [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  ctest -C $Configuration --output-on-failure --test-dir (Join-Path $repoRoot "out/build/$ConfigurePreset")
} finally {
  Pop-Location
}
