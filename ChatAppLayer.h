// ChatAppLayer.h: interface for the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)
#define AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"


class CChatAppLayer
	: public CBaseLayer
{
private:
	inline void		ResetHeader();
	CObject* mp_Dlg;

public:
	unsigned char* m_ppayload;
	int				m_length;

	BOOL			Receive(unsigned char* ppayload);
	BOOL			Send(unsigned char*, int);

	static UINT		ChatThread(LPVOID pParam);	// ChatApp 쓰레드

	CChatAppLayer(char* pName);
	virtual ~CChatAppLayer();

	typedef struct _CHAT_APP_HEADER {
		unsigned long	capp_seq_num;	// APP Sequential Number
		unsigned short	capp_totlen;	// total length of the data
		unsigned char	capp_type;	// type of application data

		unsigned char	capp_data[APP_DATA_SIZE]; // application data

	} CHAT_APP_HEADER, * PCHAT_APP_HEADER;

protected:
	CHAT_APP_HEADER		m_sHeader;
	enum {
		DATA_TYPE_BEGIN = 0x00,	// 전송 시작 부분
		DATA_TYPE_CONT = 0x01,	// 전송 중간 부분
		DATA_TYPE_END = 0x02
	};// 전송   끝 부분
};

#endif // !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)










