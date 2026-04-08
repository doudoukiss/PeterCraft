param(
  [string]$ConfigurePreset = 'windows-vs2022-dev',
  [string]$Configuration = 'Debug',
  [string]$ProfileId = 'player.default',
  [string]$Scenario = 'guided_first_run',
  [string[]]$AppArguments = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$applicationPath = Get-BuildOutputPath -ConfigurePreset $ConfigurePreset -Configuration $Configuration

if (-not (Test-Path $applicationPath)) {
  throw "Application not found at $applicationPath. Run build.ps1 first."
}

$resolvedArguments = @('--profile-id', $ProfileId, '--scenario', $Scenario) + $AppArguments
& $applicationPath @resolvedArguments
