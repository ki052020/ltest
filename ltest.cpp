#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/ioctl.h>
#include	<arpa/inet.h>
#include	<sys/socket.h>
#include	<linux/if.h>
#include	<net/ethernet.h>
#include	<netpacket/packet.h>
#include	<netinet/if_ether.h>

// 戻り値：作成した socket
int InitRawSocket(const char* device, bool bPromiscFlag);
// MAC を文字列に変換する（デバッグ用途）
char* my_ether_ntoa_r(const u_char* hwaddr, char* pbuf, socklen_t size);
// イーサヘッダをデバッグ表示する
void PrintEtherHeader(const ether_header* eh, FILE* fp);

// --------------------------------------------------------------------
// main
// arvv[1] = device名
int main(const int argc, const char* argv[])
{
	if (argc <= 1)
	{
		fprintf(stderr,"ltest device-name\n");
		return 1;
	}

	// ---------------------------------------
	// arvv[1] = device名
	const int soc = InitRawSocket(argv[1], false);
	if(soc == -1)
	{
		fprintf(stderr, "InitRawSocket:error:%s\n", argv[1]);
		return -1;
	}

	// ---------------------------------------
	// とりあえず、10 個のみ表示させてみる
	u_char buf[65535];
	for (int i = 10; i > 0; --i)
	{
		const int size = read(soc, buf, sizeof(buf));
		if (size <= 0)
			{ perror("read"); }
		else
		{
			if (size >= (int)sizeof(ether_header))
				{ PrintEtherHeader((ether_header*)buf, stdout); }
			else
				{ fprintf(stderr,"read size(%d) < %d\n", size, (int)sizeof(ether_header)); }
		}
	}

	close(soc);
	return 0;
}

// --------------------------------------------------------------------
// InitRawSocket
// 戻り値：作成した socket
int InitRawSocket(const char* device, const bool bPromiscFlag)
{
	// ---------------------------------------
	// PF_PACKET, SOCK_RAW : L2 レイヤを指定
	// ETH_P_ALL : すべてのパケット / ETH_P_IP : IP のみ
	// htons() : バイトオーダーを、ネットワークバイトオーダーに変換する
	const int soc = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (soc < 0)
	{
		perror("socket");
		return -1;
	}
	
	// ---------------------------------------
	// ifreq の取得（interface index の取得用）
	ifreq ifreq;  // ioctl用
	memset(&ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq.ifr_name, device, sizeof(ifreq.ifr_name) - 1);

	// SIOCGIFINDEX : interface index を取得する
	if(ioctl(soc, SIOCGIFINDEX, &ifreq) < 0)
	{
		perror("ioctl");
		close(soc);
		return -1;
	}

	// ---------------------------------------
	// bind
	{
		sockaddr_ll sa;  // リンクレベルヘッダ情報用
		sa.sll_family = PF_PACKET;
		sa.sll_protocol = htons(ETH_P_ALL);	
		sa.sll_ifindex = ifreq.ifr_ifindex;  // sll_ifindex : インターフェイス index
		if (bind(soc, (sockaddr*)&sa, sizeof(sa)) < 0)
		{
			perror("bind");
			close(soc);
			return -1;
		}
	}

	// ---------------------------------------
	if (bPromiscFlag)
	{
		// SIOCGIFFLAGS : デバイスの active フラグワードを取得
		if(ioctl(soc, SIOCGIFFLAGS, &ifreq) < 0)
		{
			perror("ioctl");
			close(soc);
			return -1;
		}

		ifreq.ifr_flags = ifreq.ifr_flags | IFF_PROMISC;
		// SIOCSIFFLAGS : デバイスの active フラグワードを設定
		if(ioctl(soc, SIOCSIFFLAGS, &ifreq) < 0)
		{
			perror("ioctl");
			close(soc);
			return -1;
		}
	}

	return soc;
}

// --------------------------------------------------------------------
// my_ether_ntoa_r
// MAC を文字列に変換する（デバッグ用途）
char* my_ether_ntoa_r(const u_char* const hwaddr, char* const pbuf, const socklen_t size)
{
	snprintf(pbuf, size, "%02x:%02x:%02x:%02x:%02x:%02x",
		hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	return pbuf;
}

// --------------------------------------------------------------------
// PrintEtherHeader
// イーサヘッダをデバッグ表示する
void PrintEtherHeader(const ether_header* const eh, FILE* const fp)
{
	char buf[80];
	fprintf(fp, "ether_header----------------------------\n");
	fprintf(fp, "ether_dhost = %s\n", my_ether_ntoa_r(eh->ether_dhost, buf, sizeof(buf)));
	fprintf(fp, "ether_shost = %s\n", my_ether_ntoa_r(eh->ether_shost, buf, sizeof(buf)));

	// ether_type : 16 bit値
	fprintf(fp, "ether_type = %02X", ntohs(eh->ether_type));

	switch (ntohs(eh->ether_type))
	{
	case	ETH_P_IP:
		fprintf(fp,"(IP)\n");
		break;
	case	ETH_P_IPV6:
		fprintf(fp,"(IPv6)\n");
		break;
	case	ETH_P_ARP:
		fprintf(fp,"(ARP)\n");
		break;
	default:
		fprintf(fp,"(unknown)\n");
		break;
	}
}
