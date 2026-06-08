# ⚠️ WARNING: This repository is for educational and authorized security research ONLY.

**Unauthorized use of this tool to access computer systems without explicit consent is illegal and unethical. The author assumes no liability for misuse.**

# Windows AV Bypass & Reverse Shell (C++ / Python)

This project demonstrates a simple client-server remote access tool for Windows. It is intended for authorized penetration testing, CTF challenges, and educational study of defensive evasion techniques.

The client (C++) connects back to a listener (Python), sending XOR-encrypted commands and receiving encrypted output to help evade basic static signature detection.

## ⚙️ How It Works

- **Client (`pay.cpp` / `pypay.cpp` / `payd1.exe`)**: A Windows executable that hides its console window, connects to a specified IP/Port, receives XOR-encrypted and hex-encoded commands, executes them via `cmd.exe /c`, and returns the encrypted output.
- **Listener (`pythonlis.py`)**: A Python script that listens for incoming connections, sends XOR-encrypted commands, and decrypts the output from the client.
- **Evasion**: XOR obfuscation (key: `0x3A`) and hex encoding are used on network traffic to bypass simple static signature checks.

## 📋 Prerequisites

- **Linux (for cross-compilation)**: `mingw-w64-g++` package.
    ```bash
    sudo apt update && sudo apt install g++-mingw-w64-x86-64

    Windows Target: The compiled .exe runs on any modern Windows system.

    Attacker Machine: Python 3.x must be installed.

🛠️ Compilation (Linux -> Windows)

The source file is named pay.cpp (or pypay.cpp as in your example). Use mingw-w64 to compile it to a Windows executable.
'''bash
Basic Compilation:
bash

x86_64-w64-mingw32-g++ pay.cpp -o payd1.exe -lws2_32 -static -s -Os
'''
This command:

    -lws2_32: Links the Windows Socket library.

    -static: Statically links the C++ runtime (no extra DLLs needed).

    -s: Strips debug symbols (reduces file size).

    -Os: Optimizes for size.

To match your provided example exactly (renaming the source file):
bash

x86_64-w64-mingw32-g++ pypay.cpp -o noteexxxx.exe -lws2_32 -static -s -Os

    Note: The original repository contains pay.cpp. Rename it to pypay.cpp or adjust the compilation command accordingly.

🚀 Usage
1. Modify the Target IP/Port (Optional)

Edit the hardcoded IP and port in pay.cpp (lines 26-27):
cpp

const char* host = "172.16.113.1";  // Change to your listener IP
const int port = 9999;

2. Start the Listener (Attacker Machine)

Run the Python listener on your machine (the IP that the client will connect to).
bash

python3 pythonlis.py

The script listens on 0.0.0.0:9999 by default and displays a CMD> prompt once a client connects.
3. Execute the Client (Target Machine)

Run the compiled .exe on the target Windows machine. The console will be hidden. It will immediately attempt to connect to the listener.

Example Workflow:

    Listener (Attacker): Shows [+] Listening on 0.0.0.0:9999...

    Target: Executes payd1.exe.

    Listener: Receives connection: [+] Connection from 192.168.1.10:54321

    Listener (Attacker): Type a command (e.g., whoami, ipconfig).

    Result: The command output is displayed on the attacker's terminal.

📁 Repository Files

    char/pay.cpp : The main C++ client source code.

    char/payd1.exe : A pre-compiled example binary.

    char/pythonlis.py : The Python listener script.

🔧 Customization

    XOR Key: Change the #define KEY 0x3A in pay.cpp and the KEY = 0x3A variable in pythonlis.py to any other value (must match on both sides).

    Persistence: Add registry key modifications to pay.cpp for automatic startup (use with caution).

    Command Shell: The code currently uses cmd.exe /c. You can modify RunCommand() to use powershell -Command if needed.

📝 Technical Details

    RunCommand(cmd, output, output_size): Executes a command, captures both stdout and stderr, and stores the result.

    XOR(data, data_len): Applies XOR with the fixed key to obfuscate data.

    HexEncode() / HexDecode(): Converts binary data to a hexadecimal string representation for safe transmission over sockets.

    Connection Loop: The client will continuously retry to connect if the connection is lost (sleeps for 10 seconds between attempts).

⚖️ Legal & Ethics Disclaimer

This software is provided SOLELY FOR EDUCATIONAL PURPOSES AND AUTHORIZED SECURITY TESTING.

You must not use this tool to:

    Access computer systems without explicit permission.

    Violate any applicable laws or regulations.

    Bypass security controls on systems you do not own or have written authorization to test.

The author and contributors are not responsible for any misuse or damage caused by this software. By using this code, you agree that you are solely responsible for your actions.
🧑‍💻 Author

Demonjokerwarrior

    GitHub: @Demonjokerwarrior

This README was generated based on the source code and user instructions.
