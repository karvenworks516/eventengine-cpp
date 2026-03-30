#Requires -Version 5.1
param(
    [switch]$Clean,
    [switch]$Force,
    [switch]$Debug,
    [switch]$NoStatic,
    [switch]$NoShared,
    [switch]$NoTests,
    [switch]$NoExamples,
    [switch]$TestsOnly,
    [int]$Jobs = (Get-CimInstance Win32_Processor | Measure-Object -Property NumberOfLogicalProcessors -Sum).Sum,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# --- Configuration ---
$LibuvVersion = "v1.52.1"

$ProjectDir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildDir   = Join-Path $ProjectDir "build"
$LibuvDir   = Join-Path $ProjectDir "third_party\libuv"
$BuildType  = if ($Debug) { "Debug" } else { "Release" }

$Static   = if ($NoStatic)   { "OFF" } else { "ON" }
$Shared   = if ($NoShared)   { "OFF" } else { "ON" }
$Tests    = if ($NoTests)    { "OFF" } else { "ON" }
$Examples = if ($NoExamples) { "OFF" } else { "ON" }

if ($TestsOnly) {
    $Shared = "OFF"; $Examples = "OFF"; $Tests = "ON"; $Static = "ON"
}

if ($Help) {
    @"
Usage: build.ps1 [options]

Options:
  -Clean              Remove build directory before building
  -Force              Full clean rebuild (removes build dir + resets libuv submodule)
  -Debug              Build in Debug mode (default: Release)
  -NoStatic           Skip static library
  -NoShared           Skip shared library
  -NoTests            Skip unit tests
  -NoExamples         Skip example apps
  -TestsOnly          Build only static lib + tests (no shared, no examples)
  -Jobs N             Parallel build jobs (default: $Jobs)
  -Help               Show this help

libuv version: $LibuvVersion
"@
    exit 0
}

# --- Submodule version check ---
function Sync-Libuv {
    if (-not (Test-Path (Join-Path $LibuvDir ".git"))) {
        Write-Host "==> libuv submodule not initialized, fetching..."
        git -C $ProjectDir submodule update --init --recursive
    }

    $currentTag = git -C $LibuvDir describe --tags --exact-match 2>$null
    if (-not $currentTag) { $currentTag = "" }

    if ($currentTag -eq $LibuvVersion) {
        Write-Host "==> libuv already at $LibuvVersion, skipping"
    } else {
        Write-Host "==> libuv version mismatch (current: $(if ($currentTag) { $currentTag } else { 'unknown' }), want: $LibuvVersion)"
        Write-Host "    Cleaning and re-syncing..."
        git -C $LibuvDir clean -fdx --quiet
        git -C $LibuvDir checkout . --quiet
        git -C $LibuvDir fetch --tags --quiet
        git -C $LibuvDir checkout $LibuvVersion --quiet
        Write-Host "    libuv pinned to $LibuvVersion"
        $script:Clean = $true
    }
}

# --- Force reset ---
if ($Force) {
    $Clean = $true
    Write-Host "==> Force rebuild: resetting libuv submodule"
    if (Test-Path $LibuvDir) { Remove-Item -Recurse -Force $LibuvDir }
    git -C $ProjectDir submodule update --init --recursive
}

Sync-Libuv

# --- Build ---
if ($Clean) {
    Write-Host "==> Cleaning build directory"
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
}

Write-Host "==> Configuring ($BuildType)"
Write-Host "    Static: $Static  Shared: $Shared  Tests: $Tests  Examples: $Examples"

cmake -B $BuildDir `
    -DCMAKE_BUILD_TYPE="$BuildType" `
    -DEVENTENGINE_BUILD_STATIC="$Static" `
    -DEVENTENGINE_BUILD_SHARED="$Shared" `
    -DEVENTENGINE_BUILD_TESTS="$Tests" `
    -DEVENTENGINE_BUILD_EXAMPLES="$Examples" `
    $ProjectDir

if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

Write-Host "==> Building (jobs: $Jobs)"
cmake --build $BuildDir --config $BuildType -j $Jobs

if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

Write-Host "==> Build complete"
Write-Host "    Artifacts in: $BuildDir"
