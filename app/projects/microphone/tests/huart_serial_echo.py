#!/usr/bin/env python3
"""Send a raw .bin file over UART and compare the echoed bytes."""

import argparse
import os
import time

import serial


def parse_args():
    parser = argparse.ArgumentParser(description="HSUART raw serial echo test")
    parser.add_argument("port", help="PC serial port, e.g. COM9")
    parser.add_argument("baud", type=int, help="UART baud rate, e.g. 921600")
    parser.add_argument("input_file", help="Input .bin file to send")
    parser.add_argument("--recv", help="Optional received output file path")
    parser.add_argument("--chunk-size", type=int, default=4096,
                        help="Send chunk size in bytes")
    parser.add_argument("--send-delay-ms", type=float, default=0,
                        help="Delay after each sent chunk, in milliseconds")
    return parser.parse_args()


def analyze_result(send_data, recv_data):
    common_len = min(len(send_data), len(recv_data))
    lost_bytes = max(0, len(send_data) - len(recv_data))
    error_bytes = 0

    if len(set(send_data)) == 1:
        expected = send_data[0]
        for i in range(common_len):
            if recv_data[i] != expected:
                error_bytes += 1
    else:
        for i in range(1, common_len):
            if recv_data[i] != ((recv_data[i - 1] + 1) & 0xFF):
                error_bytes += 1

    return lost_bytes, error_bytes


def main():
    args = parse_args()

    with open(args.input_file, "rb") as f:
        send_data = f.read()

    recv_path = args.recv or (
        os.path.splitext(args.input_file)[0] + "_serial_recv.bin"
    )

    ser = serial.Serial(
        port=args.port,
        baudrate=args.baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.05,
        write_timeout=5,
    )

    try:
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        print("sending", len(send_data), "bytes...")
        chunk_size = args.chunk_size
        for offset in range(0, len(send_data), chunk_size):
            ser.write(send_data[offset:offset + chunk_size])
            ser.flush()
            if args.send_delay_ms > 0:
                time.sleep(args.send_delay_ms / 1000.0)

        recv_data = bytearray()
        last_data_time = time.time()

        while len(recv_data) < len(send_data):
            remaining = len(send_data) - len(recv_data)
            available = ser.in_waiting
            if available:
                chunk = ser.read(min(available, remaining))
                if chunk:
                    recv_data.extend(chunk)
                    last_data_time = time.time()
            else:
                if time.time() - last_data_time > 3:
                    print("receive timeout")
                    break
                time.sleep(0.001)

        with open(recv_path, "wb") as f:
            f.write(recv_data)

        print("sent bytes:", len(send_data))
        print("recv bytes:", len(recv_data))

        if send_data == recv_data:
            print("RESULT: PASS")
        else:
            lost_bytes, error_bytes = analyze_result(send_data, recv_data)
            print("lost bytes:", lost_bytes)
            print("error bytes:", error_bytes)
            print("RESULT: FAIL")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
