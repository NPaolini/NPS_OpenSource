//--------- PClientDriver.h ------------------------------------------------------
//--------------------------------------------------------------------------------
#ifndef PCLIENTDRIVER_H_
#define PCLIENTDRIVER_H_
//--------------------------------------------------------------------------------
#include "PCommonClientServer.h"
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
class dataClient : public baseData
{
	public:
		dataClient(HWND hwOwnerr, HWND hwSVisor, dataContainer& container,
				const dataClientInfo& info);
		~dataClient();

		int elabMsg(HWND hwnd, DWORD msg, WPARAM wp, LPARAM lp);
		void evPaint(HDC hdc, PRect& rect);

	protected:
		virtual void adjuctForSend(pMsgBaseData)  {}

	private:

		void addInfo(PVect<char>& text, int id);

		bool writeCommandToRemote(DWORD msg, WPARAM wp, LPARAM lp);

		void fillFirstSend(LPSTR p, uint id);

		int  sendToServer(DWORD msg, WPARAM wp, LPARAM lp);
		int  handleDataReceived(uint id);
		void handleErrorReconnect(LPCSTR buff, uint id);
		int  reconnectTalk(uint id);
		int  handleNeedReconnect(dataContainer::threadType type, uint id);

		HWND hwSVisor;

		const dataClientInfo& Info;

		DWORD checkCycle;

		SOCKET mainListen;
		DWORD localAddr;

		friend unsigned FAR PASCAL mainListenProc(void*);

};
//--------------------------------------------------------------------------------
/*
inline bool dataClient::initThread()
{
	return true;
}
*/
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
#endif
