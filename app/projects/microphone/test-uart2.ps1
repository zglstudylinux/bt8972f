<#
.SYNOPSIS
    验证 BT897x UART2 二进制 RX/TX 回显。

.EXAMPLE
    .\test-uart2.ps1 -Port COM7
    .\test-uart2.ps1 -Port COM7 -BaudRate 115200 -Case slow-special
    .\test-uart2.ps1 -ListPorts
#>
[CmdletBinding()]
param(
    [string]$Port,
    [int]$BaudRate = 115200,
    [ValidateSet('all', 'slow-special', 'paced-patterns', 'burst-lengths')]
    [string]$Case = 'all',
    [int]$TimeoutMs = 3000,
    [switch]$ListPorts
)

$ErrorActionPreference = 'Stop'

if ($ListPorts) {
    [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    exit 0
}

if ([string]::IsNullOrWhiteSpace($Port)) {
    throw '必须指定 -Port，例如 -Port COM7；可先用 -ListPorts 查看。'
}

function New-ByteArray([int[]]$Values) {
    $result = New-Object byte[] $Values.Count
    for ($i = 0; $i -lt $Values.Count; $i++) {
        $result[$i] = [byte]$Values[$i]
    }
    return $result
}

function Read-Exact([System.IO.Ports.SerialPort]$Serial, [int]$Count, [int]$DeadlineMs) {
    $result = New-Object byte[] $Count
    $offset = 0
    $timer = [System.Diagnostics.Stopwatch]::StartNew()

    while (($offset -lt $Count) -and ($timer.ElapsedMilliseconds -lt $DeadlineMs)) {
        $available = $Serial.BytesToRead
        if ($available -gt 0) {
            $readCount = [Math]::Min($available, $Count - $offset)
            $offset += $Serial.Read($result, $offset, $readCount)
        } else {
            Start-Sleep -Milliseconds 1
        }
    }

    return [pscustomobject]@{
        Bytes = $result[0..([Math]::Max(0, $offset - 1))]
        Count = $offset
    }
}

function Invoke-EchoCase(
    [System.IO.Ports.SerialPort]$Serial,
    [string]$Name,
    [byte[]]$Payload,
    [int]$InterByteDelayMs
) {
    $Serial.DiscardInBuffer()
    $Serial.DiscardOutBuffer()

    if ($InterByteDelayMs -gt 0) {
        foreach ($value in $Payload) {
            $one = New-Object byte[] 1
            $one[0] = $value
            $Serial.Write($one, 0, 1)
            Start-Sleep -Milliseconds $InterByteDelayMs
        }
    } else {
        $Serial.Write($Payload, 0, $Payload.Length)
    }

    $received = Read-Exact -Serial $Serial -Count $Payload.Length -DeadlineMs $TimeoutMs
    Start-Sleep -Milliseconds 50
    $extraCount = $Serial.BytesToRead
    $extra = New-Object byte[] $extraCount
    if ($extraCount -gt 0) {
        [void]$Serial.Read($extra, 0, $extraCount)
    }

    $firstMismatch = -1
    $compareCount = [Math]::Min($Payload.Length, $received.Count)
    for ($i = 0; $i -lt $compareCount; $i++) {
        if ($Payload[$i] -ne $received.Bytes[$i]) {
            $firstMismatch = $i
            break
        }
    }
    if (($firstMismatch -lt 0) -and ($received.Count -ne $Payload.Length)) {
        $firstMismatch = $compareCount
    }

    if ($firstMismatch -ge 0) {
        $expected = if ($firstMismatch -lt $Payload.Length) { '0x{0:X2}' -f $Payload[$firstMismatch] } else { '<none>' }
        $actual = if ($firstMismatch -lt $received.Count) { '0x{0:X2}' -f $received.Bytes[$firstMismatch] } else { '<timeout>' }
        Write-Host ("[FAIL] {0}: tx={1} rx={2} mismatch={3} expected={4} actual={5}" -f `
            $Name, $Payload.Length, $received.Count, $firstMismatch, $expected, $actual) -ForegroundColor Red
        return $false
    }
    if ($extraCount -gt 0) {
        Write-Host ("[FAIL] {0}: tx={1} rx={2}+{3} extra-first=0x{4:X2}" -f `
            $Name, $Payload.Length, $received.Count, $extraCount, $extra[0]) -ForegroundColor Red
        return $false
    }

    Write-Host ("[PASS] {0}: tx={1} rx={2} delay={3}ms" -f `
        $Name, $Payload.Length, $received.Count, $InterByteDelayMs) -ForegroundColor Green
    return $true
}

$tests = @()
if (($Case -eq 'all') -or ($Case -eq 'slow-special')) {
    $tests += @{ Name = 'single-00'; Data = New-ByteArray @(0x00); Delay = 100 }
    $tests += @{ Name = 'single-ff'; Data = New-ByteArray @(0xff); Delay = 100 }
    $tests += @{ Name = 'repeat-61'; Data = New-ByteArray @(0x61, 0x61, 0x61, 0x61); Delay = 100 }
    $tests += @{ Name = 'alternating-00-ff'; Data = New-ByteArray @(0x00, 0xff, 0x00, 0xff); Delay = 100 }
    $tests += @{ Name = 'repeat-55-aa'; Data = New-ByteArray @(0x55, 0x55, 0xaa, 0xaa); Delay = 100 }
}
if (($Case -eq 'all') -or ($Case -eq 'paced-patterns')) {
    $allBytes = New-Object byte[] 256
    for ($i = 0; $i -lt 256; $i++) { $allBytes[$i] = [byte]$i }
    $tests += @{ Name = 'all-bytes-10ms'; Data = $allBytes; Delay = 10 }
    $tests += @{ Name = 'all-bytes-1ms'; Data = $allBytes; Delay = 1 }
}
if (($Case -eq 'all') -or ($Case -eq 'burst-lengths')) {
    foreach ($length in @(128, 129, 255, 256, 257)) {
        $pattern = New-Object byte[] $length
        for ($i = 0; $i -lt $length; $i++) { $pattern[$i] = [byte](($i * 37 + 11) -band 0xff) }
        $tests += @{ Name = "burst-$length"; Data = $pattern; Delay = 0 }
    }
}

$serial = New-Object System.IO.Ports.SerialPort $Port, $BaudRate, 'None', 8, 'Two'
$serial.Handshake = 'None'
$serial.ReadTimeout = 100
$serial.WriteTimeout = $TimeoutMs
$serial.DtrEnable = $false
$serial.RtsEnable = $false

$failed = 0
try {
    $serial.Open()
    Start-Sleep -Milliseconds 300
    Write-Host "UART2 echo test: port=$Port baud=$BaudRate cases=$($tests.Count)"
    foreach ($test in $tests) {
        if (-not (Invoke-EchoCase -Serial $serial -Name $test.Name -Payload $test.Data -InterByteDelayMs $test.Delay)) {
            $failed++
        }
        Start-Sleep -Milliseconds 100
    }
} finally {
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}

if ($failed -gt 0) {
    throw "$failed UART2 test case(s) failed."
}
Write-Host 'All UART2 echo tests passed.' -ForegroundColor Green
