#ifndef NETWORK_ADAPTER_H
#define NETWORK_ADAPTER_H

#include "Pcap.h"
#include "headers.h"
#include <Shlwapi.h>
#include <Packet32.h>
#include <Windows.h>
#include <ntddndis.h>
#include <Iphlpapi.h>

#define MAX_SNAP_LEN	65536

class NetworkAdapter {
public:
	NetworkAdapter();
	~NetworkAdapter();

	pcap_t* openAdapter();
	pcap_if_t* getDeviceList();
	pcap_if_t* getCurrentDevice();
	pcap_t* getAdapterHandle();
	int getLocalIpAndMask(uint32_t *pIp, uint32_t *pMask);

	BOOLEAN GetLocalMac(PUCHAR MacAddr);
	VOID GetNetAddrOfRouter(PDWORD pIpAddr, PUCHAR MacAddr);

private:
	int initDeviceList();
	void getAdapterParams();

private:
	pcap_if_t *m_alldevs;				// ��������������ṹ
	pcap_if_t *m_d;						// ����������ڵ㶨λָ��
	pcap_t *m_adhandle;					// �������������
	pcap_addr_t *m_paddr;				// ������ַ�ṹ
	char m_AdapterName[PCAP_BUF_SIZE];	// ��������������
	int m_devnum;						// ���������� (unused)
};

#endif // NETWORK_ADAPTER_H
