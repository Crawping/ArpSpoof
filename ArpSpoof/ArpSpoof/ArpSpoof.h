#ifndef ARP_SPROOF_H
#define ARP_SPROOF_H

#include "NetworkAdapter.h"

class ArpSpoof {
public:
	ArpSpoof();
	~ArpSpoof();
	u_int getNetAddr();
	u_int getHostsnum();
	void beignAttack(u_int dst_ip);

private:
	void make_arp_packet(u_char* packet, u_char* src_mac, u_int src_ip, u_int dst_ip);

private:
	NetworkAdapter m_Adapter;
	u_int m_netaddr; // �����ַ
	u_int m_netmask; // ��������
	u_int m_hostsnum; // �������ڿ����ɵ������� (��ȥ������ַȫ0�������ַ��ȫ1�Ĺ㲥��ַ)
	u_char m_local_mac[MAC_LEN]; // ���� MAC ��ַ
	u_int m_local_ip; // ���� IP ��ַ
	u_char m_router_mac[MAC_LEN]; // ·���� MAC ��ַ
	u_int m_router_ip; // ·���� IP ��ַ
};

#endif // ARP_SPROOF_H
