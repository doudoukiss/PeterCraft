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
    [string]$ConfigurePreset = 'windows-vs2022-dev',
    [string]$Configuration = 'Debug'
  )

  return Join-Path (Get-RepoRoot) "out\build\$ConfigurePreset\bin\$Configuration\PeterCraftApp.exe"
}
