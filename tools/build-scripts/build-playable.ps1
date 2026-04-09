param(
  [string]$ConfigurePreset = 'windows-vs2022-playable-runtime',
  [string]$BuildPreset = 'windows-vs2022-playable-runtime-debug',
  [string]$ProjectBuildName = 'windows-vs2022-playable-runtime',
  [switch]$SkipProjectBuild,
  [switch]$SkipAssetProcessing
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
$o3deRoot = Get-O3DERoot
$o3deProjectRoot = Get-O3DEProjectRoot
$o3deLauncher = Get-O3DEGameLauncherPath
$o3deEditor = Get-O3DEEditorPath
$assetProcessor = Get-O3DEAssetProcessorPath
$o3deProjectBuildRoot = Get-O3DEProjectBuildRoot -Name $ProjectBuildName
$generatedRoot = Join-Path $repoRoot 'Saved\Generated\o3de'
$buildSummaryPath = Join-Path $generatedRoot 'phase7_1_build_summary.md'
$decisionGatesPath = Join-Path $generatedRoot 'phase7_1_decision_gates.md'

Assert-PathExists -Path $o3deRoot -Description 'O3DE root'
Assert-PathExists -Path $o3deProjectRoot -Description 'PeterCraft O3DE project'
Assert-PathExists -Path $o3deLauncher -Description 'O3DE GameLauncher'
Assert-PathExists -Path $o3deEditor -Description 'O3DE Editor'
Assert-PathExists -Path $assetProcessor -Description 'O3DE AssetProcessorBatch'

New-Item -ItemType Directory -Path $generatedRoot -Force | Out-Null
$started = Get-Date

Push-Location $repoRoot

try {
  Register-O3DEProject

  cmake --preset $ConfigurePreset
  Assert-LastExitCode 'Configure playable host runtime'
  cmake --build --preset $BuildPreset
  Assert-LastExitCode 'Build playable host runtime'

  if (-not $SkipProjectBuild) {
    cmake -S $o3deProjectRoot -B $o3deProjectBuildRoot -G 'Visual Studio 17 2022' -A x64
    Assert-LastExitCode 'Configure O3DE playable project'

    cmake --build $o3deProjectBuildRoot --target PeterCraftRuntime.GameLauncher Editor --config profile -- /m
    Assert-LastExitCode 'Build O3DE GameLauncher and Editor'
  }

  Ensure-O3DEFirewallRules

  if (-not $SkipAssetProcessing) {
    $projectUserPath = Join-Path $repoRoot 'Saved\O3DEUser'
    $projectLogPath = Join-Path $repoRoot 'Saved\O3DELogs'
    New-Item -ItemType Directory -Path $projectUserPath -Force | Out-Null
    New-Item -ItemType Directory -Path $projectLogPath -Force | Out-Null

    & $assetProcessor "--project-path=$o3deProjectRoot" "--project-user-path=$projectUserPath" "--project-log-path=$projectLogPath"
    Assert-LastExitCode 'Process O3DE project assets'
  }

  $finished = Get-Date
  $elapsed = [Math]::Round(($finished - $started).TotalSeconds, 2)
  $summary = @"
# Phase 7.1 Playable Build Summary

- Built at: $finished
- Duration seconds: $elapsed
- O3DE root: $o3deRoot
- O3DE project: $o3deProjectRoot
- Host configure preset: $ConfigurePreset
- Host build preset: $BuildPreset
- Project build root: $o3deProjectBuildRoot
- Launcher path: $o3deLauncher
- Editor path: $o3deEditor
- Asset processing skipped: $SkipAssetProcessing
- Project build skipped: $SkipProjectBuild
"@
  Set-Content -Path $buildSummaryPath -Value $summary -Encoding UTF8

  $decisionGates = @"
# Phase 7.1 O3DE Decision Gates

- time to first playable room: unmeasured
- time to add one interactable: unmeasured
- time to add one enemy archetype: unmeasured
- time to author one blockout room: unmeasured
- time to debug one AI route failure: unmeasured
- build reproducibility: measured ($elapsed seconds for this local build run)
- average iteration time for a gameplay parameter change: unmeasured
"@
  Set-Content -Path $decisionGatesPath -Value $decisionGates -Encoding UTF8

  Write-Host 'Playable runtime configured successfully.'
  Write-Host "O3DE root: $o3deRoot"
  Write-Host "O3DE project: $o3deProjectRoot"
  Write-Host "Build summary: $buildSummaryPath"
} finally {
  Pop-Location
}
