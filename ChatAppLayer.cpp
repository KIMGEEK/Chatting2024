#define _CRT_SECURE_NO_WARNINGS
// ChatAppLayer.cpp: implementation of the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "pch.h"
#include "ChatAppLayer.h"
#include "EthernetLayer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName),
	mp_Dlg(NULL)
{
	ResetHeader();
}

CChatAppLayer::~CChatAppLayer()
{
}

void CChatAppLayer::ResetHeader() // ChatApp ��� �ʱ�ȭ
{
	m_sHeader.capp_totlen = 0x0000;
	m_sHeader.capp_type = 0x00;

	memset(m_sHeader.capp_data, 0, APP_DATA_SIZE);
}

BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	// ppayload : �Է��� ���ڿ� , nlength : ������ ����
	m_ppayload = ppayload;
	m_length = nlength;
	
	if (nlength <= APP_DATA_SIZE) {
		((CEthernetLayer*)GetUnderLayer())->SetFrameType(0x2080);
		m_sHeader.capp_totlen = nlength;
		memcpy(m_sHeader.capp_data, ppayload, nlength); // GetBuff�� data ����
		mp_UnderLayer->Send((unsigned char*)&m_sHeader, nlength + APP_HEADER_SIZE);
	}
	else {
		// ������
		AfxBeginThread(ChatThread, this); // ChatThread�Լ��� ������� ����
	}
	return TRUE;
}

BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	// ppayload�� ChatApp ��� ����ü�� �ִ´�.
	PCHAT_APP_HEADER capp_hdr = (PCHAT_APP_HEADER)ppayload;
	static unsigned char* GetBuff; // �����͸� ���� GetBuff�� �����Ѵ�.

	if (capp_hdr->capp_totlen <= APP_DATA_SIZE) {
		GetBuff = (unsigned char*)malloc(capp_hdr->capp_totlen);
		memset(GetBuff, 0, capp_hdr->capp_totlen);  
		memcpy(GetBuff, capp_hdr->capp_data, capp_hdr->capp_totlen); 
		GetBuff[capp_hdr->capp_totlen] = '\0';

		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff); // ���� �������� ������ �ø�
		return TRUE;
	}
	// �� �������� �Ѱܹ��� ppayload�� �м��Ͽ� ChatDlg �������� �Ѱ��ش�.
	if (capp_hdr->capp_type == DATA_TYPE_BEGIN) // ������ ù �κ�
	{
		// ù �κ� �� ��� �� ũ�⸸ŭ ���� �Ҵ�		
		GetBuff = (unsigned char*)malloc(capp_hdr->capp_totlen);
		memset(GetBuff, 0, capp_hdr->capp_totlen);  // GetBuff�� �ʱ�ȭ���ش�.
	}
	else if (capp_hdr->capp_type == DATA_TYPE_CONT) // ������ �߰� �κ�
	{
		// ��� ���ۿ� �״´�.
		strncat((char*)GetBuff, (char*)capp_hdr->capp_data, strlen((char*)capp_hdr->capp_data));
		GetBuff[strlen((char*)GetBuff)] = '\0';
	}
	else if (capp_hdr->capp_type == DATA_TYPE_END) // ������ �� �κ�
	{
		// ���ۿ� ���� �����͸� �ٽ� GetBuff�� �ִ´�.
		memcpy(GetBuff, GetBuff, capp_hdr->capp_totlen);
		GetBuff[capp_hdr->capp_totlen] = '\0';

		// ������ ������� �޽��� ������ ChatDlg�� �Ѱ��ش�.
		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff);
		free(GetBuff);
	}
	else
		return FALSE;

	return TRUE;
}

UINT CChatAppLayer::ChatThread(LPVOID pParam)
{
	// Thread �Լ��� static���� ���Ǳ⿡, ���ڷ� �Ѱܹ��� pParam�� ����ִ� Chatapp�� Ŭ������ �̿��Ͽ� ���� �����Ѵ�.
	BOOL bSuccess = FALSE;
	CChatAppLayer* pChat = (CChatAppLayer*)pParam;
	((CEthernetLayer*)(pChat->GetUnderLayer()))->SetFrameType(0x2080);
	int data_length = APP_DATA_SIZE; // ���� data�� ����
	int seq_tot_num; // Sequential
	int data_index;	 // data�� ���� ���� ��, ���� ���� data�� ������ �� index�� ����
					 // ���� data�� ���� ���� ù index�� �ȴ�.
	int temp = 0;

	// sequential number�� �� ������ ����
	if (pChat->m_length < APP_DATA_SIZE) // APP_DATA_SIZE ���� ������ 1. (�� ���� �����ϸ� �ȴ�.)
		seq_tot_num = 1;
	else // �׷��� ������, ������ ���̷� APP_DATA_SIZE�� ���� �� 1�� ���� ������ seq_tot_num�� �����Ѵ�.
		seq_tot_num = (pChat->m_length / APP_DATA_SIZE) + 1;

	for (int i = 0; i <= seq_tot_num + 1; i++)
	{
		// ���� data�� ���̸� ����
		if (seq_tot_num == 1) { // ���� Ƚ���� �� ���̸�, ������ ���� ��ŭ ������ �ȴ�.
			data_length = pChat->m_length;
		}
		else { // ���� Ƚ���� �� �� �̻��̰�,
			if (i == seq_tot_num) // ���� Ƚ���� ���� �������� ����, ���� �������� ���̸�ŭ ������.
				data_length = pChat->m_length % APP_DATA_SIZE;
			else // ó��, �߰� ������ �� ��, APP_DATA_SIZE��ŭ ������.
				data_length = APP_DATA_SIZE;
		}

		memset(pChat->m_sHeader.capp_data, 0, data_length);

		if (i == 0) // ó���κ� : Ÿ���� 0x00, �������� �� ���̸� �����Ѵ�.
		{
			pChat->m_sHeader.capp_totlen = pChat->m_length;
			pChat->m_sHeader.capp_type = DATA_TYPE_BEGIN;
			memset(pChat->m_sHeader.capp_data, 0, data_length);
			data_length = 0;
		}
		else if (i != 0 && i <= seq_tot_num) // �߰� �κ� : Ÿ���� 0x01, seq_num�� �������, 
		{
			data_index = data_length * 2;
			pChat->m_sHeader.capp_type = DATA_TYPE_CONT;
			pChat->m_sHeader.capp_seq_num = i - 1;

			CString str;
			str = pChat->m_ppayload;
			str = str.Mid(temp, temp + data_index);

			memcpy(pChat->m_sHeader.capp_data, str, data_length);
			temp += data_index;
		}
		else // ������ �κ� : Ÿ���� 0x02
		{
			pChat->m_sHeader.capp_type = DATA_TYPE_END;
			memset(pChat->m_ppayload, 0, data_length);
			data_length = 0;
		}
		bSuccess = pChat->mp_UnderLayer->Send((unsigned char*)&pChat->m_sHeader, data_length + APP_HEADER_SIZE);
	}

	return bSuccess;
	// AfxEndThread() �� �� ���� ��.
}