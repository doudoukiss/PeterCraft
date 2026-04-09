param(
  [string]$ConfigurePreset = 'windows-vs2022-headless',
  [string]$Configuration = 'Debug',
  [string]$ProfileId = 'player.default',
  [string]$Scenario = 'guided_first_run',
  [string[]]$AppArguments = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

& (Join-Path $PSScriptRoot 'run-headless.ps1') `
  -ConfigurePreset $ConfigurePreset `
  -Configuration $Configuration `
  -ProfileId $ProfileId `
  -Scenario $Scenario `
  -AppArguments $AppArguments
