param(
  [ValidateSet('check-budgets','run-soak','verify-saves','verify-profile','repair-profile','diff-save-revisions','export-qa-matrix','summarize-playtest','gate-rc')]
  [string]$Command = 'check-budgets',
  [Parameter(ValueFromRemainingArguments = $true)]
  [string[]]$ExtraArgs = @()
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

$repoRoot = Get-RepoRoot
$python = Get-VenvPython
Push-Location $repoRoot

try {
  & $python 'tools/quality/peter_quality.py' $Command @ExtraArgs
} finally {
  Pop-Location
}
