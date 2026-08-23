$ErrorActionPreference = 'SilentlyContinue'
if (!(Test-Path "$env:TEMP\enginotechc")) { New-Item -ItemType Directory -Path "$env:TEMP\enginotechc" -Force | Out-Null }
Set-Location "c:\Users\asus_\Downloads\EngineSoft\enginotech-cpp\examples\http-server"
$result = & "C:\msys64\usr\bin\ecn.exe" "c:\Users\asus_\Downloads\EngineSoft\enginotech-cpp\examples\http-server\main.ec" -transpile 2>&1
if ($LASTEXITCODE -ne 0) { $result; exit 1 }
$content = $result -join [Environment]::NewLine
$headers = @("#include <arpa/inet.h>","#include <dirent.h>","#include <netdb.h>","#include <pthread.h>","#include <sys/socket.h>","#include <sys/stat.h>","#include <sys/wait.h>","#include <unistd.h>","#include <strings.h>")
foreach ($h in $headers) { $content = $content.Replace($h, ("// " + $h + " // Removed for Windows")) }
$ecSrc = Get-Content "c:\Users\asus_\Downloads\EngineSoft\enginotech-cpp\examples\http-server\main.ec" -Raw
$sysCalls = [regex]::Matches($ecSrc, 'system("([^"]+)")')
if ($sysCalls.Count -gt 0) {
  $inj = ""
  foreach ($m in $sysCalls) { $inj += '    system("' + $m.Groups[1].Value + '");' + [Environment]::NewLine }
  $content = $content -replace '([}]s*)$', "$inj" + '$1'
  $content = $content -replace '(#include <stdio.h>)', '$1' + [Environment]::NewLine + '#include <stdlib.h>'
}
[System.IO.File]::WriteAllText("$env:TEMP\enginotechc\main_temp.c", $content, [System.Text.UTF8Encoding]::new($false))
$env:Path = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:Path"
& "C:\msys64\mingw64\bin\gcc.exe" "$env:TEMP\enginotechc\main_temp.c" -o "$env:TEMP\enginotechc\main.exe" -lm -mconsole 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Remove-Item "$env:TEMP\enginotechc\main_temp.c" -ErrorAction SilentlyContinue; exit 1 }
Write-Host "---" -ForegroundColor Cyan
& "$env:TEMP\enginotechc\main.exe"
Write-Host "---" -ForegroundColor Cyan
Remove-Item "$env:TEMP\enginotechc\main_temp.c" -ErrorAction SilentlyContinue
Remove-Item "$env:TEMP\enginotechc\main.exe" -ErrorAction SilentlyContinue
Remove-Item "c:\Users\asus_\Downloads\EngineSoft\enginotech-cpp\examples\http-server\_run_temp.ps1" -ErrorAction SilentlyContinue