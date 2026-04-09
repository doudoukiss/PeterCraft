$ErrorActionPreference = 'Stop'

function Get-RepoRoot {
  return (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}

function Get-VenvPython {
  $pythonPath = Join-Path (Get-RepoRoot) '.venv\Scripts\python.exe'
  if (-not (Test-Path $pythonPath)) {
    throw "Virtual environment not found. Run bootstrap.ps1 first."
  }

  return $pythonPath
}

function Get-BuildOutputPath {
  param(
    [string]$ConfigurePreset = 'windows-vs2022-headless',
    [string]$Configuration = 'Debug'
  )

  return Join-Path (Get-RepoRoot) "out\build\$ConfigurePreset\bin\$Configuration\PeterCraftApp.exe"
}

function Get-O3DERoot {
  if ($env:PETERCRAFT_O3DE_ROOT) {
    return $env:PETERCRAFT_O3DE_ROOT
  }

  return 'C:\o3de\25.10.2'
}

function Get-O3DEProjectRoot {
  return Join-Path (Get-RepoRoot) 'game\o3de'
}

function Get-O3DECliPath {
  return Join-Path (Get-O3DERoot) 'scripts\o3de.bat'
}

function Get-O3DEGameLauncherPath {
  $projectLauncher = Join-Path (Get-O3DEProjectBuildRoot) 'bin\profile\PeterCraftRuntime.GameLauncher.exe'
  if (Test-Path $projectLauncher) {
    return $projectLauncher
  }

  return Join-Path (Get-O3DERoot) 'bin\Windows\profile\Default\O3DE.GameLauncher.exe'
}

function Get-O3DEEditorPath {
  return Join-Path (Get-O3DERoot) 'bin\Windows\profile\Default\Editor.exe'
}

function Get-O3DEAssetProcessorPath {
  return Join-Path (Get-O3DERoot) 'bin\Windows\profile\Default\AssetProcessorBatch.exe'
}

function Get-O3DEProjectBuildRoot {
  param(
    [string]$Name = 'windows-vs2022-playable-runtime'
  )

  return Join-Path (Get-RepoRoot) "out\o3de\$Name"
}

function Assert-PathExists {
  param(
    [string]$Path,
    [string]$Description
  )

  if (-not (Test-Path $Path)) {
    throw "$Description not found at $Path"
  }
}

function Write-O3DEUserProjectOverride {
  $projectRoot = Get-O3DEProjectRoot
  $engineRoot = Get-O3DERoot
  $engineRootJson = $engineRoot.Replace('\', '/')
  $userProjectDirectory = Join-Path $projectRoot 'user'
  $userProjectPath = Join-Path $userProjectDirectory 'project.json'

  New-Item -ItemType Directory -Path $userProjectDirectory -Force | Out-Null
  $json = "{`n  `"engine_path`": `"$engineRootJson`"`n}`n"
  Set-Content -Path $userProjectPath -Value $json -Encoding UTF8
}

function Register-O3DEProject {
  $o3deCli = Get-O3DECliPath
  $engineRoot = Get-O3DERoot
  $projectRoot = Get-O3DEProjectRoot

  Assert-PathExists -Path $o3deCli -Description 'O3DE CLI'
  Assert-PathExists -Path $projectRoot -Description 'PeterCraft O3DE project'
  Write-O3DEUserProjectOverride

  & $o3deCli register --engine-path $engineRoot --force
  Assert-LastExitCode 'Register O3DE engine'
  & $o3deCli register --project-path $projectRoot --force
  Assert-LastExitCode 'Register PeterCraft O3DE project'
}

function Ensure-O3DEFirewallRules {
  $programs = @(
    @{ Name = 'PeterCraftRuntime GameLauncher'; Path = Get-O3DEGameLauncherPath },
    @{ Name = 'PeterCraftRuntime AssetProcessor'; Path = (Join-Path (Get-O3DERoot) 'bin\Windows\profile\Default\AssetProcessor.exe') },
    @{ Name = 'PeterCraftRuntime AssetProcessorBatch'; Path = (Join-Path (Get-O3DERoot) 'bin\Windows\profile\Default\AssetProcessorBatch.exe') }
  )

  foreach ($program in $programs) {
    if (-not (Test-Path $program.Path)) {
      continue
    }

    $existing = Get-NetFirewallApplicationFilter -PolicyStore ActiveStore -ErrorAction SilentlyContinue |
      Where-Object { $_.Program -eq $program.Path }
    if (-not $existing) {
      New-NetFirewallRule -DisplayName $program.Name -Direction Inbound -Action Allow -Program $program.Path -Profile Any | Out-Null
    }
  }
}

function Assert-LastExitCode {
  param(
    [string]$Context = 'Native command'
  )

  if ($LASTEXITCODE -ne 0) {
    throw "$Context failed with exit code $LASTEXITCODE."
  }
}
