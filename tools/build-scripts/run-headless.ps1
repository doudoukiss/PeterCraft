param(
  [string]$ConfigurePreset = 'windows-vs2022-headless',
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
  throw "Application not found at $applicationPath. Run build-headless.ps1 first."
}

$resolvedArguments = @('--runtime', 'headless', '--profile-id', $ProfileId, '--scenario', $Scenario) + $AppArguments
Push-Location $repoRoot

try {
  & $applicationPath @resolvedArguments
  $exitCode = $LASTEXITCODE
} finally {
  Pop-Location
}

if ($exitCode -ne 0) {
  exit $exitCode
}
