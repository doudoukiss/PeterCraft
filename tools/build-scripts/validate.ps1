param(
  [ValidateSet('full','files','changed')]
  [string]$Mode = 'full',
  [string]$BaseRef = 'origin/main',
  [string[]]$Files = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
$python = Get-VenvPython
Push-Location $repoRoot

try {
  $arguments = @('tools/content-validation/peter_validate.py', $Mode, '--include-lint')
  if ($Mode -eq 'changed') {
    $arguments += @('--base-ref', $BaseRef)
  } elseif ($Mode -eq 'files') {
    $arguments += $Files
  }

  & $python @arguments
} finally {
  Pop-Location
}
