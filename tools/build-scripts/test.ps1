param(
  [string]$TestPreset = 'windows-vs2022-dev-debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  ctest --preset $TestPreset
} finally {
  Pop-Location
}
