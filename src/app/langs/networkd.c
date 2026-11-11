/* systemd-networkd */
#include "../lang_def.h"

LANGUAGE(networkd) {
	.name = "networkd",
	.title = "systemd-networkd",
	.kind = K_KEYVAL,
	.extensions = "link network netdev",

	.keywords = WORDS(
		"false", "no", "off", "on", "true", "yes"),

	.preproc = WORDS(
		"ARPAllTargets", "ARPIPTargets", "ARPIntervalSec", "ARPProxy",
		"ARPValidate", "AdSelect", "Address", "Alias", "AllSlavesActive",
		"AllowPortToBeRoot", "Architecture", "BindCarrier", "BitsPerSecond",
		"Bond", "Bridge", "BridgeFDB", "Broadcast", "ClientIdentifier",
		"CopyDSCP", "Cost", "CriticalConnection", "DHCP", "DHCPServer", "DNS",
		"DNSSEC", "DNSSECNegativeTrustAnchors", "DefaultLeaseTimeSec",
		"Description", "Destination", "DestinationPort", "DiscoverPathMTU",
		"Domains", "DownDelaySec", "Driver", "Duplex", "EmitDNS", "EmitNTP",
		"EmitTimezone", "EncapsulationLimit", "FDBAgeingSec",
		"FailOverMACPolicy", "FastLeave", "ForwardDelaySec", "Gateway",
		"GratuitousARP", "Group", "GroupPolicyExtension", "HairPin",
		"HelloTimeSec", "Host", "Hostname", "IPForward", "IPMasquerade",
		"IPVLAN", "IPv4LLRoute", "IPv6AcceptRouterAdvertisements",
		"IPv6DuplicateAddressDetection", "IPv6FlowLabel", "IPv6HopLimit",
		"IPv6PrivacyExtensions", "IPv6Token", "Id", "KernelCommandLine", "Kind",
		"L2MissNotification", "L3MissNotification", "LACPTransmitRate", "LLDP",
		"LLMNR", "Label", "LearnPacketIntervalSec", "Link",
		"LinkLocalAddressing", "Local", "MACAddress", "MACAddressPolicy",
		"MACVLAN", "MACVTAP", "MIIMonitorSec", "MTUBytes", "MacLearning",
		"Match", "MaxAgeSec", "MaxLeaseTimeSec", "MaximumFDBEntries", "Metric",
		"MinLinks", "Mode", "MultiQueue", "MulticastDNS", "NTP", "Name",
		"NamePolicy", "NetDev", "Network", "OneQueue", "OriginalName",
		"PacketInfo", "PacketsPerSlave", "Path", "Peer", "PoolOffset",
		"PoolSize", "PortRange", "PreferredSource", "PrimaryReselectPolicy",
		"Remote", "RequestBroadcast", "ResendIGMP", "Route", "RouteMetric",
		"RouteShortCircuit", "Scope", "SendHostname", "Source", "TOS", "TTL",
		"Tap", "Timezone", "TransmitHashPolicy", "Tun", "Tunnel", "Type",
		"UDP6ZeroCheckSumRx", "UDP6ZeroChecksumTx", "UDPCheckSum",
		"UnicastFlood", "UpDelaySec", "UseBPDU", "UseDNS", "UseDomains",
		"UseHostname", "UseMTU", "UseNTP", "UseRoutes", "UseTimezone", "User",
		"VLAN", "VLANId", "VNetHeader", "VXLAN", "VendorClassIdentifier",
		"Virtualization", "WakeOnLan"),

	.line_comments = { ";#" },
	.quotes = "'\"",
};
