<#
.SYNOPSIS
    EnginotechC++ Build Script for Windows (PowerShell)

.DESCRIPTION
    Cross-platform build script with parallel compilation, incremental builds,
    debug/release modes, and config file support.

.PARAMETER Target
    Build target: all, compiler, stdlib, tools, test, clean, install

.PARAMETER BuildType
    Build configuration: Debug, Release, RelWithDebInfo

.PARAMETER Jobs
    Number of parallel jobs (0 = auto-detect)

.PARAMETER ConfigFile
    Path to build config file

.PARAMETER OutDir
    Output directory

.PARAMETER Verbose
    Enable verbose output

.PARAMETER Rebuild
    Force clean rebuild

.EXAMPLE
    .\scripts\build.ps1                    # Release build
    .\scripts\build.ps1 -BuildType Debug   # Debug build
    .\scripts\build.ps1 -Jobs 8            # 8 parallel jobs
    .\scripts\build.ps1 -Target test       # Build and run tests
    .\scripts\build.ps1 -Clean             # Clean rebuild
#>

param(
    [ValidateSet('all','compiler','stdlib','tools','test','clean','install')]
    [string]$Target = 'all',
    
    [ValidateSet('Debug','Release','RelWithDebInfo')]
    [string]$BuildType = 'Release',
    
    [int]$Jobs = 0,
    
    [string]$ConfigFile = 'build.config',
    
    [string]$OutDir = '.build',
    
    [switch]$Verbose,
    
    [switch]$Rebuild,
    
    [switch]$Clean
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$projectRoot = Join-Path $scriptDir '..'
Set-Location $projectRoot

# Colors
$RED = "`e[0;31m"
$GREEN = "`e[0;32m"
$YELLOW = "`e[1;33m"
$BLUE = "`e[0;34m"
$NC = "`e[0m"

function Log { Write-Host "${BLUE}[BUILD]${NC} $args" }
function Warn { Write-Host "${YELLOW}[WARN]${NC} $args" }
function Err { Write-Host "${RED}[ERROR]${NC} $args" -ForegroundColor Red }
function Success { Write-Host "${GREEN}[OK]${NC} $args" }

# Detect CPU cores
if ($Jobs -eq 0) {
    $Jobs = [Environment]::ProcessorCount
}

# Version
$version = Get-Content VERSION -ErrorAction SilentlyContinue | Where-Object { $_ -match '^\d' } | Select-Object -First 1
if (-not $version) { $version = 'dev' }
$version = $version.Trim()

$defs = "-DENG_VERSION=`"$version`" -DBUILD_TYPE=`"$BuildType`""

# Compiler detection
$CXX = $env:CXX
if (-not $CXX) {
    $candidates = @('clang++', 'g++', 'zig c++', 'cl.exe')
    foreach ($c in $candidates) {
        if (Get-Command $c.Split(' ')[0] -ErrorAction SilentlyContinue) {
            $CXX = $c
            break
        }
    }
}
if (-not $CXX) {
    Err "No C++ compiler found. Install clang++, g++, zig, or Visual Studio."
    exit 1
}
Log "Using compiler: $CXX"

# Compile flags
switch ($BuildType) {
    'Debug' { $cxxFlags = "-std=c++20 -g -O0 -Wall -Wextra -DDEBUG" }
    'Release' { $cxxFlags = "-std=c++20 -O3 -DNDEBUG -march=native" }
    'RelWithDebInfo' { $cxxFlags = "-std=c++20 -O2 -g -DNDEBUG" }
}
$cxxFlags += " $($env:CXXFLAGS)"

$ldFlags = $env:LDFLAGS

# Load config file
if (Test-Path $ConfigFile) {
    Log "Loading config: $ConfigFile"
    $config = Get-Content $ConfigFile -Raw
    # Simple config parser (key=value)
    foreach ($line in $config -split "`n") {
        $line = $line.Trim()
        if ($line -and -not $line.StartsWith('#')) {
            $parts = $line -split '=', 2
            if ($parts.Count -eq 2) {
                $env:$($parts[0].Trim()) = $parts[1].Trim().Trim('"')
            }
        }
    }
}

# Module definitions
$modules = @(
    @{ Name='compiler'; Dirs='compiler'; Exclude='tests' },
    @{ Name='std'; Dirs='std'; Exclude='tests' },
    @{ Name='runtime'; Dirs='runtime'; Exclude='tests' },
    @{ Name='build-system'; Dirs='build-system'; Exclude='tests' },
    @{ Name='formatter'; Dirs='formatter'; Exclude='tests' },
    @{ Name='linter'; Dirs='linter'; Exclude='tests' },
    @{ Name='langserver'; Dirs='langserver'; Exclude='tests' },
    @{ Name='packages'; Dirs='packages'; Exclude='tests' }
)

function Collect-Sources($dirs, $exclude) {
    $sources = @()
    foreach ($d in $dirs -split ' ') {
        $sources += Get-ChildItem -Path $d -Recurse -Filter '*.cpp' -ErrorAction SilentlyContinue |
            Where-Object { $exclude -eq '' -or $_.FullName -notmatch "\\$exclude\\" } |
            Select-Object -ExpandProperty FullName
    }
    return $sources | Sort-Object
}

function Build-Object($src, $obj, $deps) {
    $needsRebuild = $Rebuild -or (Test-Path $obj -not) -or ($src.LastWriteTime -gt $obj.LastWriteTime)
    if ($needsRebuild -and (Test-Path $deps)) {
        $depContent = Get-Content $deps -Raw
        foreach ($dep in $depContent -split "`n") {
            $dep = $dep.Trim()
            if ($dep -and (Test-Path $dep) -and $dep.LastWriteTime -gt $obj.LastWriteTime) {
                $needsRebuild = $true
                break
            }
        }
    }
    
    if ($needsRebuild) {
        $cmd = "$CXX $cxxFlags $defs -MMD -MP -MF `"$deps`" -c `"$src`" -o `"$obj`""
        if ($Verbose) { Log "  $cmd" }
        $result = Invoke-Expression $cmd
        if ($LASTEXITCODE -ne 0) {
            Err "Failed to compile: $src"
            exit 1
        }
        return 0
    }
    return 2
}

function Build-Module($module) {
    Log "Building module: $($module.Name)"
    $sources = Collect-Sources $module.Dirs $module.Exclude
    if (-not $sources) {
        Warn "No sources for $($module.Name)"
        return @()
    }
    
    $objDir = Join-Path $OutDir "obj/$($module.Name)"
    New-Item -ItemType Directory -Path $objDir -Force | Out-Null
    
    $results = @()
    $changed = 0
    $upToDate = 0
    
    # Parallel compilation using jobs
    $scriptBlock = {
        param($src, $objDir, $cxx, $cxxFlags, $defs, $Verbose)
        $obj = Join-Path $objDir "$($src.Replace('\', '_').Replace('/', '_')).o"
        $deps = "$obj.d"
        $result = Build-Object $src $obj $deps
        return "$result:$obj"
    }
    
    $jobs = @()
    foreach ($src in $sources) {
        $jobs += Start-Job -ScriptBlock $scriptBlock -ArgumentList $src, $objDir, $CXX, $cxxFlags, $defs, $Verbose
        while ($jobs.Count -ge $Jobs) {
            $done = $jobs | Where-Object { $_.State -ne 'Running' }
            if ($done) {
                $jobs = $jobs | Where-Object { $_.State -eq 'Running' }
            } else {
                Start-Sleep -Milliseconds 100
            }
        }
    }
    
    # Wait for remaining jobs
    $jobs | Wait-Job | Out-Null
    foreach ($job in $jobs) {
        $result = $job | Receive-Job
        $job | Remove-Job
        $parts = $result -split ':', 2
        switch ($parts[0]) {
            '0' { $changed++; $results += $parts[1] }
            '2' { $upToDate++; $results += $parts[1] }
            default { Err "Build failed"; exit 1 }
        }
    }
    
    Log "$($module.Name): $changed compiled, $upToDate up to date"
    return $results
}

function Link-Executable($name, $objs) {
    Log "Linking: $name"
    $output = Join-Path $OutDir $name
    $objList = $objs -join ' '
    $cmd = "$CXX $ldFlags $objList -o `"$output`""
    if ($Verbose) { Log "  $cmd" }
    Invoke-Expression $cmd
    if ($LASTEXITCODE -ne 0) {
        Err "Link failed: $name"
        exit 1
    }
    Success "Created: $output"
}

function Build-All {
    $allObjs = @()
    foreach ($module in $modules) {
        $objs = Build-Module $module
        $allObjs += $objs
    }
    Link-Executable "engc.exe" $allObjs
}

function Run-Tests {
    $engc = Join-Path $OutDir "engc.exe"
    if (-not (Test-Path $engc)) {
        Err "Compiler not built"
        exit 1
    }
    & $engc test
}

function Clean-Build {
    Log "Cleaning build artifacts..."
    if (Test-Path $OutDir) { Remove-Item $OutDir -Recurse -Force }
    Success "Clean complete"
}

function Install-Build {
    if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        Err "Install requires Administrator privileges"
        exit 1
    }
    Log "Installing..."
    $engc = Join-Path $OutDir "engc.exe"
    Copy-Item $engc "C:\Program Files\Enginotech\engc.exe" -Force
    # Install stdlib
    $stdlibDest = "C:\Program Files\Enginotech\std"
    if (Test-Path $stdlibDest) { Remove-Item $stdlibDest -Recurse -Force }
    Copy-Item "std" $stdlibDest -Recurse
    Success "Installed to C:\Program Files\Enginotech"
}

# Main
if ($Clean -or $Rebuild) { Clean-Build }
if ($Target -eq 'clean') { exit 0 }

switch ($Target) {
    'test' { Build-All; Run-Tests }
    'install' { Build-All; Install-Build }
    default { Build-All }
}

Success "Build complete: $OutDir\engc.exe (v$version)"