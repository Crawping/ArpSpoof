#include "NetworkAdapter.h"

//#define WIRELESS // 无线网络

/////////////////////////////////////////////// public ///////////////////////////////////////////////

NetworkAdapter::NetworkAdapter()
{
	m_alldevs = NULL;
	m_d = NULL;
	m_adhandle = NULL;
	m_paddr = NULL;

	if (initDeviceList() != -1)
	{
		getAdapterParams();
		m_adhandle = openAdapter();
	}
	else
	{
		printf("\nError: adapters not found!");
	}
}

NetworkAdapter::~NetworkAdapter()
{
}

pcap_t* NetworkAdapter::openAdapter()
{
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap_t* adhandle = pcap_open(m_AdapterName, // 设备名
		MAX_SNAP_LEN,	// 可处理的数据包长度
		PCAP_OPENFLAG_PROMISCUOUS, // 混杂模式
		1000,  // 读取超时时间
		NULL, // 远程机器验证
		errbuf); // 错误缓冲池

	if (adhandle == NULL)
	{
		printf("\nError: cannot open the adapter: %s", errbuf);
		pcap_freealldevs(m_alldevs);
	}
	return adhandle;
}

pcap_if_t* NetworkAdapter::getDeviceList()
{
	return m_alldevs;
}

pcap_if_t* NetworkAdapter::getCurrentDevice()
{
	return m_d;
}

pcap_t* NetworkAdapter::getAdapterHandle()
{
	return m_adhandle;
}

/**
 * 获取一个可用的网络适配器对应的 ip 地址(本机 ip 地址)及其掩码.
 * return (0) if succeeded, or (1) if failed
 */
int NetworkAdapter::getLocalIpAndMask(uint32_t *pIp, uint32_t *pMask)
{
	uint32_t ip, mask;
	BOOLEAN bOk = FALSE;
	for (m_paddr = m_d->addresses; m_paddr; m_paddr = m_paddr->next)
	{
		ip = ((struct sockaddr_in *)(m_paddr->addr))->sin_addr.S_un.S_addr;
		mask = ((struct sockaddr_in *)(m_paddr->netmask))->sin_addr.S_un.S_addr;

		if (ip && mask)
		{
			if (pIp != NULL) { *pIp = ip; }
			if (pMask != NULL) { *pMask = mask; }
			bOk = TRUE;
			break;
		}
	}
	if (!bOk)
	{
		pcap_freealldevs(m_alldevs);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

BOOLEAN NetworkAdapter::GetLocalMac(PUCHAR MacAddr)
{
	LPADAPTER lpAdapter = PacketOpenAdapter(m_AdapterName + 8); // `+8` 跳过"rpcap:\\"
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
		return FALSE;

	PPACKET_OID_DATA pOidData = (PPACKET_OID_DATA)malloc(
		sizeof(PACKET_OID_DATA) + MAC_LEN); // 查看结构体定义，结合MAC地址的长度，便可知道'+6'的含义
	if (pOidData == NULL)
	{
		PacketCloseAdapter(lpAdapter);
		return FALSE;
	}

	// Retrieve the adapter MAC querying the NIC driver
	pOidData->Oid = OID_802_3_CURRENT_ADDRESS; // 获取 MAC 地址
	pOidData->Length = MAC_LEN;
	memset(pOidData->Data, 0, MAC_LEN);

	BOOLEAN bOk = PacketRequest(lpAdapter, FALSE, pOidData);
	if (bOk)
		memcpy(MacAddr, pOidData->Data, MAC_LEN);

	free(pOidData);
	PacketCloseAdapter(lpAdapter);
	return bOk;
}

VOID NetworkAdapter::GetNetAddrOfRouter(PDWORD pIpAddr, PUCHAR MacAddr)
{
	PMIB_IPNETTABLE pIpNetTable = NULL;
	ULONG dwSize = 0;

	if (GetIpNetTable(pIpNetTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		// The required size is returned in the `dwSize`
		pIpNetTable = (PMIB_IPNETTABLE)malloc(dwSize);
		GetIpNetTable(pIpNetTable, &dwSize, FALSE);
	}

	for (DWORD i = 0; i < pIpNetTable->dwNumEntries; i++)
	{
		// 第一条 dwPhysAddrLen 不为 0 的项就是默认网关
		if (pIpNetTable->table[i].dwPhysAddrLen == MAC_LEN)
		{
			if (MacAddr != NULL)
				memcpy(MacAddr, pIpNetTable->table[i].bPhysAddr, MAC_LEN);
			if (pIpAddr != NULL)
				*pIpAddr = pIpNetTable->table[i].dwAddr;
			break;
		}
	}
	free(pIpNetTable);
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

int NetworkAdapter::initDeviceList()
{
	int ret = -1;
	char errbuf[PCAP_ERRBUF_SIZE];
	ret = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &m_alldevs, errbuf);
	if (ret == -1)
	{
		printf("\nError: pcap_findalldevs_ex: %s", errbuf);
	}
	return ret; // return (-1) if failed
}

/**
* 获取适配器参数:
* 1. 将`m_d'定位到一个可用的网络适配器
* 2. 获取适配器数量`m_devnum'
* 3. 获取`m_d'指向的适配器的名字, 结果保存在`m_AdapterName'
*/
void NetworkAdapter::getAdapterParams()
{
	int count = 0;
	bool located = false;
	pcap_if_t *d;
	for (d = m_alldevs; d; d = d->next, count++)
	{
#ifdef WIRELESS
		if (!located && d->description)
		{
			// 描述信息中含有关键字 "Microsoft" 的适配器即为可用的适配器
			if (StrStrIA(d->description, "Microsoft"))
			{
				m_d = d; // 1. 定位到可用的网络适配器
				located = true;
			}
		}
#else
		printf("\n[%d] %s", count, d->description);
#endif
	}
	if (count == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
	}
	else
	{
#ifndef WIRELESS
		// 让用户选择一个网络适配器
		int i, j;
		printf("\n[NetworkAdapter] choose an available adapter: ");
		scanf("%d", &i);
		if (i < 0 || i >= count)
		{
			printf("invalid index!\n");
			exit(1);
		}
		for (m_d = m_alldevs, j = 0; m_d && j < i; m_d = m_d->next, j++) {}
		printf("\n--%s\n", m_d->description);
#endif
		m_devnum = count; // 2. 网络适配器数量
		strcpy(m_AdapterName, m_d->name); // 3. 网络适配器名字
	}
}
