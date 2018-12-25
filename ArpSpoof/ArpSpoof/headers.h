#ifndef HEADERS_H
#define HEADERS_H
#include "type.h"

#define MAC_LEN				6		// MAC ��ַ, 48 bits = 6 bytes
#define IPV4_LEN			4		// IPV4 ��ַ, 32 bits = 4 bytes
#define PADDING_LEN			18		// ARP ���ݰ�����Ч�غɳ���
#define ICMP_DATA_LEN		32		// ICMP ���ݰ�����Ч�غɳ���


#pragma pack(push, 1)

#define ETHPROTOCAL_IPV4		0x0800 // ��̫���ϲ�Э������: IPv4
#define ETHPROTOCAL_ARP			0x0806 // ��̫���ϲ�Э������: ARP

typedef struct ether_header 
{
	uint8_t daddr[MAC_LEN];				// Ŀ�� MAC ��ַ
	uint8_t saddr[MAC_LEN];				// Դ MAC ��ַ
	uint16_t prototype;					// �ϲ�Э������ (0x0800->IPv4, 0x0806->ARP)
} ether_header;

#define HARD_ETHERNET			0x0001
#define ARP_REQUEST				0x0001 // ARP ����
#define ARP_RESPONCE			0x0002 // ARP Ӧ��

typedef struct arp_header 
{
	uint16_t arp_hrd;				// Ӳ������
	uint16_t arp_pro;				// �ϲ�Э������
	uint8_t arp_hln;				// Ӳ����ַ����
	uint8_t arp_pln;				// Э���ַ����
	uint16_t arp_op;				// ����ѡ�� (���� or Ӧ��)
	uint8_t arp_shaddr[MAC_LEN];	// ������Ӳ����ַ (MAC)
	uint32_t arp_spaddr;			// ������Э���ַ (IPv4)
	uint8_t arp_thaddr[MAC_LEN];	// Ŀ��Ӳ����ַ (MAC)
	uint32_t arp_tpaddr;			// Ŀ��Э���ַ (IPv4)
} arp_header;

typedef struct arp_packet 
{
	ether_header eh;				// ��̫���ײ�
	arp_header ah;					// ARP �ײ�
} arp_packet;

#define IPV4PROTOCOL_ICMP	1

typedef struct ip_header 
{
	uint8_t  ver_ihl;				// �汾 (4 bits) + �ײ����� (4 bits)
	uint8_t  tos;					// ��������(Type of service) 
	uint16_t tlen;					// �ܳ�(Total length) 
	uint16_t identification;		// ��ʶ(Identification)
	uint16_t flags_fo;				// ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
	uint8_t  ttl;					// ���ʱ��(Time to live)
	uint8_t  proto;					// �ϲ�Э��(Protocol)
	uint16_t cksum;					// �ײ�У���(Header checksum)
	uint32_t  saddr;				// Դ��ַ(Source address)
	uint32_t  daddr;				// Ŀ�ĵ�ַ(Destination address)
} ip_header;

#define ICMP_REQUEST	8
#define ICMP_REPLY		0

typedef struct icmp_header 
{
	uint8_t type;				// ICMP���ݱ�����
	uint8_t code;				// ����
	uint16_t cksum;				// У���
	uint16_t id;				// ��ʶ(ͨ��Ϊ��ǰ����pid)
	uint16_t seq;				// ���
} icmp_header;

typedef struct icmp_packet {
	ether_header eh;				// ��̫���ײ�
	ip_header iph;					// IPv4 �ײ�
	icmp_header icmph;				// ICMP �ײ�
	uint8_t data[ICMP_DATA_LEN];
} icmp_packet;

typedef struct tcp_header 
{
	uint16_t sport;				// Դ�˿ں�
	uint16_t dport;				// Ŀ�Ķ˿ں�
	uint32_t seq;				// ���
	uint32_t ack;				// ȷ�Ϻ�
	uint8_t lenres;				// 4 bits ������ƫ�ƺ� 4 bits �ı����ֶ�
	uint8_t flag;				// ��־
	uint16_t win;				// ���ڳ���
	uint16_t cksum;				// У���
	uint16_t urp;				// ����ָ��
} tcp_header;

typedef struct udp_header 
{
	uint16_t sport;           	// Դ�˿�(Source port)
	uint16_t dport;          	// Ŀ�Ķ˿�(Destination port)
	uint16_t len;				// UDP���ݰ�����(Datagram length)
	uint16_t cksum;         	// У���(Checksum)
} udp_header;

#pragma pack(pop)

#endif // HEADERS_H
