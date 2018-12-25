#ifndef ARP_SPOOF_H
#define ARP_SPOOF_H

#include "NetworkAdapter.h"

class ArpSpoof {
public:
	ArpSpoof(NetworkAdapter &adapter);
	~ArpSpoof();
	void ArpSpoof::beignAttack(uint32_t dst_ip, uint32_t src_ip, uint8_t *src_mac);
	void ArpSpoof::beignAttack(uint32_t dst_ip, uint32_t src_ip);

private:
	void make_arpspoof_packet(uint8_t *packet, uint32_t dst_ip, uint32_t src_ip, uint8_t *src_mac);
	void make_arpspoof_packet(uint8_t *packet, uint32_t dst_ip, uint32_t src_ip);

private:
	pcap_t *m_adhandle; // �������������
	uint8_t m_local_mac[MAC_LEN]; // ���� MAC ��ַ
};

#endif // ARP_SPOOF_H
