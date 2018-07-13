#include "stdafx.h"
#include "network.h"

int network_quality = 100;
size_t send_interval = 100;			// âðåìÿ îæèäàíèÿ ìåæäó îòïðàâêàìè
size_t update_interval = 1000;		// âðåìÿ îæèäàíèÿ ìåæäó àïäåéòàìè
std::string nodeaddr = "localhost";	// àäðåñ ïóëà
std::string nodeport = "8125";		// ïîðò ïóëà

std::string updateraddr = "localhost";// àäðåñ ïóëà
std::string updaterport = "8125";		// ïîðò ïóëà

std::string infoaddr = "localhost";	// àäðåñ ïóëà
std::string infoport = "8125";		// ïîðò ïóëà

std::string proxyport = "8125";		// ïîðò ïóëà
std::map <u_long, unsigned long long> satellite_size; // Ñòðóêòóðà ñ îáúåìàìè ïëîòîâ ñàòåëëèòîâ
std::thread showWinner;

char str_signature[65];

char* GetJSON(char const *const req) {
	const unsigned BUF_SIZE = 1024;

	char *buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, BUF_SIZE);
	if (buffer == nullptr) ShowMemErrorExit();

	char *find = nullptr;
	unsigned long long msg_len = 0;
	int iResult = 0;
	struct addrinfo *result = nullptr;
	struct addrinfo hints;
	SOCKET WalletSocket = INVALID_SOCKET;

	char *json = nullptr;

	RtlSecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(infoaddr.c_str(), infoport.c_str(), &hints, &result);
	if (iResult != 0) {
		bm_wattron(12);
		bm_wprintw("WINNER: Getaddrinfo failed with error: %d\n", iResult, 0);
		bm_wattroff(12);
		Log("\nWinner: getaddrinfo error");
	}
	else
	{
		WalletSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (WalletSocket == INVALID_SOCKET)
		{
			bm_wattron(12);
			bm_wprintw("WINNER: Socket function failed with error: %ld\n", WSAGetLastError(), 0);
			bm_wattroff(12);
			Log("\nWinner: Socket error: "); Log_u(WSAGetLastError());
		}
		else
		{
			unsigned t = 1000;
			setsockopt(WalletSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(unsigned));
			iResult = connect(WalletSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				bm_wattron(12);
				bm_wprintw("WINNER: Connect function failed with error: %ld\n", WSAGetLastError(), 0);
				bm_wattroff(12);
				Log("\nWinner: Connect server error "); Log_u(WSAGetLastError());
			}
			else
			{
				iResult = send(WalletSocket, req, (int)strlen(req), 0);
				if (iResult == SOCKET_ERROR)
				{
					bm_wattron(12);
					bm_wprintw("WINNER: Send request failed: %ld\n", WSAGetLastError(), 0);
					bm_wattroff(12);
					Log("\nWinner: Error sending request: "); Log_u(WSAGetLastError());
				}
				else
				{
					char *tmp_buffer;
					unsigned long long msg_len = 0;
					int iReceived_size = 0;
					while ((iReceived_size = recv(WalletSocket, buffer + msg_len, BUF_SIZE - 1, 0)) > 0)
					{
						msg_len = msg_len + iReceived_size;
						Log("\nrealloc: ");
						tmp_buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, msg_len + BUF_SIZE);
						if (tmp_buffer == nullptr) ShowMemErrorExit();
						memcpy(tmp_buffer, buffer, msg_len);
						HeapFree(hHeap, 0, buffer);
						buffer = tmp_buffer;
						buffer[msg_len + 1] = 0;
						Log_llu(msg_len);
					}

					if (iReceived_size < 0)
					{
						bm_wattron(12);
						bm_wprintw("WINNER: Get info failed: %ld\n", WSAGetLastError(), 0);
						bm_wattroff(12);
						Log("\nWinner: Error response: "); Log_u(WSAGetLastError());
					}
					else
					{
						find = strstr(buffer, "\r\n\r\n");
						if (find != nullptr)
						{
							json = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, msg_len);
							if (json == nullptr) ShowMemErrorExit();
							sprintf_s(json, HeapSize(hHeap, 0, json), "%s", find + 4 * sizeof(char));
						}
					} // recv() != SOCKET_ERROR
				} //send() != SOCKET_ERROR
			} // Connect() != SOCKET_ERROR
		} // socket() != INVALID_SOCKET
		iResult = closesocket(WalletSocket);
	} // getaddrinfo() == 0
	HeapFree(hHeap, 0, buffer);
	freeaddrinfo(result);
	return json;
}


void hostname_to_ip(char const *const  in_addr, char* out_addr)
{
	struct addrinfo *result = nullptr;
	struct addrinfo *ptr = nullptr;
	struct addrinfo hints;
	DWORD dwRetval;
	struct sockaddr_in  *sockaddr_ipv4;

	RtlSecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	dwRetval = getaddrinfo(in_addr, NULL, &hints, &result);
	if (dwRetval != 0) {
		Log("\n getaddrinfo failed with error: "); Log_llu(dwRetval);
		WSACleanup();
		exit(-1);
	}
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {

		if (ptr->ai_family == AF_INET)
		{
			sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			char str[INET_ADDRSTRLEN];
			inet_ntop(hints.ai_family, &(sockaddr_ipv4->sin_addr), str, INET_ADDRSTRLEN);
			//strcpy(out_addr, inet_ntoa(sockaddr_ipv4->sin_addr));
			strcpy_s(out_addr, 50, str);
			Log("\nAddress: "); Log(in_addr); Log(" defined as: "); Log(out_addr);
		}
	}
	freeaddrinfo(result);
}


void proxy_i(void)
{
	int iResult;
	size_t const buffer_size = 1000;
	char* buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, buffer_size);
	if (buffer == nullptr) ShowMemErrorExit();
	char* tmp_buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, buffer_size);
	if (tmp_buffer == nullptr) ShowMemErrorExit();
	char tbuffer[9];
	SOCKET ServerSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = nullptr;
	struct addrinfo hints;

	RtlSecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(nullptr, proxyport.c_str(), &hints, &result);
	if (iResult != 0) {
		bm_wattron(12);
		bm_wprintw("PROXY: getaddrinfo failed with error: %d\n", iResult, 0);
		bm_wattroff(12);
	}

	ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ServerSocket == INVALID_SOCKET) {
		bm_wattron(12);
		bm_wprintw("PROXY: socket failed with error: %ld\n", WSAGetLastError(), 0);
		bm_wattroff(12);
		freeaddrinfo(result);
	}

	iResult = bind(ServerSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		bm_wattron(12);
		bm_wprintw("PROXY: bind failed with error: %d\n", WSAGetLastError(), 0);
		bm_wattroff(12);
		freeaddrinfo(result);
		closesocket(ServerSocket);
	}
	freeaddrinfo(result);
	BOOL l = TRUE;
	iResult = ioctlsocket(ServerSocket, FIONBIO, (unsigned long*)&l);
	if (iResult == SOCKET_ERROR)
	{
		Log("\nProxy: ! Error ioctlsocket's: "); Log_u(WSAGetLastError());
		bm_wattron(12);
		bm_wprintw("PROXY: ioctlsocket failed: %ld\n", WSAGetLastError(), 0);
		bm_wattroff(12);
	}

	iResult = listen(ServerSocket, 8);
	if (iResult == SOCKET_ERROR) {
		bm_wattron(12);
		bm_wprintw("PROXY: listen failed with error: %d\n", WSAGetLastError(), 0);
		bm_wattroff(12);
		closesocket(ServerSocket);
	}
	Log("\nProxy thread started");

	for (; !exit_flag;)
	{
		struct sockaddr_in client_socket_address;
		int iAddrSize = sizeof(struct sockaddr_in);
		ClientSocket = accept(ServerSocket, (struct sockaddr *)&client_socket_address, (socklen_t*)&iAddrSize);

		char client_address_str[INET_ADDRSTRLEN];
		inet_ntop(hints.ai_family, &(client_socket_address.sin_addr), client_address_str, INET_ADDRSTRLEN);

		if (ClientSocket == INVALID_SOCKET)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				Log("\nProxy:! Error Proxy's accept: "); Log_u(WSAGetLastError());
				bm_wattron(12);
				bm_wprintw("PROXY: can't accept. Error: %ld\n", WSAGetLastError(), 0);
				bm_wattroff(12);
			}
		}
		else
		{
			RtlSecureZeroMemory(buffer, buffer_size);
			do {
				RtlSecureZeroMemory(tmp_buffer, buffer_size);
				iResult = recv(ClientSocket, tmp_buffer, (int)(buffer_size - 1), 0);
				strcat_s(buffer, buffer_size, tmp_buffer);
			} while (iResult > 0);

			Log("\nProxy get info: ");  Log_server(buffer);
			unsigned long long get_accountId = 0;
			unsigned long long get_nonce = 0;
			unsigned long long get_deadline = 0;
			unsigned long long get_totalsize = 0;
			// locate HTTP header
			char *find = strstr(buffer, "\r\n\r\n");
			if (find != nullptr)
			{
				if (strstr(buffer, "submitNonce") != nullptr)
				{

					char *startaccountId = strstr(buffer, "accountId=");
					if (startaccountId != nullptr)
					{
						startaccountId = strpbrk(startaccountId, "0123456789");
						char *endaccountId = strpbrk(startaccountId, "& }\"");

						char *startnonce = strstr(buffer, "nonce=");
						char *startdl = strstr(buffer, "deadline=");
						char *starttotalsize = strstr(buffer, "X-Capacity");
						if ((startnonce != nullptr) && (startdl != nullptr))
						{
							startnonce = strpbrk(startnonce, "0123456789");
							char *endnonce = strpbrk(startnonce, "& }\"");
							startdl = strpbrk(startdl, "0123456789");
							char *enddl = strpbrk(startdl, "& }\"");

							endaccountId[0] = 0;
							endnonce[0] = 0;
							enddl[0] = 0;

							get_accountId = _strtoui64(startaccountId, 0, 10);
							get_nonce = _strtoui64(startnonce, 0, 10);
							get_deadline = _strtoui64(startdl, 0, 10);

							if (starttotalsize != nullptr)
							{
								starttotalsize = strpbrk(starttotalsize, "0123456789");
								char *endtotalsize = strpbrk(starttotalsize, "& }\"");
								endtotalsize[0] = 0;
								get_totalsize = _strtoui64(starttotalsize, 0, 10);
								satellite_size.insert(std::pair <u_long, unsigned long long>(client_socket_address.sin_addr.S_un.S_addr, get_totalsize));
							}
							EnterCriticalSection(&sharesLock);
							shares.push_back({ client_address_str, get_accountId, get_deadline, get_nonce });
							LeaveCriticalSection(&sharesLock);

							_strtime_s(tbuffer);
							bm_wattron(2);
							bm_wprintw("%s [%20llu]\treceived DL: %11llu {%s}\n", tbuffer, get_accountId, get_deadline / baseTarget, client_address_str, 0);
							bm_wattroff(2);
							Log("Proxy: received DL "); Log_llu(get_deadline); Log(" from "); Log(client_address_str);

							//Подтверждаем
							RtlSecureZeroMemory(buffer, buffer_size);
							size_t acc = Get_index_acc(get_accountId);
							int bytes = sprintf_s(buffer, buffer_size, "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n{\"result\": \"proxy\",\"accountId\": %llu,\"deadline\": %llu,\"targetDeadline\": %llu}", get_accountId, get_deadline / baseTarget, bests[acc].targetDeadline);
							iResult = send(ClientSocket, buffer, bytes, 0);
							if (iResult == SOCKET_ERROR)
							{
								Log("\nProxy: ! Error sending to client: "); Log_u(WSAGetLastError());
								bm_wattron(12);
								bm_wprintw("PROXY: failed sending to client: %ld\n", WSAGetLastError(), 0);
								bm_wattroff(12);
							}
							else
							{
								if (use_debug)
								{
									_strtime_s(tbuffer);
									bm_wattron(9);
									bm_wprintw("%s [%20llu]\tsent confirmation to %s\n", tbuffer, get_accountId, client_address_str, 0);
									bm_wattroff(9);
								}
								Log("\nProxy: sent confirmation to "); Log(client_address_str);
							}
						}
					}
				}
				else
				{
					if (strstr(buffer, "getMiningInfo") != nullptr)
					{
						RtlSecureZeroMemory(buffer, buffer_size);
						int bytes = sprintf_s(buffer, buffer_size, "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n{\"baseTarget\":\"%llu\",\"height\":\"%llu\",\"generationSignature\":\"%s\",\"targetDeadline\":%llu}", baseTarget, height, str_signature, targetDeadlineInfo);
						iResult = send(ClientSocket, buffer, bytes, 0);
						if (iResult == SOCKET_ERROR)
						{
							Log("\nProxy: ! Error sending to client: "); Log_u(WSAGetLastError());
							bm_wattron(12);
							bm_wprintw("PROXY: failed sending to client: %ld\n", WSAGetLastError(), 0);
							bm_wattroff(12);
						}
						else
						{
							Log("\nProxy: sent update to "); Log(client_address_str);
						}
					}
					else
					{
						if ((strstr(buffer, "getBlocks") != nullptr) || (strstr(buffer, "getAccount") != nullptr) || (strstr(buffer, "getRewardRecipient") != nullptr))
						{
							; // ничего не делаем, не ошибка, пропускаем
						}
						else
						{
							find[0] = 0;
							bm_wattron(15);
							bm_wprintw("PROXY: %s\n", buffer, 0);//You can crash the miner when the proxy is enabled and you open the address in a browser.  wprintw("PROXY: %s\n", "Error", 0);
							bm_wattroff(15);
						}
					}
				}
			}
			iResult = closesocket(ClientSocket);
		}
		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	HeapFree(hHeap, 0, buffer);
	HeapFree(hHeap, 0, tmp_buffer);
}

void send_i(void)
{
	Log("\nSender: started thread");
	SOCKET ConnectSocket;

	int iResult = 0;
	size_t const buffer_size = 1000;
	char* buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, buffer_size);
	if (buffer == nullptr) ShowMemErrorExit();

	char tbuffer[9];

	struct addrinfo *result = nullptr;
	struct addrinfo hints;

	for (; !exit_flag;)
	{
		if (stopThreads == 1)
		{
			HeapFree(hHeap, 0, buffer);
			return;
		}

		for (auto iter = shares.begin(); iter != shares.end();)
		{

			//Гасим шару если она больше текущего targetDeadline, актуально для режима Proxy
			if ((iter->best / baseTarget) > bests[Get_index_acc(iter->account_id)].targetDeadline)
			{
				if (use_debug)
				{
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu]\t%llu > %llu  discarded\n", tbuffer, iter->account_id, iter->best / baseTarget, bests[Get_index_acc(iter->account_id)].targetDeadline, 0);
					bm_wattroff(2);
				}
				EnterCriticalSection(&sharesLock);
				iter = shares.erase(iter);
				LeaveCriticalSection(&sharesLock);
				continue;
			}

			RtlSecureZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			iResult = getaddrinfo(nodeaddr.c_str(), nodeport.c_str(), &hints, &result);
			if (iResult != 0) {
				if (network_quality > 0) network_quality--;
				bm_wattron(12);
				bm_wprintw("SENDER: getaddrinfo failed with error: %d\n", iResult, 0);
				bm_wattroff(12);
				continue;
			}
			ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				if (network_quality > 0) network_quality--;
				bm_wattron(12);
				bm_wprintw("SENDER: socket failed with error: %ld\n", WSAGetLastError(), 0);
				bm_wattroff(12);
				freeaddrinfo(result);
				continue;
			}
			const unsigned t = 1000;
			setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(unsigned));
			iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				if (network_quality > 0) network_quality--;
				Log("\nSender:! Error Sender's connect: "); Log_u(WSAGetLastError());
				bm_wattron(12);
				_strtime_s(tbuffer);
				bm_wprintw("%s SENDER: can't connect. Error: %ld\n", tbuffer, WSAGetLastError(), 0);
				bm_wattroff(12);
				freeaddrinfo(result);
				continue;
			}
			else
			{
				freeaddrinfo(result);

				int bytes = 0;
				RtlSecureZeroMemory(buffer, buffer_size);
				if (miner_mode == 0)
				{
					bytes = sprintf_s(buffer, buffer_size, "POST /burst?requestType=submitNonce&secretPhrase=%s&nonce=%llu HTTP/1.0\r\nConnection: close\r\n\r\n", pass, iter->nonce);
				}
				if (miner_mode == 1)
				{
					unsigned long long total = total_size / 1024 / 1024 / 1024;
					for (auto It = satellite_size.begin(); It != satellite_size.end(); ++It) total = total + It->second;
					bytes = sprintf_s(buffer, buffer_size, "POST /burst?requestType=submitNonce&accountId=%llu&nonce=%llu&deadline=%llu HTTP/1.0\r\nHost: %s:%s\r\nX-Miner: Blago %s\r\nX-Capacity: %llu\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", iter->account_id, iter->nonce, iter->best, nodeaddr.c_str(), nodeport.c_str(), version, total);
				}

				// Sending to server
				iResult = send(ConnectSocket, buffer, bytes, 0);
				if (iResult == SOCKET_ERROR)
				{
					if (network_quality > 0) network_quality--;
					Log("\nSender: ! Error deadline's sending: "); Log_u(WSAGetLastError());
					bm_wattron(12);
					bm_wprintw("SENDER: send failed: %ld\n", WSAGetLastError(), 0);
					bm_wattroff(12);
					continue;
				}
				else
				{
					unsigned long long dl = iter->best / baseTarget;
					_strtime_s(tbuffer);
					if (network_quality < 100) network_quality++;
					bm_wattron(9);
					bm_wprintw("%s [%20llu] sent DL: %15llu %5llud %02llu:%02llu:%02llu\n", tbuffer, iter->account_id, dl, (dl) / (24 * 60 * 60), (dl % (24 * 60 * 60)) / (60 * 60), (dl % (60 * 60)) / 60, dl % 60, 0);
					bm_wattroff(9);

					EnterCriticalSection(&sessionsLock);
					//sessions.push_back({ ConnectSocket, iter->account_id, dl, iter->best, iter->nonce });
					sessions.push_back({ ConnectSocket, dl, *iter });
					LeaveCriticalSection(&sessionsLock);

					bests[Get_index_acc(iter->account_id)].targetDeadline = dl;
					EnterCriticalSection(&sharesLock);
					iter = shares.erase(iter);
					LeaveCriticalSection(&sharesLock);
				}
			}
		}

		if (!sessions.empty())
		{
			EnterCriticalSection(&sessionsLock);
			for (auto iter = sessions.begin(); iter != sessions.end() && !stopThreads;)
			{
				ConnectSocket = iter->Socket;

				BOOL l = TRUE;
				iResult = ioctlsocket(ConnectSocket, FIONBIO, (unsigned long*)&l);
				if (iResult == SOCKET_ERROR)
				{
					if (network_quality > 0) network_quality--;
					Log("\nSender: ! Error ioctlsocket's: "); Log_u(WSAGetLastError());
					bm_wattron(12);
					bm_wprintw("SENDER: ioctlsocket failed: %ld\n", WSAGetLastError(), 0);
					bm_wattroff(12);
					continue;
				}
				RtlSecureZeroMemory(buffer, buffer_size);
				size_t  pos = 0;
				iResult = 0;
				do {
					iResult = recv(ConnectSocket, &buffer[pos], (int)(buffer_size - pos - 1), 0);
					if (iResult > 0) pos += (size_t)iResult;
				} while (iResult > 0);

				if (iResult == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK) //разрыв соединения, молча переотправляем дедлайн
					{
						if (network_quality > 0) network_quality--;
						//wattron(6);
						//wprintw("%s [%20llu] not confirmed DL %10llu\n", tbuffer, iter->body.account_id, iter->deadline, 0);
						//wattroff(6);
						Log("\nSender: ! Error getting confirmation for DL: "); Log_llu(iter->deadline);  Log("  code: "); Log_u(WSAGetLastError());
						iter = sessions.erase(iter);
						shares.push_back({ iter->body.file_name, iter->body.account_id, iter->body.best, iter->body.nonce });
					}
				}
				else //что-то получили от сервера
				{
					if (network_quality < 100) network_quality++;

					//получили пустую строку, переотправляем дедлайн
					if (buffer[0] == '\0')
					{
						Log("\nSender: zero-length message for DL: "); Log_llu(iter->deadline);
						shares.push_back({ iter->body.file_name, iter->body.account_id, iter->body.best, iter->body.nonce });
					}
					else //получили ответ пула
					{
						char *find = strstr(buffer, "{");
						if (find == nullptr)
						{
							find = strstr(buffer, "\r\n\r\n");
							if (find != nullptr) find = find + 4;
							else find = buffer;
						}

						unsigned long long ndeadline;
						unsigned long long naccountId = 0;
						unsigned long long ntargetDeadline = 0;
						rapidjson::Document answ;
						// burst.ninja        {"requestProcessingTime":0,"result":"success","block":216280,"deadline":304917,"deadlineString":"3 days, 12 hours, 41 mins, 57 secs","targetDeadline":304917}
						// pool.burst-team.us {"requestProcessingTime":0,"result":"success","block":227289,"deadline":867302,"deadlineString":"10 days, 55 mins, 2 secs","targetDeadline":867302}
						// proxy              {"result": "proxy","accountId": 17930413153828766298,"deadline": 1192922,"targetDeadline": 197503}
						if (!answ.Parse<0>(find).HasParseError())
						{
							if (answ.IsObject())
							{
								if (answ.HasMember("deadline")) {
									if (answ["deadline"].IsString())	ndeadline = _strtoui64(answ["deadline"].GetString(), 0, 10);
									else
										if (answ["deadline"].IsInt64()) ndeadline = answ["deadline"].GetInt64();
									Log("\nSender: confirmed deadline: "); Log_llu(ndeadline);

									if (answ.HasMember("targetDeadline")) {
										if (answ["targetDeadline"].IsString())	ntargetDeadline = _strtoui64(answ["targetDeadline"].GetString(), 0, 10);
										else
											if (answ["targetDeadline"].IsInt64()) ntargetDeadline = answ["targetDeadline"].GetInt64();
									}
									if (answ.HasMember("accountId")) {
										if (answ["accountId"].IsString())	naccountId = _strtoui64(answ["accountId"].GetString(), 0, 10);
										else
											if (answ["accountId"].IsInt64()) naccountId = answ["accountId"].GetInt64();
									}

									unsigned long long days = (ndeadline) / (24 * 60 * 60);
									unsigned hours = (ndeadline % (24 * 60 * 60)) / (60 * 60);
									unsigned min = (ndeadline % (60 * 60)) / 60;
									unsigned sec = ndeadline % 60;
									_strtime_s(tbuffer);
									bm_wattron(10);
									if ((naccountId != 0) && (ntargetDeadline != 0))
									{
										EnterCriticalSection(&bestsLock);
										bests[Get_index_acc(naccountId)].targetDeadline = ntargetDeadline;
										LeaveCriticalSection(&bestsLock);
										bm_wprintw("%s [%20llu] confirmed DL: %10llu %5llud %02u:%02u:%02u\n", tbuffer, naccountId, ndeadline, days, hours, min, sec, 0);
										if (use_debug) bm_wprintw("%s [%20llu] set targetDL: %10llu\n", tbuffer, naccountId, ntargetDeadline, 0);
									}
									else bm_wprintw("%s [%20llu] confirmed DL: %10llu %5llud %02u:%02u:%02u\n", tbuffer, iter->body.account_id, ndeadline, days, hours, min, sec, 0);
									bm_wattroff(10);
									if (ndeadline < deadline || deadline == 0)  deadline = ndeadline;

									if (ndeadline != iter->deadline)
									{
										bm_wattron(6);
										bm_wprintw("----Fast block or corrupted file?----\nSent deadline:\t%llu\nServer's deadline:\t%llu \n----\n", iter->deadline, ndeadline, 0); //shares[i].file_name.c_str());
										bm_wattroff(6);
									}
								}
								else {
									if (answ.HasMember("errorDescription")) {
										bm_wattron(15);
										bm_wprintw("[ERROR %u] %s\n", answ["errorCode"].GetInt(), answ["errorDescription"].GetString(), 0);
										bm_wattroff(15);
										bm_wattron(12);
										if (answ["errorCode"].GetInt() == 1004) bm_wprintw("You need change reward assignment and wait 4 blocks (~16 minutes)\n"); //error 1004
										bm_wattroff(12);
									}
									else {
										bm_wattron(15);
										bm_wprintw("%s\n", find);
										bm_wattroff(15);
									}
								}
							}
						}
						else
						{
							if (strstr(find, "Received share") != nullptr)
							{
								_strtime_s(tbuffer);
								deadline = bests[Get_index_acc(iter->body.account_id)].DL; //может лучше iter->deadline ?
																						   // if(deadline > iter->deadline) deadline = iter->deadline;
								bm_wattron(10);
								bm_wprintw("%s [%20llu] confirmed DL   %10llu\n", tbuffer, iter->body.account_id, iter->deadline, 0);
								bm_wattroff(10);
							}
							else //получили нераспознанный ответ
							{
								int minor_version;
								int status = 0;
								const char *msg;
								size_t msg_len;
								struct phr_header headers[12];
								size_t num_headers = sizeof(headers) / sizeof(headers[0]);
								phr_parse_response(buffer, strlen(buffer), &minor_version, &status, &msg, &msg_len, headers, &num_headers, 0);

								if (status != 0)
								{
									bm_wattron(6);
									//wprintw("%s [%20llu] NOT confirmed DL %10llu\n", tbuffer, iter->body.account_id, iter->deadline, 0);
									std::string error_str(msg, msg_len);
									bm_wprintw("Server error: %d %s\n", status, error_str.c_str());
									bm_wattroff(6);
									Log("\nSender: server error for DL: "); Log_llu(iter->deadline);
									shares.push_back({ iter->body.file_name, iter->body.account_id, iter->body.best, iter->body.nonce });
								}
								else //получили непонятно что
								{
									bm_wattron(7);
									bm_wprintw("%s\n", buffer);
									bm_wattroff(7);
								}
							}
						}
					}
					iResult = closesocket(ConnectSocket);
					Log("\nSender: Close socket. Code = "); Log_u(WSAGetLastError());
					iter = sessions.erase(iter);
				}
				if (iter != sessions.end()) ++iter;
			}
			LeaveCriticalSection(&sessionsLock);
		}
		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(send_interval));
	}
	HeapFree(hHeap, 0, buffer);
	return;
}

void updater_i(void)
{
	if (updateraddr.length() <= 3) {
		Log("\nGMI: ERROR in UpdaterAddr");
		exit(2);
	}
	for (; !exit_flag;) {
		pollLocal();
		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(update_interval));
	}
}


void pollLocal(void) {
	size_t const buffer_size = 1000;
	char *buffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, buffer_size);
	if (buffer == nullptr) ShowMemErrorExit();

	int iResult;
	struct addrinfo *result = nullptr;
	struct addrinfo hints;
	SOCKET UpdaterSocket = INVALID_SOCKET;

	RtlSecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(updateraddr.c_str(), updaterport.c_str(), &hints, &result);
	if (iResult != 0) {
		if (network_quality > 0) network_quality--;
		Log("\n*! GMI: getaddrinfo failed with error: "); Log_u(WSAGetLastError());
	}
	else {
		UpdaterSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (UpdaterSocket == INVALID_SOCKET)
		{
			if (network_quality > 0) network_quality--;
			Log("\n*! GMI: socket function failed with error: "); Log_u(WSAGetLastError());
		}
		else {
			const unsigned t = 1000;
			setsockopt(UpdaterSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(unsigned));
			//Log("\n*Connecting to server: "); Log(updateraddr); Log(":"); Log(updaterport);
			iResult = connect(UpdaterSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				if (network_quality > 0) network_quality--;
				Log("\n*! GMI: connect function failed with error: "); Log_u(WSAGetLastError());
			}
			else {
				int bytes = sprintf_s(buffer, buffer_size, "POST /burst?requestType=getMiningInfo HTTP/1.0\r\nHost: %s:%s\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", nodeaddr.c_str(), nodeport.c_str());
				iResult = send(UpdaterSocket, buffer, bytes, 0);
				if (iResult == SOCKET_ERROR)
				{
					if (network_quality > 0) network_quality--;
					Log("\n*! GMI: send request failed: "); Log_u(WSAGetLastError());
				}
				else {
					RtlSecureZeroMemory(buffer, buffer_size);
					size_t  pos = 0;
					iResult = 0;
					do {
						iResult = recv(UpdaterSocket, &buffer[pos], (int)(buffer_size - pos - 1), 0);
						if (iResult > 0) pos += (size_t)iResult;
					} while (iResult > 0);
					if (iResult == SOCKET_ERROR)
					{
						if (network_quality > 0) network_quality--;
						Log("\n*! GMI: get mining info failed:: "); Log_u(WSAGetLastError());
					}
					else {
						if (network_quality < 100) network_quality++;
						Log("\n* GMI: Received: "); Log_server(buffer);

						// locate HTTP header
						char *find = strstr(buffer, "\r\n\r\n");
						if (find == nullptr)	Log("\n*! GMI: error message from pool");
						else {
							rapidjson::Document gmi;
							if (gmi.Parse<0>(find).HasParseError()) Log("\n*! GMI: error parsing JSON message from pool");
							else {
								if (gmi.IsObject())
								{
									if (gmi.HasMember("baseTarget")) {
										if (gmi["baseTarget"].IsString())	baseTarget = _strtoui64(gmi["baseTarget"].GetString(), 0, 10);
										else
											if (gmi["baseTarget"].IsInt64()) baseTarget = gmi["baseTarget"].GetInt64();
									}

									if (gmi.HasMember("height")) {
										if (gmi["height"].IsString())	height = _strtoui64(gmi["height"].GetString(), 0, 10);
										else
											if (gmi["height"].IsInt64()) height = gmi["height"].GetInt64();
									}

									//POC2 determination
									if (height >= POC2StartBlock) {
										POC2 = true;
									}

									if (gmi.HasMember("generationSignature")) {
										strcpy_s(str_signature, gmi["generationSignature"].GetString());
										if (xstr2strr(signature, 33, gmi["generationSignature"].GetString()) == 0)	Log("\n*! GMI: Node response: Error decoding generationsignature\n");
									}
									if (gmi.HasMember("targetDeadline")) {
										if (gmi["targetDeadline"].IsString())	targetDeadlineInfo = _strtoui64(gmi["targetDeadline"].GetString(), 0, 10);
										else
											if (gmi["targetDeadline"].IsInt64()) targetDeadlineInfo = gmi["targetDeadline"].GetInt64();
									}
								}
							}
						}
					}
				}
			}
			iResult = closesocket(UpdaterSocket);
		}
		freeaddrinfo(result);
	}
	HeapFree(hHeap, 0, buffer);
}

void ShowWinner(unsigned long long const num_block)
{
	char* generator = nullptr;
	char* generatorRS = nullptr;
	unsigned long long last_block_height = 0;
	char* name = nullptr;
	char* rewardRecipient = nullptr;
	char* pool_accountRS = nullptr;
	char* pool_name = nullptr;
	unsigned long long timestamp0 = 0;
	unsigned long long timestamp1 = 0;
	char tbuffer[9];
	char* json;
	char* str_req;



	//get second to last block
	str_req = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
	if (str_req == nullptr) ShowMemErrorExit();

	sprintf_s(str_req, HeapSize(hHeap, 0, str_req), "POST /burst?requestType=getBlock&height=%llu HTTP/1.0\r\nConnection: close\r\n\r\n", num_block - 1);
	json = GetJSON(str_req);
	//Log("\n getBlocks: ");

	if (json == nullptr)	Log("\n-! error in message from pool (getBlocks)\n");
	else
	{
		rapidjson::Document doc_block;
		if (doc_block.Parse<0>(json).HasParseError() == false)
		{
			timestamp1 = doc_block["timestamp"].GetUint64();
		}
		else Log("\n- error parsing JSON getBlocks");
	}
	HeapFree(hHeap, 0, str_req);
	if (json != nullptr) HeapFree(hHeap, 0, json);

	str_req = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
	if (str_req == nullptr) ShowMemErrorExit();

	//get last block
	sprintf_s(str_req, HeapSize(hHeap, 0, str_req), "POST /burst?requestType=getBlock&height=%llu HTTP/1.0\r\nConnection: close\r\n\r\n", num_block);

	//try several times
	for (int attempt = 0;attempt < 10;attempt++) {

		//delay
		Sleep(1000);
		json = GetJSON(str_req);
		//Log("\n getBlocks: ");

		if (json == nullptr)	Log("\n-! error in message from pool (getBlocks)\n");
		else
		{
			rapidjson::Document doc_block;
			if (doc_block.Parse<0>(json).HasParseError() == false)
			{
				//if has errorcode = wallet too slow
				if (!doc_block.HasMember("errorCode"))
				{

					generatorRS = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_block["generatorRS"].GetString()) + 1);
					if (generatorRS == nullptr) ShowMemErrorExit();
					strcpy_s(generatorRS, HeapSize(hHeap, 0, generatorRS), doc_block["generatorRS"].GetString());
					generator = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_block["generator"].GetString()) + 1);
					if (generator == nullptr) ShowMemErrorExit();
					strcpy_s(generator, HeapSize(hHeap, 0, generator), doc_block["generator"].GetString());
					last_block_height = doc_block["height"].GetUint();
					timestamp0 = doc_block["timestamp"].GetUint64();
					break;
				}
				else
				{
					_strtime_s(tbuffer);
					bm_wattron(11);
					bm_wprintw("%s Winner: wallet has no winner info yet...\n", tbuffer, 0);
					bm_wattroff(11);
				}
			}
			else Log("\n- error parsing JSON getBlocks");
		}
	}
	HeapFree(hHeap, 0, str_req);
	if (json != nullptr) HeapFree(hHeap, 0, json);



	if ((generator != nullptr) && (generatorRS != nullptr) && (timestamp0 != 0) && (timestamp1 != 0))
		if (last_block_height == height - 1)
		{
			// Запрос данных аккаунта
			str_req = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
			if (str_req == nullptr) ShowMemErrorExit();
			sprintf_s(str_req, HeapSize(hHeap, 0, str_req), "POST /burst?requestType=getAccount&account=%s HTTP/1.0\r\nConnection: close\r\n\r\n", generator);
			json = GetJSON(str_req);
			//Log("\n getAccount: ");

			if (json == nullptr)	Log("\n- error in message from pool (getAccount)\n");
			else
			{
				rapidjson::Document doc_acc;
				if (doc_acc.Parse<0>(json).HasParseError() == false)
				{
					if (doc_acc.HasMember("name"))
					{
						name = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_acc["name"].GetString()) + 1);
						if (name == nullptr) ShowMemErrorExit();
						strcpy_s(name, HeapSize(hHeap, 0, name), doc_acc["name"].GetString());
					}
				}
				else Log("\n- error parsing JSON getAccount");
			}
			HeapFree(hHeap, 0, str_req);
			if (json != nullptr) HeapFree(hHeap, 0, json);

			// Запрос RewardAssighnment по данному аккаунту
			str_req = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
			if (str_req == nullptr) ShowMemErrorExit();
			sprintf_s(str_req, HeapSize(hHeap, 0, str_req), "POST /burst?requestType=getRewardRecipient&account=%s HTTP/1.0\r\nConnection: close\r\n\r\n", generator);
			json = GetJSON(str_req);
			HeapFree(hHeap, 0, str_req);
			//Log("\n getRewardRecipient: ");

			if (json == nullptr)	Log("\n- error in message from pool (getRewardRecipient)\n");
			else
			{
				rapidjson::Document doc_reward;
				if (doc_reward.Parse<0>(json).HasParseError() == false)
				{
					if (doc_reward.HasMember("rewardRecipient"))
					{
						rewardRecipient = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_reward["rewardRecipient"].GetString()) + 1);
						if (rewardRecipient == nullptr) ShowMemErrorExit();
						strcpy_s(rewardRecipient, HeapSize(hHeap, 0, rewardRecipient), doc_reward["rewardRecipient"].GetString());
					}
				}
				else Log("\n-! error parsing JSON getRewardRecipient");
			}

			if (json != nullptr) HeapFree(hHeap, 0, json);

			if (rewardRecipient != nullptr)
			{
				// Если rewardRecipient != generator, то майнит на пул, узнаём имя пула...
				if (strcmp(generator, rewardRecipient) != 0)
				{
					// Запрос данных аккаунта пула
					str_req = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
					if (str_req == nullptr) ShowMemErrorExit();
					sprintf_s(str_req, HeapSize(hHeap, 0, str_req), "POST /burst?requestType=getAccount&account=%s HTTP/1.0\r\nConnection: close\r\n\r\n", rewardRecipient);
					json = GetJSON(str_req);
					//Log("\n getAccount: ");

					if (json == nullptr)
					{
						Log("\n- error in message from pool (pool getAccount)\n");
					}
					else
					{
						rapidjson::Document doc_pool;
						if (doc_pool.Parse<0>(json).HasParseError() == false)
						{
							pool_accountRS = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_pool["accountRS"].GetString()) + 1);
							if (pool_accountRS == nullptr) ShowMemErrorExit();
							strcpy_s(pool_accountRS, HeapSize(hHeap, 0, pool_accountRS), doc_pool["accountRS"].GetString());
							if (doc_pool.HasMember("name"))
							{
								pool_name = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, strlen(doc_pool["name"].GetString()) + 1);
								if (pool_name == nullptr) ShowMemErrorExit();
								strcpy_s(pool_name, HeapSize(hHeap, 0, pool_name), doc_pool["name"].GetString());
							}
						}
						else Log("\n- error parsing JSON pool getAccount");
					}
					HeapFree(hHeap, 0, str_req);
					HeapFree(hHeap, 0, json);
				}
			}

			bm_wattron(11);
			_strtime_s(tbuffer);
			if (name != nullptr) bm_wprintw("%s Winner: %llus by %s (%s)\n", tbuffer, timestamp0 - timestamp1, generatorRS + 6, name, 0);
			else bm_wprintw("%s Winner: %llus by %s\n", tbuffer, timestamp0 - timestamp1, generatorRS + 6, 0);
			if (pool_accountRS != nullptr)
			{
				if (pool_name != nullptr) bm_wprintw("%s Winner's pool: %s (%s)\n", tbuffer, pool_accountRS + 6, pool_name, 0);
				else bm_wprintw("%s Winner's pool: %s\n", tbuffer, pool_accountRS + 6, 0);
			}
			bm_wattroff(11);
		}

	HeapFree(hHeap, 0, generatorRS);
	HeapFree(hHeap, 0, generator);
	HeapFree(hHeap, 0, name);
	HeapFree(hHeap, 0, rewardRecipient);
	HeapFree(hHeap, 0, pool_name);
	HeapFree(hHeap, 0, pool_accountRS);
}

// Helper routines taken from http://stackoverflow.com/questions/1557400/hex-to-char-array-in-c
int xdigit(char const digit) {
	int val;
	if ('0' <= digit && digit <= '9') val = digit - '0';
	else if ('a' <= digit && digit <= 'f') val = digit - 'a' + 10;
	else if ('A' <= digit && digit <= 'F') val = digit - 'A' + 10;
	else val = -1;
	return val;
}

size_t xstr2strr(char *buf, size_t const bufsize, const char *const in) {
	if (!in) return 0; // missing input string

	size_t inlen = (size_t)strlen(in);
	if (inlen % 2 != 0) inlen--; // hex string must even sized

	size_t i, j;
	for (i = 0; i<inlen; i++)
		if (xdigit(in[i])<0) return 0; // bad character in hex string

	if (!buf || bufsize<inlen / 2 + 1) return 0; // no buffer or too small

	for (i = 0, j = 0; i<inlen; i += 2, j++)
		buf[j] = xdigit(in[i]) * 16 + xdigit(in[i + 1]);

	buf[inlen / 2] = '\0';
	return inlen / 2 + 1;
}