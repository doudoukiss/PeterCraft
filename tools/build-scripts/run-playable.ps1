param(
  [string]$ConfigurePreset = 'windows-vs2022-playable-preflight',
  [string]$Configuration = 'Debug',
  [string]$ProfileId = 'player.default',
  [string]$Scenario = 'guided_first_run',
  [string[]]$AppArguments = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
$applicationPath = Get-BuildOutputPath -ConfigurePreset $ConfigurePreset -Configuration $Configuration

if (-not (Test-Path $applicationPath)) {
  throw "Application not found at $applicationPath. Run build-playable.ps1 first."
}

$resolvedArguments = @('--runtime', 'playable', '--profile-id', $ProfileId, '--scenario', $Scenario) + $AppArguments
Push-Location $repoRoot

try {
  & $applicationPath @resolvedArguments
  $exitCode = $LASTEXITCODE
} finally {
  Pop-Location
}

if ($exitCode -eq 2) {
  Write-Host 'Playable runtime preflight confirmed. Backend unavailable until Phase 7.1.'
  exit 0
}

if ($exitCode -ne 0) {
  exit $exitCode
}
