//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

enum class ConnectionId : uint8_t {
  ACTIVE,
  BLE,
  USB,
  USB_PAIR,
};

enum class PairConnectionId : uint8_t {
  ACTIVE,
  BLE,
  CABLE,
};

class Connection {
public:
  static bool IsConnected(ConnectionId connectionId);
  static ConnectionId GetActiveConnection();
  static void SetPreferredConnection(ConnectionId first, ConnectionId second,
                                     ConnectionId third);
  static bool IsPairConnected(PairConnectionId pairConnectionId);
  static PairConnectionId GetActivePairConnection();

  // This is based on the currently active connection.
  static bool IsHostSleeping();

  static void PrintInfo();

  static ConnectionId preferredConnections[];
};

//---------------------------------------------------------------------------