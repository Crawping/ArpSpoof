#ifndef ICMP_DETECT_H
#define ICMP_DETECT_H

#include "Pcap.h"
#include "headers.h"
#include "NetworkAdapter.h"
#include "List.h"

DWORD WINAPI recvICMPThread(LPVOID param);

// pcap_loop �Ļص�����
void icmp_packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

// ���ڷ�װ���ݸ��̺߳����Ĳ���
typedef struct {
	pcap_t* adhandle;
	pcap_if_t* alldevs;
	pcap_if_t* d;
	bpf_program* pfcode;
	char* filter;
} PCAP_PARAM, *PPCAP_PARAM;


class IcmpDetect {
public:
	IcmpDetect(u_int net, u_int netmask);
	~IcmpDetect();
	List getAliveHosts();
	void beginDetect();

private:
	u_short cksum(u_short *p, int len);

	void make_icmp_packet(u_char* packet,
		u_char* src_mac, u_int src_ip,
		u_char* dst_mac, u_int dst_ip,
		u_char type, u_short seq);

	void sendICMP(u_char* packet, int len);

private:
	NetworkAdapter m_Adapter;
	u_int m_net; // �����ַ
	u_int m_netmask; // ����
	u_int m_hostnum; // �����ڿɱ����� ip ��������
	char *m_pkt_filter = "icmp"; // ��������: �����ܲ����� ICMP ���ݱ�
	struct bpf_program m_fcode;
	HANDLE m_hRecvThread; // ����ICMP���ĵ��߳̾��
};

#endif // ICMP_DETECT_H