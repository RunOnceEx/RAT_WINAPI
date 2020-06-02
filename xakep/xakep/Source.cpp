#include "winsock2.h"
#include "intrin.h"
#include "ntdll.h"
#include "iphlpapi.h"
#include "stdio.h"
#include <string>
#include <iostream>

#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

using namespace std;

SOCKET Socket;
SOCKADDR_IN ServerAddr;

//��������� �������� ��������� �� �������
struct CLIENTS
{
	SOCKET NUMBER; //��� ���� �� ��������� �� �����, ��� ��� ��� �� �����
	char USERNAME[100]; //���� � ������� �� ��������� ��� �����
	char OSVER[100]; //���� � ������� �� ��������� ������ ��
	char HWID[100]; //���������� ���� ��� ����� �����
};

struct DLE
{
	char URL[100];
};

//��������� �������� ��������� �� �������
struct CMDiDATA
{
	DWORD CMD; //��� � ��� ����� ������ �������
	char DATA[1000]; //� ��� ������ ������� �� ����� ������� ��� ��������� �������
};

DWORD WINAPI RecvThread(LPVOID param)
{
	while (TRUE)
	{
		CMDiDATA indata; //������� ��������� ���������� �� ��������
		memset(&indata, 0, sizeof(CMDiDATA)); //�������� 
		if (sizeof(CMDiDATA) == recv(Socket, (char*)&indata, sizeof(CMDiDATA), 0)); //����� ��������� �� �������
		{
			switch (indata.CMD)
			{
				case 1://���� indata.cmd == 1
				{
					DLE dle; //�������������� ��������� DLE
					memset(&dle, 0, sizeof(dle)); //������� ��
					memcpy(&dle, indata.DATA, sizeof(indata.DATA)); //� �������� � ��� indata.DATA
					break;
				}
				default:
					break;
			}
		}
	}
}

DWORD WINAPI ConsoleReader(LPVOID param)
{
	while (TRUE)
	{
		string line = { 0 }; //�������������� ����� ������
		getline(cin, line); //��������� ������� � ���� ����� �� �������
		char* mycmd = strdup(line.c_str()); //������������ ��� ������ � ����, ������� ����� �_str

		if (mycmd[0] != 0)
		{
			int i, j;
			char CMDCHAR[sizeof(int)];
			memset(CMDCHAR, 0, sizeof(CMDCHAR));
			int CMD = 0;
			for (i = 0, j = 0; ; i++, j++)
			{
				if (mycmd[i] == ':')
				{
					break;
				}
				CMDCHAR[j] = mycmd[i];
			}
			CMD = atoi(CMDCHAR);
			printf("CMD: %d\n", CMD);
			
			int x, y;
			char USERNAME[100];
			memset(USERNAME, 0, sizeof(USERNAME));
			for (x = i + 1, y = 0; ; x++, y++)
			{
				if (mycmd[x] == ':')
				{
					break;
				}
				USERNAME[y] = mycmd[x];
			}
			printf("USERNAME: %s\n", USERNAME);

			int a, b;
			char OSVER[100];
			memset(OSVER, 0, sizeof(OSVER));
			for (a = x + 1, b = 0; ; a++, b++)
			{
				if (mycmd[a] == ':')
				{
					break;
				}
				OSVER[b] = mycmd[a];
			}
			printf("OSVER: %s\n", OSVER);

			int n, m;
			char HWID[100];
			memset(HWID, 0, sizeof(HWID));
			for (n = a + 1, m = 0; ; n++, m++)
			{
				if (mycmd[n] == ':')
				{
					break;
				}
				HWID[m] = mycmd[n];
			}
			printf("HWID: %s\n", HWID);

			if (connect(Socket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == 0)
			{
				//���� �� ������������ �� �������� ��������:)
				CMDiDATA indata; //������� ��������� ���������� �� ��������
				CLIENTS client; //������� ��������� ���������� �� ���� � �������
				memset(&indata, 0, sizeof(CMDiDATA)); //�������� 
				memset(&client, 0, sizeof(CLIENTS)); //��������
				memcpy(client.HWID, HWID, sizeof(client.HWID));
				memcpy(client.USERNAME, USERNAME, sizeof(client.USERNAME));
				memcpy(client.OSVER, OSVER, sizeof(client.OSVER));
				indata.CMD = CMD;
				memcpy(indata.DATA, &client, sizeof(CLIENTS));
				send(Socket, (char*)&indata, sizeof(CMDiDATA), 0);
			}
			closesocket(Socket);
		}
	}
}

//������� ����� ����� ������ �������
int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitleA("XAKEP");

	//�������������� ��������� �������
	struct WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	WSAStartup(DLLVersion, &wsaData);

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAddr.sin_family = AF_INET;
	//������ ������ �� ��������� � �������������� �����
	ServerAddr.sin_port = htons(80);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//������������ � �������!!!

	HANDLE RECV[2];
	RECV[0] = CreateThread(NULL, 0, RecvThread, NULL, 0, 0); //������� ���� ������� ����� ���������� ��������� ��������� �� �������
	RECV[1] = CreateThread(NULL, 0, ConsoleReader, NULL, 0, 0);
	WaitForMultipleObjects(2, RECV, TRUE, INFINITE); //���������� ������� ���� ���� ���� ��������...
}
