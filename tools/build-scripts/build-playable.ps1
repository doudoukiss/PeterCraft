param(
  [string]$ConfigurePreset = 'windows-vs2022-playable-preflight',
  [string]$BuildPreset = 'windows-vs2022-playable-preflight-debug'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  cmake --preset $ConfigurePreset
  Assert-LastExitCode 'Configure playable preflight'
  cmake --build --preset $BuildPreset
  Assert-LastExitCode 'Build playable preflight'
  Write-Host 'Playable runtime preflight configured successfully.'
  Write-Host 'Phase 7.0 does not build an engine-backed runtime yet. Real playable builds start in Phase 7.1.'
} finally {
  Pop-Location
}
