#Requires -Version 5.1
param(
    [string]$Filter = "",
    [switch]$Verbose,
    [switch]$List,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

$ProjectDir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildDir   = Join-Path $ProjectDir "build"
$TestDir    = Join-Path $BuildDir "tests"

if ($Help) {
    @"
Usage: run_tests.ps1 [options]

Options:
  -Filter PATTERN     GoogleTest filter (e.g. "TimerServiceTest.*")
  -Verbose            Show individual test output
  -List               List available tests without running
  -Help               Show this help
"@
    exit 0
}

# Find test executables under build/tests (any config subdirectory for multi-config generators)
$searchPaths = @($TestDir)
foreach ($cfg in @("Debug", "Release", "RelWithDebInfo", "MinSizeRel")) {
    $cfgDir = Join-Path $TestDir $cfg
    if (Test-Path $cfgDir) { $searchPaths += $cfgDir }
}

$testBins = @()
foreach ($dir in $searchPaths) {
    if (Test-Path $dir) {
        $testBins += Get-ChildItem -Path $dir -Filter "test_*.exe" -File -ErrorAction SilentlyContinue
    }
}

if ($testBins.Count -eq 0) {
    Write-Error "Error: No test executables found under $TestDir`nRun build.ps1 with tests enabled first."
    exit 1
}

# List mode
if ($List) {
    foreach ($bin in $testBins) {
        Write-Host "--- $($bin.Name) ---"
        & $bin.FullName --gtest_list_tests
    }
    exit 0
}

# Run tests
$failed = 0
foreach ($bin in $testBins) {
    Write-Host "==> Running $($bin.Name)"

    $args = @()
    if ($Filter)  { $args += "--gtest_filter=$Filter" }
    if ($Verbose) { $args += "--gtest_print_time=1" }

    & $bin.FullName @args
    if ($LASTEXITCODE -ne 0) {
        Write-Host "    FAILED"
        $failed++
    } else {
        Write-Host "    PASSED"
    }
    Write-Host ""
}

if ($failed -gt 0) {
    Write-Host "==> $failed test suite(s) FAILED"
    exit 1
} else {
    Write-Host "==> All test suites PASSED"
}
