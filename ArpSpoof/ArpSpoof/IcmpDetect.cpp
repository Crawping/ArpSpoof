#include "IcmpDetect.h"
#include <assert.h>

#define THREAD_TIMEOUT	5000
#define GATE_HOSTNO		1 // ����·����������

u_int g_netmask;
u_int g_Local_ip;
List g_aliveHosts = NULL; // ��������б�

DWORD WINAPI recvICMPThread(LPVOID param)
{
	PPCAP_PARAM _param = (PPCAP_PARAM)param;
	u_int netmask;

	if (_param->d->addresses != NULL)
	{
		// ��ýӿڵ�һ����ַ������
		netmask = ((struct sockaddr_in *)(_param->d->addresses->netmask))->sin_addr.S_un.S_addr;
	}
	else
	{
		// ����ӿ�û�е�ַ������һ��C�������
		netmask = 0xffffff;
	}

	// ���������
	if (pcap_compile(_param->adhandle, _param->pfcode, _param->filter, 1, netmask) <0)
	{
		printf("\nUnable to compile the packet filter. Check the syntax.\n");
		pcap_freealldevs(_param->alldevs);
		return EXIT_FAILURE;
	}

	// ����
	if (pcap_setfilter(_param->adhandle, _param->pfcode) < 0)
	{
		printf("\nError setting the filter.\n");
		pcap_freealldevs(_param->alldevs);
		return EXIT_FAILURE;
	}

	// �ͷ��豸�б���ʼ����
	printf("\nThread[%d] listening on %s...\n", GetCurrentThreadId(), _param->d->description);
	pcap_freealldevs(_param->alldevs);
	pcap_loop(_param->adhandle, 0, icmp_packet_handler, NULL);
	
	return EXIT_SUCCESS;;
}

void icmp_packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	ip_header* iph;
	icmp_header *icmph;
	u_int ip_len;
	in_addr addr;

	iph = (ip_header*)(pkt_data + sizeof(ether_header));
	ip_len = (iph->ver_ihl & 0x0F) << 2;
	icmph = (icmp_header *)(pkt_data + sizeof(ether_header) + ip_len);
	
	// �ж��Ƿ��ǻظ��ҷ��� ICMP ����
	if (icmph->type == ICMP_REPLY &&
		icmph->id == (u_short)GetCurrentProcessId())
	{
		addr.S_un.S_addr = iph->saddr;
		
		// ����������·������Ӧ�ó����ڹ����б���
		u_int hostno = htonl(iph->saddr & ~g_netmask);
		if (hostno != GATE_HOSTNO && iph->saddr != g_Local_ip)
		{
			if (List_Add(g_aliveHosts, iph->saddr)) // ����������� IP �����б�
				printf("\n%s", inet_ntoa(addr));
		}
	}
}

/////////////////////////////////////////////// public ///////////////////////////////////////////////

IcmpDetect::IcmpDetect(u_int net, u_int netmask)
{
	m_net = net;
	m_netmask = g_netmask = netmask;
	m_hostnum = htonl(~netmask) - 1; // ��ȥ������ȫ0�������ַ��������ȫ1�Ĺ㲥��ַ

	g_aliveHosts = List_Create();
}

IcmpDetect::~IcmpDetect()
{

}

void IcmpDetect::beginDetect()
{
	u_char packet[sizeof(icmp_packet)];
	u_char src_mac[MAC_LEN];
	u_char dst_mac[MAC_LEN];
	u_int src_ip, netmask;
	u_int dst_ip;

	// ��ȡ���� MAC ��ַ
	if (!m_Adapter.GetLocalMac(src_mac))
	{
		printf("\nError: cannot get physical address(MAC) of your PC!");
		return;
	}
	// ��ȡĬ�����ض�Ӧ�� MAC ��ַ (��·������ MAC ��ַ)
	m_Adapter.GetNetAddrOfRouter(NULL, dst_mac);

	// ��ȡ���� IP ��ַ����������
	if (m_Adapter.getLocalIpAndMask(&src_ip, &netmask) == -1)
	{
		printf("\nError: cannot get ip addr and subnet mask of you PC!");
		return;
	}
	g_Local_ip = src_ip;

	PCAP_PARAM param;
	param.adhandle = m_Adapter.getAdapterHandle();
	param.alldevs = m_Adapter.getDeviceList();
	param.d = m_Adapter.getCurrentDevice();
	param.filter = m_pkt_filter;
	param.pfcode = &m_fcode;

	assert(param.adhandle && param.alldevs && param.d);

	// ���������߳�
	m_hRecvThread = CreateThread(NULL, 0, recvICMPThread,
		(LPVOID)&param, 0, NULL);
	if (m_hRecvThread == NULL)
	{
		printf("\nCreateThread Error: #%d", GetLastError());
		return;
	}

	// ���� ICMP ������
	for (u_int host = 1;host < m_hostnum;host++)
	{
		dst_ip = m_net | htonl(host);
		make_icmp_packet(packet,
			src_mac, src_ip,
			dst_mac, dst_ip,
			ICMP_REQUEST, host);
		sendICMP(packet, sizeof(packet));
	}
}

List IcmpDetect::getAliveHosts()
{
	WaitForSingleObject(m_hRecvThread, THREAD_TIMEOUT);
	return g_aliveHosts;
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

u_short IcmpDetect::cksum(u_short *p, int len) {
	int cksum = 0;
	u_short answer = 0;

	// ��16bitsΪ��λ�ۼ�
	while (len > 1) {
		u_short t = *p;
		cksum += *p++;
		len -= 2;
	}
	// ������ݵ��ֽ���Ϊ����, �����һ���ֽ���Ϊ16bits�ĸ�8bits, ��8bits��0, �����ۼ�
	if (len == 1) {
		answer = *(u_short *)p;
		cksum += answer;
	}
	// cksum��32bits��int, ��У�����Ϊ16bits, �轫cksum�ĸ�16bits�ӵ���16bits��
	cksum = (cksum >> 16) + (cksum & 0xffff);
	// ��λ��
	return (~(u_short)cksum);
}

void IcmpDetect::make_icmp_packet(u_char* packet,
	u_char* src_mac, u_int src_ip,
	u_char* dst_mac, u_int dst_ip,
	u_char type, u_short seq)
{
	icmp_packet icmp_pkt;
	memset(&icmp_pkt, 0, sizeof(icmp_packet));

	// -----------------�����̫���ײ�-----------------
	memcpy(icmp_pkt.eh.daddr, dst_mac, MAC_LEN);
	memcpy(icmp_pkt.eh.saddr, src_mac, MAC_LEN);
	icmp_pkt.eh.prototype = htons(ETHPROTOCAL_IPV4);

	// -----------------��� IPv4 �ײ�-----------------
	icmp_pkt.iph.ver_ihl = 0x45;
	icmp_pkt.iph.tlen = htons(sizeof(icmp_pkt) - sizeof(ether_header));
	icmp_pkt.iph.ttl = 128;
	icmp_pkt.iph.proto = IPV4PROTOCOL_ICMP;
	icmp_pkt.iph.saddr = src_ip;
	icmp_pkt.iph.daddr = dst_ip;
	icmp_pkt.iph.cksum = cksum((u_short*)&icmp_pkt.iph, sizeof(ip_header));

	// -----------------��� ICMP �ײ�-----------------
	icmp_pkt.icmph.type = type;
	icmp_pkt.icmph.id = (u_short)GetCurrentProcessId();
	icmp_pkt.icmph.seq = htons(seq);
	memset(icmp_pkt.data, 0xCC, sizeof(icmp_pkt.data));
	icmp_pkt.icmph.cksum = cksum((u_short*)&icmp_pkt.icmph,
		sizeof(icmp_header) + sizeof(icmp_pkt.data));

	memcpy(packet, &icmp_pkt, sizeof(icmp_pkt));
}

void IcmpDetect::sendICMP(u_char* packet, int len)
{
	pcap_t* adhandle = m_Adapter.getAdapterHandle();
	if (adhandle != NULL)
	{
		if (pcap_sendpacket(adhandle, packet, len) == -1)
			printf("\nError: sending packet failed!");
	}
}
