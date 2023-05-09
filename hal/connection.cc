//---------------------------------------------------------------------------

#include "connection.h"
#include "../console.h"
#include "../split/split_usb_status.h"
#include "ble.h"
#include "usb_status.h"

//---------------------------------------------------------------------------

ConnectionId Connection::preferredConnections[3] = {
    ConnectionId::USB,
    ConnectionId::USB_PAIR,
    ConnectionId::BLE,
};

//---------------------------------------------------------------------------

__attribute__((weak)) bool Connection::IsConnected(ConnectionId connectionId) {
  switch (connectionId) {
  case ConnectionId::ACTIVE:
    return Ble::IsConnected() || UsbStatus::instance.IsConnected() ||
           SplitUsbStatus::instance.IsConnected();
  case ConnectionId::BLE:
    return Ble::IsConnected();
  case ConnectionId::USB:
    return UsbStatus::instance.IsConnected();
  case ConnectionId::USB_PAIR:
    return SplitUsbStatus::instance.IsConnected();
  }
  return false;
}

__attribute__((weak)) ConnectionId Connection::GetActiveConnection() {
  for (ConnectionId connectionId : preferredConnections) {
    switch (connectionId) {
    case ConnectionId::ACTIVE:
      continue;
    case ConnectionId::BLE:
      if (Ble::IsConnected()) {
        return ConnectionId::BLE;
      }
      continue;
    case ConnectionId::USB:
      if (UsbStatus::instance.IsConnected()) {
        return ConnectionId::USB;
      }
      continue;
    case ConnectionId::USB_PAIR:
      if (SplitUsbStatus::instance.IsConnected()) {
        return ConnectionId::USB_PAIR;
      }
      continue;
    }
  }
  return ConnectionId::ACTIVE;
}

__attribute__((weak)) void
Connection::SetPreferredConnection(ConnectionId first, ConnectionId second,
                                   ConnectionId third) {
  preferredConnections[0] = first;
  preferredConnections[1] = second;
  preferredConnections[2] = third;
}

__attribute__((weak)) bool
Connection::IsPairConnected(PairConnectionId pairConnectionId) {
  switch (pairConnectionId) {
  case PairConnectionId::ACTIVE:
    return Split::IsPairConnected() || Ble::IsPairConnected();
  case PairConnectionId::BLE:
    return Ble::IsPairConnected();
  case PairConnectionId::CABLE:
    return Split::IsPairConnected();
  }
  return false;
}

__attribute__((weak)) PairConnectionId Connection::GetActivePairConnection() {
  if (Split::IsPairConnected()) {
    return PairConnectionId::CABLE;
  }
  if (Ble::IsPairConnected()) {
    return PairConnectionId::BLE;
  }
  return PairConnectionId::ACTIVE;
}

__attribute__((weak)) bool Connection::IsHostSleeping() {
  ConnectionId activeConnectionId = GetActiveConnection();
  switch (activeConnectionId) {
  case ConnectionId::ACTIVE:
    return false;
  case ConnectionId::BLE:
    return Ble::IsSleeping();
  case ConnectionId::USB:
    return UsbStatus::instance.IsSleeping();
  case ConnectionId::USB_PAIR:
    return SplitUsbStatus::instance.IsSleeping();
  }
  return false;
}

//---------------------------------------------------------------------------

void Connection::PrintInfo() {
#if JAVELIN_BLE || JAVELIN_SPLIT
  static const char *const NAMES[] = {"none", "ble", "usb", "usb2"};

  Console::Printf("Connections\n");
#if JAVELIN_BLE
  Console::Printf("  BLE: %s\n",
                  Ble::IsConnected()
                      ? Ble::IsSleeping() ? "true (asleep)" : "true"
                      : "false");
#endif
  Console::Printf("  USB: %s\n", UsbStatus::instance.IsConnected()
                                     ? UsbStatus::instance.IsSleeping()
                                           ? "true (asleep)"
                                           : "true"
                                     : "false");
#if JAVELIN_SPLIT
  Console::Printf("  USB2: %s\n", SplitUsbStatus::instance.IsConnected()
                                      ? SplitUsbStatus::instance.IsSleeping()
                                            ? "true (asleep)"
                                            : "true"
                                      : "false");
#endif
  Console::Printf("  Active: %s\n", NAMES[(size_t)GetActiveConnection()]);

#if JAVELIN_BLE
  Console::Printf("  BLE Pair: %s\n",
                  Ble::IsPairConnected() ? "true" : "false");
#endif
#if JAVELIN_SPLIT
  Console::Printf("  Cable Pair: %s\n",
                  Split::IsPairConnected() ? "true" : "false");
#endif

#endif
}

//---------------------------------------------------------------------------
