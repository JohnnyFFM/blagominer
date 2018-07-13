#include "stdafx.h"
#include "logger.h"
FILE * fp_Log = nullptr;
bool use_log = true;

void Log_init(void)
{
	if (use_log)
	{
		std::stringstream ss;
		if (CreateDirectory(L"Logs", nullptr) == ERROR_PATH_NOT_FOUND)
		{
			bm_wattron(12);
			bm_wprintw("CreateDirectory failed (%d)\n", GetLastError(), 0);
			bm_wattroff(12);
			use_log = false;
			return;
		}
		GetLocalTime(&cur_time);
		ss << "Logs\\" << cur_time.wYear << "-" << cur_time.wMonth << "-" << cur_time.wDay << "_" << cur_time.wHour << "_" << cur_time.wMinute << "_" << cur_time.wSecond << ".log";
		std::string filename = ss.str();
		if ((fp_Log = _fsopen(filename.c_str(), "wt", _SH_DENYNO)) == NULL)
		{
			bm_wattron(12);
			bm_wprintw("LOG: file openinig error\n", 0);
			bm_wattroff(12);
			use_log = false;
		}
		Log(version);
	}
}

void Log(char const *const strLog)
{
	if (use_log)
	{
		// если строка содержит интер, то добавить время  
		if (strLog[0] == '\n')
		{
			GetLocalTime(&cur_time);
			fprintf_s(fp_Log, "\n%02d:%02d:%02d %s", cur_time.wHour, cur_time.wMinute, cur_time.wSecond, strLog + 1);
		}
		else fprintf_s(fp_Log, "%s", strLog);
		fflush(fp_Log);
	}
}

void Log_server(char const *const strLog)
{
	size_t len_str = strlen(strLog);
	if ((len_str> 0) && use_log)
	{
		char * Msg_log = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, len_str * 2 + 1);
		if (Msg_log == nullptr)	ShowMemErrorExit();

		for (size_t i = 0, j = 0; i<len_str; i++, j++)
		{
			if (strLog[i] == '\r')
			{
				Msg_log[j] = '\\';
				j++;
				Msg_log[j] = 'r';
			}
			else
				if (strLog[i] == '\n')
				{
					Msg_log[j] = '\\';
					j++;
					Msg_log[j] = 'n';
				}
				else
					if (strLog[i] == '%')
					{
						Msg_log[j] = '%';
						j++;
						Msg_log[j] = '%';
					}
					else Msg_log[j] = strLog[i];
		}

		fprintf_s(fp_Log, "%s", Msg_log);
		fflush(fp_Log);
		HeapFree(hHeap, 0, Msg_log);
	}
}

void Log_llu(unsigned long long const llu_num)
{
	if (use_log)
	{
		fprintf_s(fp_Log, "%llu", llu_num);
		fflush(fp_Log);
	}
}

void Log_u(size_t const u_num)
{
	if (use_log)
	{
		fprintf_s(fp_Log, "%u", (unsigned)u_num);
		fflush(fp_Log);
	}
}