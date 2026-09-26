#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include <unordered_map>
#include <map>
#include <set>
#include <tuple>
#include <ns3/node.h>
#include <ns3/callback.h>
#include "qbb-net-device.h"
#include "switch-mmu.h"
#include "pint.h"

namespace ns3 {

class Packet;

enum class FlowRoutingStrategy {
	ECMP,
	UGAL_L,
};
typedef Callback<void, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t,
	bool, uint64_t>
	UgalLRouteDecisionCallback;

class SwitchNode : public Node{
	// One loopback plus up to 64 data ports. The bundled topologies use at most
	// 24 data ports; dfly(4,8,4,33) uses 15. Keeping this at 1025 allocates a
	// 32 MiB ingress/egress counter matrix per switch.
	static const uint32_t pCnt = 65;
	static const uint32_t qCnt = 8;	// Number of queues/priorities used
	uint32_t m_ecmpSeed;
	std::unordered_map<uint32_t, std::vector<int> > m_rtTable; // map from ip address (u32) to possible ECMP port (index of dev)
	std::unordered_map<uint32_t, uint32_t> m_nextHopNode;
	bool m_routeLabelLogging;
	std::map<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t>, int>
		m_labeledFlowOutDev;
	std::set<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t> >
		m_loggedLabeledFlows;
	std::map<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t>, int>
		m_ecmpFlowOutDev;
	std::set<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t> >
		m_loggedEcmpFlows;
	struct UgalLRoute {
		int outDev;
		uint32_t nextHopNodeId;
		uint32_t minimalHops;
		uint32_t nonminimalHops;
		uint64_t nonminimalExtraRtt;
	};
	FlowRoutingStrategy m_flowRoutingStrategy;
	uint64_t m_ugalLBiasBytes;
	std::unordered_map<uint32_t, UgalLRoute> m_ugalLRoutes;
	std::map<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t>, int>
		m_ugalLFlowOutDev;
	std::set<std::tuple<uint32_t, uint32_t, uint16_t, uint16_t> >
		m_loggedUgalLFlows;

	UgalLRouteDecisionCallback m_ugalLRouteDecisionCallback;
	uint64_t GetEgressQueueBytes(int outDev) const;

	// monitor of PFC
	uint32_t m_bytes[pCnt][pCnt][qCnt]; // m_bytes[inDev][outDev][qidx] is the bytes from inDev enqueued for outDev at qidx
	
	uint64_t m_txBytes[pCnt]; // counter of tx bytes

	uint32_t m_lastPktSize[pCnt];
	uint64_t m_lastPktTs[pCnt]; // ns
	double m_u[pCnt];

protected:
	bool m_ecnEnabled;
	uint32_t m_ccMode;
	uint64_t m_maxRtt;

	uint32_t m_ackHighPrio; // set high priority for ACK/NACK

private:
	int GetOutDev(Ptr<const Packet>, CustomHeader &ch);
	void SendToDev(Ptr<Packet>p, CustomHeader &ch);
	static uint32_t EcmpHash(const uint8_t* key, size_t len, uint32_t seed);
	void CheckAndSendPfc(uint32_t inDev, uint32_t qIndex);
	void CheckAndSendResume(uint32_t inDev, uint32_t qIndex);
public:
	Ptr<SwitchMmu> m_mmu;

	static TypeId GetTypeId (void);
	SwitchNode();
	void SetEcmpSeed(uint32_t seed);
	void AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx,
	                   uint32_t nextHopNodeId);
	void SetFlowRoutingStrategy(FlowRoutingStrategy strategy,
	                            uint64_t ugalLBiasBytes = 0);
	void SetRoutingDecisionLogging(bool enabled);
	void SetUgalLRouteDecisionCallback(
		UgalLRouteDecisionCallback callback);
	void AddUgalLRoute(Ipv4Address &dstAddr, uint32_t intf_idx,
	                   uint32_t nextHopNodeId, uint32_t minimalHops,
	                   uint32_t nonminimalHops, uint64_t nonminimalExtraRtt);
	void ClearTable();
	bool SwitchReceiveFromDevice(Ptr<NetDevice> device, Ptr<Packet> packet, CustomHeader &ch);
	void SwitchNotifyDequeue(uint32_t ifIndex, uint32_t qIndex, Ptr<Packet> p);

	// for approximate calc in PINT
	int logres_shift(int b, int l);
	int log2apprx(int x, int b, int m, int l); // given x of at most b bits, use most significant m bits of x, calc the result in l bits
};

} /* namespace ns3 */

#endif /* SWITCH_NODE_H */
