<#
.SYNOPSIS
    校验 UART2 独立 TX 测试帧。

.EXAMPLE
    .\test-uart2-tx.ps1 -Port COM17
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Port,
    [int]$BaudRate = 115200,
    [int]$FrameCount = 100,
    [int]$TimeoutMs = 20000
)

$ErrorActionPreference = 'Stop'
$FrameSize = 72
$PayloadSize = 64
$Magic = [byte[]](0x55, 0xaa, 0x5a, 0xa5)

function Get-Crc16([byte[]]$Buffer, [int]$Offset, [int]$Length) {
    [uint16]$crc = 0xffff
    for ($i = 0; $i -lt $Length; $i++) {
        $crc = [uint16]($crc -bxor $Buffer[$Offset + $i])
        for ($bit = 0; $bit -lt 8; $bit++) {
            if (($crc -band 1) -ne 0) {
                $crc = [uint16](($crc -shr 1) -bxor 0xa001)
            } else {
                $crc = [uint16]($crc -shr 1)
            }
        }
    }
    return $crc
}

function Find-Magic([System.Collections.Generic.List[byte]]$Buffer) {
    for ($i = 0; $i -le $Buffer.Count - $Magic.Length; $i++) {
        $matched = $true
        for ($j = 0; $j -lt $Magic.Length; $j++) {
            if ($Buffer[$i + $j] -ne $Magic[$j]) {
                $matched = $false
                break
            }
        }
        if ($matched) { return $i }
    }
    return -1
}

$serial = New-Object System.IO.Ports.SerialPort $Port, $BaudRate, 'None', 8, 'Two'
$serial.Handshake = 'None'
$serial.ReadTimeout = 100
$serial.DtrEnable = $false
$serial.RtsEnable = $false
$buffer = New-Object 'System.Collections.Generic.List[byte]'
$valid = 0
$crcErrors = 0
$payloadErrors = 0
$sequenceErrors = 0
$droppedBytes = 0
$lastSequence = $null
$firstSequence = $null
$timer = [System.Diagnostics.Stopwatch]::StartNew()

try {
    $serial.Open()
    $serial.DiscardInBuffer()
    Write-Host "UART2 TX test: port=$Port baud=$BaudRate format=8N2 target=$FrameCount"

    while (($valid -lt $FrameCount) -and ($timer.ElapsedMilliseconds -lt $TimeoutMs)) {
        $available = $serial.BytesToRead
        if ($available -eq 0) {
            Start-Sleep -Milliseconds 1
            continue
        }

        $chunk = New-Object byte[] $available
        [void]$serial.Read($chunk, 0, $available)
        $buffer.AddRange($chunk)

        while ($buffer.Count -ge $Magic.Length) {
            $start = Find-Magic $buffer
            if ($start -lt 0) {
                $keep = [Math]::Min($Magic.Length - 1, $buffer.Count)
                $droppedBytes += $buffer.Count - $keep
                if ($keep -gt 0) {
                    $tail = $buffer.GetRange($buffer.Count - $keep, $keep)
                    $buffer.Clear()
                    $buffer.AddRange($tail)
                } else {
                    $buffer.Clear()
                }
                break
            }
            if ($start -gt 0) {
                $droppedBytes += $start
                $buffer.RemoveRange(0, $start)
            }
            if ($buffer.Count -lt $FrameSize) { break }

            $frame = $buffer.GetRange(0, $FrameSize).ToArray()
            $buffer.RemoveRange(0, $FrameSize)
            $sequence = [int]$frame[4] -bor ([int]$frame[5] -shl 8)
            $expectedCrc = Get-Crc16 $frame 0 ($FrameSize - 2)
            $actualCrc = [int]$frame[$FrameSize - 2] -bor ([int]$frame[$FrameSize - 1] -shl 8)
            if ($actualCrc -ne $expectedCrc) {
                $crcErrors++
                continue
            }

            $payloadOk = $true
            for ($i = 0; $i -lt $PayloadSize; $i++) {
                if ($frame[6 + $i] -ne [byte](($sequence + $i) -band 0xff)) {
                    $payloadOk = $false
                    break
                }
            }
            if (-not $payloadOk) {
                $payloadErrors++
                continue
            }

            if ($null -eq $firstSequence) {
                $firstSequence = $sequence
            } elseif ($sequence -ne (($lastSequence + 1) -band 0xffff)) {
                $sequenceErrors++
                Write-Host ("[FAIL] sequence: previous={0} current={1}" -f $lastSequence, $sequence) -ForegroundColor Red
            }
            $lastSequence = $sequence
            $valid++
        }
    }
} finally {
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}

Write-Host ("result: valid={0}/{1} first={2} last={3} crc_errors={4} payload_errors={5} sequence_errors={6} discarded_bytes={7} elapsed_ms={8}" -f `
    $valid, $FrameCount, $firstSequence, $lastSequence, $crcErrors, $payloadErrors, $sequenceErrors, $droppedBytes, $timer.ElapsedMilliseconds)

if (($valid -ne $FrameCount) -or ($crcErrors -ne 0) -or ($payloadErrors -ne 0) -or
    ($sequenceErrors -ne 0) -or ($droppedBytes -ne 0)) {
    throw 'UART2 TX test failed.'
}

Write-Host 'UART2 TX test passed.' -ForegroundColor Green
