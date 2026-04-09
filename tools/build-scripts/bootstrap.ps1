$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
Push-Location $repoRoot

try {
  if (-not (Test-Path '.venv\Scripts\python.exe')) {
    py -3 -m venv .venv
    Assert-LastExitCode 'Create virtual environment'
  }

  & .venv\Scripts\python.exe -m pip install --upgrade pip
  Assert-LastExitCode 'Upgrade pip'
  & .venv\Scripts\python.exe -m pip install -r tools/content-validation/requirements.txt
  Assert-LastExitCode 'Install validation requirements'

  if (Get-Command git -ErrorAction SilentlyContinue) {
    try {
      git lfs install --local | Out-Host
    } catch {
      Write-Warning 'Git LFS is not available in this shell. Binary asset support will need it later.'
    }
  }

  Write-Host 'Bootstrap complete.'
} finally {
  Pop-Location
}
