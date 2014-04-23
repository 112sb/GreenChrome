#define UNICODE
#define _UNICODE

#include "GreenChrome.h"

EXTERNC BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID pv)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		LoadSysDll();//����ϵͳdllԭ�й���

		//exeȫ·��
		wchar_t fullPath[MAX_PATH];
		GetModuleFileName(NULL, fullPath, MAX_PATH);

		//exe����·��
		wchar_t exePath[MAX_PATH];
		wcscpy(exePath, fullPath);
		PathRemoveFileSpec(exePath);

		//ini·��
		wchar_t iniPath[MAX_PATH];
		wcscpy(iniPath, exePath);
		wcscat(iniPath, L"\\GreenChrome.ini");

		// ����Ĭ��ini�ļ�
		if(!PathFileExists(iniPath))
		{
			FILE *fp = _wfopen(iniPath, L"wb");
			if(fp)
			{
				fwrite(DefaultConfig, sizeof(DefaultConfig), 1, fp);
				fclose(fp);
			}
		}

		//�Զ���������pin�Ŀ�ݷ�ʽ����ֻ������
		AutoLockLnk(fullPath);

		//����������
		wchar_t *command = GetCommandLine();

		wchar_t parentPath[MAX_PATH];
		if(GetParentPath(parentPath) && _wcsicmp(parentPath, fullPath)!=0) //�����̲���Chrome
		{
			wchar_t *MyCommandLine = (wchar_t *)malloc(1024*20);
			int i;
			int nArgs;
			LPWSTR *szArglist = CommandLineToArgvW(command, &nArgs);
			for(i=0; i<nArgs; i++)
			{
				if(i==0) //�ڽ���·������׷�Ӳ���
				{
					wcscpy(MyCommandLine, L"\"");
					wcscat(MyCommandLine, szArglist[i]);
					wcscat(MyCommandLine, L"\"");

					wchar_t *AddList = (wchar_t *)malloc(1024*20);
					wchar_t *temp = (wchar_t *)malloc(1024*20);
					int AddListLen;
					AddListLen = GetPrivateProfileSection(L"׷�Ӳ���", AddList, 1024*20, iniPath);

					StringSplit CommandList(AddList,0,AddListLen);
					for(int j=0;j<CommandList.GetCount();j++)
					{
						StringSplit add_cmd(CommandList.GetIndex(j),'=');
						if(add_cmd.GetCount()==2) // xxx=bbb
						{
							if( !wcsstr(command,add_cmd.GetIndex(0)) )
							{
								wcscat(MyCommandLine, L" ");
								wcscat(MyCommandLine, add_cmd.GetIndex(0));
								wcscat(MyCommandLine, L"=");

								ExpandEnvironmentStrings(add_cmd.GetIndex(1), temp, 1024*20);
								wchar_t *_temp = replace(temp, L"%app%", exePath);
								if(wcschr(_temp,' ') && _temp[0]!='\"')
								{
									wcscat(MyCommandLine, L"\"");
									wcscat(MyCommandLine, _temp);
									wcscat(MyCommandLine, L"\"");
								}
								else
								{
									wcscat(MyCommandLine, _temp);
								}
								free(_temp);
							}
						}
						else //�����Ⱥ�  xxxxx
						{
							if( !wcsstr(command,CommandList.GetIndex(j)) )
							{
								wcscat(MyCommandLine, L" ");
								if(wcschr(CommandList.GetIndex(j),' ') && CommandList.GetIndex(j)[0]!='\"')
								{
									wcscat(MyCommandLine, L"\"");
									wcscat(MyCommandLine, CommandList.GetIndex(j));
									wcscat(MyCommandLine, L"\"");

								}
								else
								{
									wcscat(MyCommandLine, CommandList.GetIndex(j));
								}
							}
						}
					}
					free(temp);
					free(AddList);
				}
				else
				{
					// ԭ��ƴ�Ӳ���
					wcscat(MyCommandLine, L" \"");
					wcscat(MyCommandLine, szArglist[i]);
					wcscat(MyCommandLine, L"\"");
				}
			}
			LocalFree(szArglist);

			//��������
			STARTUPINFO si = {0};
			PROCESS_INFORMATION pi = {0};
			si.cb = sizeof(STARTUPINFO);
			//GetStartupInfo(&si);

			//OutputDebugStringW(MyCommandLine);

			if (CreateProcess(fullPath, MyCommandLine, NULL, NULL, false, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE, NULL, 0, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				ExitProcess(NULL);
			}
		}
	}
	return TRUE;
}
