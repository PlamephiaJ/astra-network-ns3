#ifndef RDMA_ROUTING_LABEL_H
#define RDMA_ROUTING_LABEL_H

#include <cstdint>

namespace ns3 {

// The routing policy assigns a semantic label; this transport encoding carries
// that label unchanged in the flow tuple. The switch only maps 0 to its short
// next hop and 1 to its long next hop.
constexpr uint16_t kDefaultRdmaDestinationPort = 100;
constexpr uint16_t kRoutingLabelPortBase = 101;
constexpr int32_t kDefaultRoutingLabel = -1;
constexpr int32_t kShortRoutingLabel = 0;
constexpr int32_t kLongRoutingLabel = 1;
constexpr int32_t kSupportedRoutingLabels = 2;

inline bool IsSupportedRoutingLabel(int32_t routingLabel) {
  return routingLabel == kDefaultRoutingLabel ||
         (routingLabel >= kShortRoutingLabel &&
          routingLabel < kSupportedRoutingLabels);
}

inline const char *RoutingLabelName(int32_t routingLabel) {
  if (routingLabel == kShortRoutingLabel) {
    return "short";
  }
  if (routingLabel == kLongRoutingLabel) {
    return "long";
  }
  return "default";
}

inline uint16_t EncodeRoutingLabelInPort(int32_t routingLabel) {
  return routingLabel == kDefaultRoutingLabel
             ? kDefaultRdmaDestinationPort
             : static_cast<uint16_t>(kRoutingLabelPortBase + routingLabel);
}

inline int32_t DecodeRoutingLabelFromPorts(uint16_t sourcePort,
                                           uint16_t destinationPort) {
  if (destinationPort >= kRoutingLabelPortBase &&
      destinationPort < kRoutingLabelPortBase + kSupportedRoutingLabels) {
    return destinationPort - kRoutingLabelPortBase;
  }
  // ACK/NACK packets reverse the data packet's source/destination ports.
  if (sourcePort >= kRoutingLabelPortBase &&
      sourcePort < kRoutingLabelPortBase + kSupportedRoutingLabels) {
    return sourcePort - kRoutingLabelPortBase;
  }
  return kDefaultRoutingLabel;
}

}  // namespace ns3

#endif  // RDMA_ROUTING_LABEL_H
