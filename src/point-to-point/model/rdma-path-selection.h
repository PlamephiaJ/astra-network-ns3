#ifndef RDMA_PATH_SELECTION_H
#define RDMA_PATH_SELECTION_H

#include <cstdint>

namespace ns3 {

// Port 100 is the backend's legacy/default RDMA destination port. The two
// adjacent reserved ports encode an explicit path choice in the flow's
// 5-tuple, so every data packet and its reverse ACK/NACK carry the same choice.
constexpr uint16_t kDefaultRdmaDestinationPort = 100;
constexpr uint16_t kPinnedRdmaPathPortBase = 101;
constexpr int32_t kUnpinnedRdmaPath = -1;
constexpr int32_t kSupportedPinnedRdmaPaths = 2;

inline uint16_t EncodeRdmaPathInPort(int32_t pathId) {
  return pathId == kUnpinnedRdmaPath
             ? kDefaultRdmaDestinationPort
             : static_cast<uint16_t>(kPinnedRdmaPathPortBase + pathId);
}

inline int32_t DecodeRdmaPathFromPorts(uint16_t sourcePort,
                                       uint16_t destinationPort) {
  if (destinationPort >= kPinnedRdmaPathPortBase &&
      destinationPort <
          kPinnedRdmaPathPortBase + kSupportedPinnedRdmaPaths) {
    return destinationPort - kPinnedRdmaPathPortBase;
  }
  // ACK/NACK packets reverse the data packet's source/destination ports.
  if (sourcePort >= kPinnedRdmaPathPortBase &&
      sourcePort < kPinnedRdmaPathPortBase + kSupportedPinnedRdmaPaths) {
    return sourcePort - kPinnedRdmaPathPortBase;
  }
  return kUnpinnedRdmaPath;
}

} // namespace ns3

#endif // RDMA_PATH_SELECTION_H
