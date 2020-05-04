#pragma once

#define SEND_BUFF_BYTES		128
#define RECV_BUFF_BYTES		128

//����/ά��֡/��Ӧ��֡
#define LNPF_KEEPALIVE				0x0000
//����֡
#define LNPF_KEY_FRAME				0x0001
//Դ��Ϣ֡
#define LNPF_OPERATOR_SOURCE_FRMAE	0x0002
//Դ��Ϣ֡��������֡
#define LNPF_OPERATOR_SDN_FRMAE		0x0004
//��������Դ��Ϣ֡
#define LNPF_OTHER_SOURCE_FRAME		0x0020
//��������Դ��Ϣ֡��������֡
#define LNPF_OTHER_SDN_FRMAE		0x0040
//�ɴ������ֻ���������Ϣ(����)
#define LNPF_TEXT_FRAME				0x0100
//������֡��������/ת��KEY_FRAME
#define LNPF_SERVER_FRAME			0x8000
//����
#define LNPF_OFF_LINE				0xffff

#define LNPSF_OFF_LINE				0x0000
#define LNPSF_ON_LINE				0x0001
#define LNPSF_REQUEST				0x0002
//ǿ���־���
#define LNPSF_SERVER				0x0004
//�Ƿ�ת��������SERVER�⣬������������Ҫת���յ��İ�
#define LNPSF_BROKER				0x0008

#define NM_BIND_OK					0x0010
#define NM_BIND_FAILED				0x8010
#define NM_EXIT						0x0030

#define LRC_WSASTARTUP_FAILED		0x8001

#define SEND_QUEUE_SIZE				100

#define LA_FLAG						((((WORD)'A') << 8) | ((WORD)'L'))
typedef struct
{
	WORD	nLaFlg;		//'LA'	LA_FLAG
	WORD	nPkgFlgs;	//LNPF_...
} LANWPKGHEAD;

typedef struct
{
	DWORD	nToState : 1;	// 0: off; 1: on
	DWORD	nActTick : 31;	// action tickcount
	DWORD	nReserved;		// 30,000,000
} LANWPKGKEYFRAME;

typedef struct
{
	WORD	nSrcFlgs;	//LNPSF_
	WORD	nPort;
	DWORD	nAddr;
} LANWPKGSRCFRAME;

typedef struct
{
	BYTE	vText[32];
} LANWPKGTXTFRAME;

typedef struct
{
	long	nLastSendto;		// ��������ýڵ��ʱ��
	long	nLastRecvFrom;		// �������Ըýڵ��ʱ��
	long	nSendPkgs;			// �����ܰ���
	long	nRecvPkgs;			// �����ܰ���
} LANWSRCNODEATTR;

typedef struct tagLANWSRCNODE
{
	LANWPKGSRCFRAME			base;
	LANWSRCNODEATTR			attr;
	struct tagLANWSRCNODE*	branch;	// ͬһ��Դ���������ϵĽڵ��⣬��֧�ڵ���ֻ��branch
	struct tagLANWSRCNODE*	next;	// ��ͬ��Դ
} LANWSRCNODE;

class INetworkEventListener
{
public:
	virtual void OnEvent(WORD nMsg) = 0;
};

class INetworkFrameListener
{
public:
	virtual void OnKeyFrame(const LANWPKGKEYFRAME* pFrame) = 0;
	virtual void OnSourceFrame(const LANWPKGSRCFRAME* pFrame, const char* szDomainName) = 0;
	virtual void OnTextFrame(const LANWPKGTXTFRAME* pFrame) = 0;
};

/**
[STABLESOURCE]
node1=[www.layala.org:2009,x,x,x],[...],...
**/
class CLaNetwork
{
public:
	CLaNetwork(DWORD nFlgs);
	~CLaNetwork();

	BOOL Startup(WORD nPort);
	void Shutdown();

	void BindEventListener(INetworkEventListener* pEventListener) { m_pEventListener = pEventListener; };
	void BindFrameListener(INetworkFrameListener* pFrameListener) { m_pFrameListener = pFrameListener; };

	BOOL AppendKeyFrame(const LANWPKGKEYFRAME* pFrame);
	BOOL PickKeyFrame(LANWPKGKEYFRAME* pFrame);
	BOOL IsSendKeyFrameQueueEmpty() { return m_nSendInPos == m_nSendOutPos; };
	//void AppendTextFrameQ(const LANWPKGKEYFRAME* pFrame);

	BOOL AppendStbSrcNode(const LANWSRCNODE* pStbSrc);
	const LANWSRCNODE* GetStbSrcList() { return m_pStbTabHead; };

private:

	// ��������
	DWORD	m_nFlgs;
	WORD	m_nLocalPort;
	BOOL	m_bNeedExit;

	// CW�����Ͷ���
	LANWPKGKEYFRAME		m_vSendQ[SEND_QUEUE_SIZE];
	int					m_nSendInPos;	// �Ƚ���, ����λ��
	int					m_nSendOutPos;	// �ȳ���, ����λ��, ������InPosʱ����Ϊ��

	// ��̬�����ڵ��
	LANWSRCNODE*		m_pStbTabHead;

	// ��̬�����ڵ��
	LANWSRCNODE*		m_pDynTabHead;

	INetworkEventListener*	m_pEventListener;
	INetworkFrameListener*	m_pFrameListener;

	BOOL		m_bWsaInit;
	HANDLE		m_hNwRecvThread;
	HANDLE		m_hNwSendThread;
	DWORD		m_nNwRecvThreadID;
	DWORD		m_nNwSendThreadID;
	SOCKET		m_socket;

	BOOL Bind(WORD nPort);
	static DWORD WINAPI NwRecvThreadProc(LPVOID pOwner);
	static DWORD WINAPI NwSendThreadProc(LPVOID pOwner);
	BOOL SendKeyFrame(const char* pPkgBuff, int nLen, sockaddr_in* pAddr, const LANWSRCNODE* pHeadNode);
	BOOL AppendNodeByAddr(const sockaddr* pAddr);
};
