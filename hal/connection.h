//---------------------------------------------------------------------------

#pragma once
#include "../keyboard_led_status.h"
#include <stdint.h>

//---------------------------------------------------------------------------

enum class ConnectionId : uint8_t {
  NONE,
  ACTIVE = NONE,

  BLE,
  USB,
  USB_PAIR,
};

enum class PairConnectionId : uint8_t {
  NONE,
  ACTIVE = NONE,

  BLE,
  CABLE,
};

class Connection {
public:
  static bool IsConnected(ConnectionId connectionId = ConnectionId::ACTIVE);
  static ConnectionId GetActiveConnection();
  static void SetPreferredConnection(ConnectionId first, ConnectionId second,
                                     ConnectionId third);
  static bool
  IsPairConnected(PairConnectionId pairConnectionId = PairConnectionId::ACTIVE);
  static PairConnectionId GetActivePairConnection();

  static KeyboardLedStatus GetActiveKeyboardLedStatus();

  // This is based on the currently active connection.
  static bool IsHostSleeping(ConnectionId connectionId = ConnectionId::ACTIVE);

  static void PrintInfo();

  static ConnectionId preferredConnections[];
};

//---------------------------------------------------------------------------
