param(
  [string]$ConfigurePreset = 'windows-vs2022-playable-runtime',
  [string]$Configuration = 'Debug',
  [string]$ProfileId = 'player.default',
  [string]$Scenario = 'guided_first_run',
  [switch]$LaunchOneRoomProof,
  [string[]]$AppArguments = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
$applicationPath = Get-BuildOutputPath -ConfigurePreset $ConfigurePreset -Configuration $Configuration
$o3deProjectRoot = Get-O3DEProjectRoot
$o3deLauncher = Get-O3DEGameLauncherPath
$generatedRoot = Join-Path $repoRoot 'Saved\Generated\o3de'
$runSummaryPath = Join-Path $generatedRoot 'phase7_1_runtime_spike.md'
$decisionGatesPath = Join-Path $generatedRoot 'phase7_1_decision_gates.md'

Assert-PathExists -Path $applicationPath -Description 'Playable host application'
Assert-PathExists -Path $o3deProjectRoot -Description 'PeterCraft O3DE project'
Assert-PathExists -Path $o3deLauncher -Description 'O3DE GameLauncher'

New-Item -ItemType Directory -Path $generatedRoot -Force | Out-Null
Register-O3DEProject
Ensure-O3DEFirewallRules

if ($LaunchOneRoomProof) {
  $started = Get-Date
  Start-Process -FilePath $o3deLauncher -WorkingDirectory $o3deProjectRoot -ArgumentList @(
    "--project-path=$o3deProjectRoot",
    '--regset=/O3DE/Autoexec/ConsoleCommands/LoadLevel=one_room_proof'
  ) | Out-Null
  $elapsed = [Math]::Round(((Get-Date) - $started).TotalSeconds, 2)

  $summary = @"
# Phase 7.1 Runtime Spike

- Timestamp: $(Get-Date)
- Mode: one_room_proof
- Time to first playable room seconds: $elapsed
- O3DE project: $o3deProjectRoot
- Launcher: $o3deLauncher
"@
  Set-Content -Path $runSummaryPath -Value $summary -Encoding UTF8

  $decisionGates = @"
# Phase 7.1 O3DE Decision Gates

- time to first playable room: measured ($elapsed seconds)
- time to add one interactable: unmeasured
- time to add one enemy archetype: unmeasured
- time to author one blockout room: unmeasured
- time to debug one AI route failure: unmeasured
- build reproducibility: see phase7_1_build_summary.md
- average iteration time for a gameplay parameter change: unmeasured
"@
  Set-Content -Path $decisionGatesPath -Value $decisionGates -Encoding UTF8
  Write-Host 'Launched the O3DE one-room proof.'
  exit 0
}

$resolvedArguments = @('--runtime', 'playable', '--profile-id', $ProfileId, '--scenario', $Scenario) + $AppArguments
Push-Location $repoRoot

try {
  & $applicationPath @resolvedArguments
  $exitCode = $LASTEXITCODE
} finally {
  Pop-Location
}

$summary = @"
# Phase 7.1 Runtime Spike

- Timestamp: $(Get-Date)
- Mode: playable_runtime
- Profile: $ProfileId
- Scenario: $Scenario
- Host app: $applicationPath
- O3DE project: $o3deProjectRoot
- Launcher: $o3deLauncher
- Exit code: $exitCode
"@
Set-Content -Path $runSummaryPath -Value $summary -Encoding UTF8

if ($exitCode -ne 0) {
  exit $exitCode
}
