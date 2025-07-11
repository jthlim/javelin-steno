# Javelin Script Reference

Javelin uses scripts to customize all behavior, including button processing,
RGB lighting and screen displays.

# Language Reference

Javelin Script is a C-like syntax language that is very rudimentary by design.
It generates an intermediate bytecode, that can be inspected in the Web Tools ->
Script Tool -> Edit Script -> View Disassembly.

## Values

Values are untyped in javelin-script -- there are no compile time checks
ensure that values of the right type are being used.

A value could be:

- An integer value.

  - `1234` (decimal)
  - `0x1234` (hex)

- A pointer to a string.

  - `"Test"`

- A pointer to a byte list.

  - `[[00 ff 12 34]]`
  - Accessing individual bytes in a byte list is done with `[n]`
    - `byteList[i]`

- A pointer to a function
  - Named function: `@functionName`
  - Anonymous function: `@{ <instructions> }`
  - Anonymous function with parameters: `@(a, b) { <instructions> }`
  - Anonymous function with return value: `@var { <instructions> }`
  - Anonymous function with parameters and return value: `@(a, b) var { <instructions> }`
  - The programmer will need to ensure pointers to functions are called
    with the right number of parameters, and that return values are used;
    no checks are provided by the compiler.

- A pointer to unsigned 16-bit values, which includes string, byte list and
  function pointers.

  - `[< "string1", [[11 22 33]], @func1 >]`
  - Accessing individual elements in a 16-bit value list is done using `[<n>]`
    - `stringTable[<i>]`

There are no explicit boolean values -- for conditions (e.g. `if` statements)
only the value 0 is considered falsy. All other values are truthy.

Floating point numbers are not supported as the rp2040 does not have
floating point hardware.

## Constants

- `const constName = <expr>;`

Constants are fixed at compile-time, and the value they are given must be
determinable at the point of declaration.

## Variables

- `var varName;`
- `var varName = <expr>;`
- `var varName[<SIZE>];`

Array sizes must be constant at the point of declaration.

Variables can be declared at global or local scope.

- Global variables can be inspected and changed in Global Values tool,
  available from the Visual Editor menu.
- Global values start with the initial value of 0.
- Local variables cannot be automatically inspected, but the `printValue`
  or `printData` functions can be used to help debug if necessary. These
  will show in the Console tool only if the Show All Data setting is
  enabled.
- Local variables that are not assigned a value have an undefined value.

## Functions

- `func funcName(params) { ... }`
- `func funcWithReturnValueName(params) var { ... }`

Example:

```go
  func isValid(x, y) var {
    return x != 10 && x != 20 && x != y;
  }
```

## Flow Control

- `if (<condition>) { ... }`
- `if (<condition>) { ... } else if (<condition>) { .... } else { ... }`
- `for (<initializer>; <condition>; <update>) { ... }`
- `while (<condition>) { ... }`
- `do { ... } while (<condition>);`
- `break`
- `continue`

Single statements can be used instead of blocks (`{ ... }`) for flow control
statements.

## Operators

- `+` Add
- `-` Negate / Subtract
- `*` Multiply
- `/` Quotient
- `%` Remainder
- `~` Bitwise Not
- `&` Bitwise And
- `^` Bitwise Xor
- `|` Bitwise Or
- `!` Logical Not
- `&&` Logical And
- `||` Logical Or
- `<<` Shift Left
- `>>` Arithmetic Shift Right
- `>>>` Logical Shift Right
- `<`, `<=`, `>`, `>=`, `==`, `!=` Comparisons
- `[<index expr>]` Sub-element access, 0-based.
  - `a[x]` will look up the xth byte in a byte list, unless `a` is
    a global or local variable name.
  - No bounds checking is performed on _index_
- `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `<<=`, `>>=`, `>>>=` syntactic
  shortcuts are supported with some limitations.

The fallback operator `<identifier> ?? <expr>` specifies to use the
right hand _expr_ if the identifier is not defined.

`&&` and `||` are short circuiting, meaning that if the left hand side is
sufficient to determine the result, the right hand side will not be evaluated.

# Function Reference

## Control Functions

- `func pressScanCode(<scanCode>)`

  - _scanCode_ constants can be found below.
  - Any pressed scan code should be accompanied by a `releaseScanCode`
    at some point in the future.

- `func releaseScanCode(<scanCode>)`

  - Releasing a scan code that is not already pressed is a no-op.

- `func tapScanCode(<scanCode>)`

  - Triggers a press and release of _scanCode_ immediately.
  - Note that on macOS, Caps Lock will not work with a tap, as it needs
    to be held for a certain amount of time before it activates. This can be handled with:
    ```
    pressScanCode(SC_CAPS);
    startTimer(<timerId>, 200, 0, @{ releaseScanCode(SC_CAPS); });
    ```

- `func isScanCodePressed(<scanCode>) var`

  - Returns whether _scanCode_ is currently pressed.

- `func moveMouse(<dx>, <dy>)`

  - _dx_ and _dy_ are values between -32768 and 32767 inclusive.

- `func vWheelMouse(<delta>)`

  - Vertical mouse wheel. _delta_ is a value between -32768 and 32767 inclusive.

- `func hWheelMouse(<delta>)`

  - Horizontal mouse wheel. _delta_ is a value between -32768 and 32767 inclusive.

- `func pressMouseButton(<buttonIndex>)`

- `func releaseMouseButton(<buttonIndex>)`

- `func tapMouseButton(<buttonIndex>)`

- `func isMouseButtonPressed(<buttonIndex>) var`

- `func sendText("Example")`

  - Sends all of the key presses required to emit the specified string.
  - e.g.,
    - Press Shift
    - Press E
    - Release E
    - Release Shift
    - Press X
    - Release X
    - Press A
    - Release A
    - etc.
  - These use the active host layout information to convert unicode to key
    presses.

- `func sendMidi(<command>, <param1>, <param2>)`

  _command_                      | _param1_     | _param2_
  -------------------------------|--------------|---------
  `0x80-0x8f`: Note off          | Key          | Off Velocity
  `0x90-0x9f`: Note on           | Key          | On Velocity
  `0xa0-0xaf`: Aftertouch        | Key          | Pressure
  `0xb0-0xbf`: Control Change    | Controller   | Value
  `0xc0-0xbf`: Program Change    | Program      | 0 (unused)
  `0xd0-0xbf`: Channel Pressure  | Pressure     | 0 (unused)
  `0xe0-0xef`: Pitch Bend        | LSB (7 bits) | MSB (7 bits)
  `0xf0-0xff`: System            |              |

  - `sendMidi` has been implemented on pico devices over USB and Jarne over
    USB and BLE.
  - Users who paired their Jarne before MIDI support was added will need to
    delete the pairing on their host and re-pair BLE to get MIDI
    functionality working.

### Constants

```
const SC_NONE = 0;

const SC_A = 0x04;
const SC_B = 0x05;
const SC_C = 0x06;
const SC_D = 0x07;
const SC_E = 0x08;
const SC_F = 0x09;
const SC_G = 0x0a;
const SC_H = 0x0b;
const SC_I = 0x0c;
const SC_J = 0x0d;
const SC_K = 0x0e;
const SC_L = 0x0f;
const SC_M = 0x10;
const SC_N = 0x11;
const SC_O = 0x12;
const SC_P = 0x13;
const SC_Q = 0x14;
const SC_R = 0x15;
const SC_S = 0x16;
const SC_T = 0x17;
const SC_U = 0x18;
const SC_V = 0x19;
const SC_W = 0x1a;
const SC_X = 0x1b;
const SC_Y = 0x1c;
const SC_Z = 0x1d;

const SC_1 = 0x1e;
const SC_2 = 0x1f;
const SC_3 = 0x20;
const SC_4 = 0x21;
const SC_5 = 0x22;
const SC_6 = 0x23;
const SC_7 = 0x24;
const SC_8 = 0x25;
const SC_9 = 0x26;
const SC_0 = 0x27;

const SC_ENTER = 0x28;
const SC_ESC = 0x29;
const SC_BACKSPACE = 0x2a;
const SC_TAB = 0x2b;
const SC_SPACE = 0x2c;
const SC_MINUS = 0x2d;
const SC_EQUAL = 0x2e;
const SC_L_BRACKET = 0x2f;
const SC_R_BRACKET = 0x30;
const SC_BACKSLASH = 0x31;
const SC_HASH_TILDE = 0x32;
const SC_SEMICOLON = 0x33;
const SC_APOSTROPHE = 0x34;
const SC_GRAVE = 0x35;
const SC_COMMA = 0x36;
const SC_DOT = 0x37;
const SC_SLASH = 0x38;
const SC_CAPS = 0x39;

const SC_F1 = 0x3a;
const SC_F2 = 0x3b;
const SC_F3 = 0x3c;
const SC_F4 = 0x3d;
const SC_F5 = 0x3e;
const SC_F6 = 0x3f;
const SC_F7 = 0x40;
const SC_F8 = 0x41;
const SC_F9 = 0x42;
const SC_F10 = 0x43;
const SC_F11 = 0x44;
const SC_F12 = 0x45;

const SC_SYS_RQ = 0x46;
const SC_SCROLL_LOCK = 0x47;
const SC_PAUSE = 0x48;
const SC_INSERT = 0x49;
const SC_HOME = 0x4a;
const SC_PAGE_UP = 0x4b;
const SC_DELETE = 0x4c;
const SC_END = 0x4d;
const SC_PAGE_DOWN = 0x4e;
const SC_RIGHT = 0x4f;
const SC_LEFT = 0x50;
const SC_DOWN = 0x51;
const SC_UP = 0x52;

const SC_NUM_LOCK = 0x53;
const SC_KP_SLASH = 0x54;
const SC_KP_ASTERISK = 0x55;
const SC_KP_MINUS = 0x56;
const SC_KP_PLUS = 0x57;
const SC_KP_ENTER = 0x58;
const SC_KP_1 = 0x59;
const SC_KP_2 = 0x5a;
const SC_KP_3 = 0x5b;
const SC_KP_4 = 0x5c;
const SC_KP_5 = 0x5d;
const SC_KP_6 = 0x5e;
const SC_KP_7 = 0x5f;
const SC_KP_8 = 0x60;
const SC_KP_9 = 0x61;
const SC_KP_0 = 0x62;
const SC_KP_DOT = 0x63;

const SC_BACKSLASH_PIPE = 0x64;
const SC_COMPOSE = 0x65;
const SC_POWER = 0x66;
const SC_KP_EQUAL = 0x67;

const SC_F13 = 0x68;
const SC_F14 = 0x69;
const SC_F15 = 0x6a;
const SC_F16 = 0x6b;
const SC_F17 = 0x6c;
const SC_F18 = 0x6d;
const SC_F19 = 0x6e;
const SC_F20 = 0x6f;
const SC_F21 = 0x70;
const SC_F22 = 0x71;
const SC_F23 = 0x72;
const SC_F24 = 0x73;

const SC_OPEN = 0x74;
const SC_HELP = 0x75;
const SC_MENU = 0x76;
const SC_SELECT = 0x77;
const SC_STOP = 0x78;
const SC_AGAIN = 0x79;
const SC_UNDO = 0x7a;
const SC_CUT = 0x7b;
const SC_COPY = 0x7c;
const SC_PASTE = 0x7d;
const SC_FIND = 0x7e;
const SC_MUTE = 0x7f;
const SC_VOLUME_UP = 0x80;
const SC_VOLUME_DOWN = 0x81;
const SC_KP_COMMA = 0x85;

const SC_L_CTRL = 0xe0;
const SC_L_SHIFT = 0xe1;
const SC_L_ALT = 0xe2;
const SC_L_META = 0xe3;
const SC_R_CTRL = 0xe4;
const SC_R_SHIFT = 0xe5;
const SC_R_ALT = 0xe6;
const SC_R_META = 0xe7;

// From Consumer Page (0x0c)
const SC_CONSUMER_PLAY = 0xa0;                // UsageId 0xb0
const SC_CONSUMER_PAUSE = 0xa1;               // UsageId 0xb1
const SC_CONSUMER_RECORD = 0xa2;              // UsageId 0xb2
const SC_CONSUMER_FAST_FORWARD = 0xa3;        // UsageId 0xb3
const SC_CONSUMER_REWIND = 0xa4;              // UsageId 0xb4
const SC_CONSUMER_SCAN_NEXT_TRACK = 0xa5;     // UsageId 0xb5
const SC_CONSUMER_SCAN_PREVIOUS_TRACK = 0xa6; // UsageId 0xb6
const SC_CONSUMER_STOP = 0xa7;                // UsageId 0xb7
const SC_CONSUMER_EJECT = 0xa8;               // UsageId 0xb8

const SC_CONSUMER_STOP_EJECT = 0xbc;          // UsageId 0xcc
const SC_CONSUMER_PLAY_PAUSE = 0xbd;          // UsageId 0xcd
const SC_CONSUMER_PLAY_SKIP = 0xbe;           // UsageId 0xce

const SC_CONSUMER_MUTE = 0xd2;                // UsageId 0xe2
const SC_CONSUMER_VOLUME_UP = 0xd9;           // UsageId 0xe9
const SC_CONSUMER_VOLUME_DOWN = 0xda;         // UsageId 0xea

const SC_CONSUMER_BRIGHTNESS_UP = 0xde,       // UsageId 0x6f
const SC_CONSUMER_BRIGHTNESS_DOWN = 0xdf,     // UsageId 0x70

const MOUSE_BUTTON_LEFT = 0;
const MOUSE_BUTTON_RIGHT = 1;
const MOUSE_BUTTON_MIDDLE = 2;
// Mouse buttons 3-31 can also be used in the functions.
```

## Stenography Functions

- `func pressStenoKey(<stenoKey>)`

- `func releaseStenoKey(<stenoKey>)`

- `func isStenoKeyPressed(<stenoKey>) var`

- `func cancelAllStenoKeys()`

- `func cancelStenoKey(<stenoKey>)`

- `func isStenoJoinNext() var`
  - Returns whether the next input will join to the previous without
    adding whitespace.

### Constants

```
const SK_NONE = -1;
const SK_S1 = 0;
const SK_S2 = 1;
const SK_TL = 2;
const SK_KL = 3;
const SK_PL = 4;
const SK_WL = 5;
const SK_HL = 6;
const SK_RL = 7;
const SK_A = 8;
const SK_O = 9;
const SK_STAR1 = 10;
const SK_STAR2 = 11;
const SK_STAR3 = 12;
const SK_STAR4 = 13;
const SK_E = 14;
const SK_U = 15;
const SK_FR = 16;
const SK_RR = 17;
const SK_PR = 18;
const SK_BR = 19;
const SK_LR = 20;
const SK_GR = 21;
const SK_TR = 22;
const SK_SR = 23;
const SK_DR = 24;
const SK_ZR = 25;
const SK_NUM1 = 26;
const SK_NUM2 = 27;
const SK_NUM3 = 28;
const SK_NUM4 = 29;
const SK_NUM5 = 30;
const SK_NUM6 = 31;
const SK_NUM7 = 32;
const SK_NUM8 = 33;
const SK_NUM9 = 34;
const SK_NUM10 = 35;
const SK_NUM11 = 36;
const SK_NUM12 = 37;
const SK_FUNCTION = 38;
const SK_POWER = 39;
const SK_RES1 = 40;
const SK_RES2 = 41;
const SK_X1 = 38;
const SK_X2 = 39;
const SK_X3 = 40;
const SK_X4 = 41;
const SK_X5 = 42;
const SK_X6 = 43;
const SK_X7 = 44;
const SK_X8 = 45;
const SK_X9 = 46;
const SK_X10 = 47;
const SK_X11 = 48;
const SK_X12 = 49;
const SK_X13 = 50;
const SK_X14 = 51;
const SK_X15 = 52;
const SK_X16 = 53;
const SK_X17 = 54;
const SK_X18 = 54;
const SK_X19 = 56;
const SK_X20 = 57;
const SK_X21 = 58;
const SK_X22 = 59;
const SK_X23 = 60;
const SK_X24 = 61;
const SK_X25 = 62;
const SK_X26 = 63;
```

## Support functions

- `func pressAll()`

  - Calls all press scripts for buttons that are pressed.

- `func isInPressAll() var`

  - Returns whether the function is currently within a `pressAll()`

- `func releaseAll()`

  - Releases all pressed scan codes and steno keys.
  - This does not call any scripts

- `func callAllReleaseScripts()`

  - Calls all release scripts for buttons that are pressed.

- `func isInReleaseAll() var`

  - Returns whether the function is currently within a `callAllReleaseScripts()`.

- `func pressButton(<buttonIndex>)`

  - Simulates _buttonIndex_ being pressed.
  - The script function will be called, and `isButtonPressed` will return true
  - for that index.

- `func releaseButton(<buttonIndex>)`

  - Simulates _buttonIndex_ being released.
  - The script function will be called, and `isButtonPressed` will return false
  - for that index.

## Button State Functions

- `func isButtonPressed(buttonIndex) var`

  - Returns if the physical button is pressed.

- `func checkButtonState("01 10") var`

  - Returns if the current button state matches the string.
  - 0 = not pressed, space = ignore, all others = pressed.
    - The example string checks that:
      - button 0 is off.
      - button 1 is on.
      - button 3 is on.
      - button 4 is off.
  - The string should be the same length as the number of buttons.

- `func getPressCount()`

  - Returns the number of times buttons have been pressed since startup.

- `func getReleaseCount()`
  - Returns the number of times buttons have been released since startup.

## RGB Functions

- `func setRgb(id, r, g, b)`

  - For boards with rgb lights, sets an individual light to r, g, b.

- `func setHsv(id, h, s, v)`
  - For boards with rgb lights, sets an individual light to h, s, v.
    - h = hue, 0-65536 represents 0° - 360°
      - Internally wraps if values out of range provided.
    - s = saturation, 0-256 represents 0.0 - 1.0
    - v = value, 0-255 represents 0.0 - 1.0

- `func enableScriptRgb()`
- `func disableScriptRgb()`

  - Controls whether setRgb/setHsv commands are performed. This is to allow
    external control of RGB.
  - When disabled, only external scripts or console commands will take effect.

By convention, boards with per-key rgb lights have each key rgb id match
the button index, then underglow lights after.

## Drawing Functions

- `func clearDisplay(<displayId>)`

  - Sets _displayId_ to a blank canvas.
  - _displayId_ is 0 for single screen keyboards.
  - _displayId_ is 0 for main side; and 1 for pair side of split keyboards.

- `func setDrawColor(<displayId>, <color>)`

  - Sets the current draw color for _displayId_.
  - For monochrome displays, 0 is empty, 1 is set.

- `func setAutoDraw(<displayId>, <autoDraw>)`
  - Sets _displayId_ to be auto-drawn by Javelin.
  - Constants:
    ```
    const AUTO_DRAW_NONE = 0;
    const AUTO_DRAW_PAPER_TAPE = 1;
    const AUTO_DRAW_STENO_LAYOUT = 2;
    const AUTO_DRAW_WPM = 3;
    ```
- `func setScreenOn(<displayId>, <on>)`

  - Enables screen display.

- `func setScreenContrast(<displayId>, <contrast>)`

  - Sets the screen contrast from 0-255

- `func drawPixel(<displayId>, <x>, <y>)`

  - Sets the pixel at (_x_, _y_) to the current draw color

- `func drawLine(<displayId>, <x1>, <y1>, <x2>, <y2>)`

  - Draws a line from (x1, y1) to (x2, y2) using the current draw color.

- `func drawImage(<displayId>, <x>, <y>, <image>)`

  - Draws an image at (x, y) using the current draw color.

- `func drawText(<displayId>, <x>, <y>, <fontId>, <alignment>, <text>)`

  - Constants

    ```
    const TEXT_ALIGNMENT_LEFT = 0;
    const TEXT_ALIGNMENT_MIDDLE = 1;
    const TEXT_ALIGNMENT_RIGHT = 2;

    const FONT_ID_NORMAL = 0;
    const FONT_ID_LARGE = 1;
    const FONT_ID_DOS = 2;
    const FONT_ID_SMALL_DIGITS = 3;
    const FONT_ID_MEDIUM_DIGITS = 4;
    const FONT_ID_LARGE_DIGITS = 5;
    const FONT_ID_HUGE_DIGITS = 6;
    ```

- `func measureTextWidth(<fontId>, <text>)`

  - Returns the width of _text_ in pixels.
  - _fontId_ uses the same constants as drawText.

- `func drawRect(<displayId>, <left>, <top>, <right>, <bottom>)`

  - Draws a rectangle using the current draw color

- `func DrawLuminanceRange(<displayId>, <x>, <y>, <data>, <minValue>, <maxValue>)`
  - Draws pixels on the screen when the pixel value is in range
    [_minValue_, _maxValue_).

## Host Connectivity Functions

- `func isConnected(<connectionId>)`

  - Returns whether _connectionId_ is active.
  - Use _CONNECTION_ID_ACTIVE_ to determine if actions will reach any host.

- `func getActiveConnection() var`

  - Returns the connection ID of the active host.

- `func setPreferredConnection(firstPreferenceConnectionId, secondPreferenceConnectionId, thirdPreferenceConnectionId)`

  - On multi-device firmware, sets the preferred order in which to use
    connections.

- `func isHostSleeping() var`

  - Returns true if the current active host is suspended.

- `func startBlePairing()`

  - Initiate pairing for the currently active BLE profile.

- `func isBleAdvertising() var`

  - Returns whether the device is advertising to establish a BLE connection.

- `func isBleScanning() var`

  - Returns whether the device is scanning to accept a BLE connection.

- `func disconnectBle()`

  - Disconnects the currently active BLE profile.

- `func unpairBle()`

  - Disconnects and deletes pairing for the currently active BLE profile.

- `func getBleProfile() var`

  - Returns the BLE profile ID (0-4) that is currently active.

- `func setBleProfile(profileId)`

  - Sets the current BLE profile (0-4)
  - If paired but disconnected, this will also initiate a reconnect.

- `func isBleProfileConnected(profileId) var`

  - Returns whether the BLE _profileId_ is connected.

- `func isBleProfilePaired(profileId) var`

  - Returns whether the BLE _profileId_ is paired.

- `func isBleProfileSleeping(profileId) var`

  - Returns whether the BLE _profileId_'s host is sleeping.

- `func isUsbMounted() var` [deprecated]

  - Returns whether any usb port is connected.
  - Superseded by `isConnected(CONNECTION_ID_ANY)`.

- `func isUsbSuspended() var` [deprecated]
  - Returns whether any usb port is suspended.
  - Superseded by `isHostSleeping()`.

Constants

```
const CONNECTION_ID_NONE = 0;
const CONNECTION_ID_ANY = 0;
const CONNECTION_ID_BLE = 1;
const CONNECTION_ID_USB = 2;
const CONNECTION_ID_USB2 = 3;
```

## Combo Support Functions

Combo support functions make it much easier to configure and handle
multi-button press actions. Calls to underlying button press/release methods
are suppressed until it is confirmed that no combo is involved.

- `func addCombo(isOrdered, comboTimeOut, buttons, pressFunction, releaseFunction)`

  - Adds a combo trigger.
    - Combo triggers suppress calls to button presses/releases until
      _comboTimeOut_ (milliseconds) expires.
    - Using 0 for _comboTimeOut_ represents no timeout.
    - _buttons_ is a byte list terminated in `ff`.
    - If _isOrdered_ is non-zero, the combo is only triggered if buttons are
      pressed in the specified order.
    - _pressFunction_ is called when all of the keys in a combo are pressed.
    - _releaseFunction_ is called when the first key is released.
  - Javelin supports combos that have overlapping definitions, e.g. one
    combo can have buttons 1 & 2, and a second combo can have buttons 1, 2 and 3.
  - Example invocation:
    - `addCombo(0, 50, [[00 10 ff]], @{ sendText("Hi"); }, @{});`
      - This will trigger the text "Hi" when buttons 0 and 16 (10 hex) are
        pressed within 50 ms.
    - `addCombo(1, 50, [[00 10 ff]], @{ sendText("Hi"); }, @{});`
      - This will trigger the text "Hi" when buttons 0 then button 16 (10 hex)
        are pressed within 50 ms.
    - `addCombo(0, 50, [[00 01 02 ff]], @{ pressScanCode(SC_A); }, @{ releaseScanCode(SC_A); });`
      - This will press the letter A when buttons 0, 1 & 2 are pressed within
        50 ms, and release it when the first of these buttons is let go.

- `func resetCombos()`

  - Removes all registered combos.
  - Any combos that were active will have their `releaseFunction` called.

## Pair Connectivity Functions

- `func getActivePairConnection() var`

  - Returns the active pair connection ID
  - Constants
    ```
    const PAIR_CONNECTION_ID_NONE = 0;
    const PAIR_CONNECTION_ID_BLE = 1;
    const PAIR_CONNECTION_ID_CABLE = 2;
    ```

- `func isPairConnected(pairConnectionId) var`
  - Returns whether the pair is connected
  - Constants
    ```
    const PAIR_CONNECTION_ID_ANY = 0;
    const PAIR_CONNECTION_ID_BLE = 1;
    const PAIR_CONNECTION_ID_CABLE = 2;
    ```

## Power Related Functions

- `func isMainPowered() var`

  - Returns whether the main (typically left) side of a split keyboard
    is externally powered.

- `func isPairPowered() var`

  - Returns whether the pair (typically right) side of a split keyboard
    is externally powered.

- `func isCharging() var`

  - Returns whether the current device is being charged.
  - In split keyboards, will return false if the side the script is running
    on not charging, even if the partner side is being charged.

- `func getBatteryPercentage() var`

  - Returns value from 0-100.

- `func setBoardPower(<enable>)`

  - If the board supports separate power domains, control power to RGB lights.

- `func isBoardPowered() var`

  - If the board supports separate power domains, returns whether power is
    being supplied to the RGB lights.

- `func setPairBoardPower(<setting>)`

  - Controls power to the pair side's RGB lights.
  - Constants
    ```
    const PAIR_BOARD_POWER_NO_OVERRIDE = 0;
    const PAIR_BOARD_POWER_FORCE_OFF = 1;
    const PAIR_BOARD_POWER_FORCE_ON = 2;
    ```

## Timer Functions

- `func getTime() var`

  - Returns milliseconds since start up.

- `func startTimer(<timerId>, <delayInMilliseconds>, <isRepeating>, <handler>)`

  - Starts a timer that will call _handler_ after _delayInMilliseconds_.
  - _timerId_ is any chosen unique id for future calls to `stopTimer` and `isTimerActive` and must be positive.
  - Any existing _timerId_ will be replaced by the _handler_ specified.
  - If _isRepeating_ is `1`, the timer should be stopped with `stopTimer`.
  - _handler_ is either:
    - `func handler() { ... }`, or
    - `func handler(<timerId>) { ... }`

- `func stopTimer(<timerId>)`

  - Cancels the specified _timerId_ if active.
  - If no timer with _timerId_ is running, this is a no-op.

- `func isTimerActive(<timerId>) var`
  - Returns whether _timerId_ is currently active.

## GPIO Functions

- `func setGpioPin(<pin>, <zeroOrOne>)`

  - Sets a GPIO _pin_ to 0 or 1.

- `func setGpioPinDutyCycle(<pin>, <dutyCycle>)`

  - _dutyCycle_ ranges from 0 to 100.
  - Sets GPIO _pin_ to be on _dutyCycle_% of the time.

- `func setGpioInputPin(<pin>, <pull>)`

  - Marks a _pin_ for input with specified _pull_ configuration.
  - Constants:
    ```
    const PULL_NONE = 0;
    const PULL_DOWN = 1;
    const PULL_UP = 2;
    ```

- `func readGpioPin(<pin>) var`

  - Returns whether a GPIO _pin_ is logic high level.
  - `setGpioInputPin` must be called before hand for the pin.

## Audio Functions

- `func stopSound()`

  - Stops all sound playback.

- `func playFrequency(<frequencyInHz>)`

  - Plays a sound at the specified _frequencyInHz_.

- `func playSequence(<sequenceByteList>)`

  - Plays a sequence and automatically stops upon completion.
  - A sequence is a series of notes that have the following encoding:
    - noteIndex: 7 bits
    - duration: 9 bits, the duration in 10ms increments.
  - noteIndex is:
    - 00: End of sequence
    - 01: Pause
    - 02+: Note, with middle-C at 50, concert A at 59, each increment is one
      semitone.
  - Example:
    - C8 for 100ms, E8 for 100ms repeated 3 times is:
    - `playSequence([[62 05 66 05 62 05 66 05 62 05 66 05 00 00]])`

- `func playWaveform(<data>)`

  - Placeholder, currently not available on any platform.

## Security Key Functions

These methods are only available on chips with secure storage.

- `func isWaitingForUserPresence() var`

  - Returns whether user presence has been requested.

- `func replyUserPresence(<isPresent>)`
  - Responds whether a user is present or not.
  - `replyUserPresence(1);` is the equivalent of touching other security keys.

## Security Functions

- `func enableConsole()`
- `func disableConsole()`
- `func isConsoleEnabled() var`

  - These functions enable and disable external tools access to the keyboard.

- `func enableFlashWrite()`
- `func disableFlashWrite()`
- `func isFlashWriteEnabled() var`

  - These functions enable and disable updating flash on the keyboard.

## Math Functions

- `func sin(x) var`

  - `x` is a fixed point 16 angle where 65536 represents 360 degrees.
  - Returns a fixed point 16 value (between -65336 and 65336).

- `func cos(x) var`

  - `x` is a fixed point 16 angle where 65536 represents 360 degrees.
  - Returns a fixed point 16 value (between -65336 and 65336).

- `func tan(x) var`

  - `x` is a fixed point 16 angle where 65536 represents 360 degrees.
  - Returns a fixed point 16 value.

- `func asin(x) var`

  - `x` is a fixed point 16 value (between -65336 and 65336).
  - Returns an angle from -32767 to 32768.

- `func acos(x) var`

  - `x` is a fixed point 16 value (between -65336 and 65336).
  - Returns an angle from -32767 to 32768.

- `func atan(x) var`

  - `x` is a fixed point 16 value.
  - Returns an angle from -16384 to 16384.

- `func atan2(y, x) var`

  - Returns an angle from -32767 to 32768.

## Infrared functions
_This section is preliminary. These functions may change without notice._

Javelin provides 3 ways of sending infrared data:
- Sending high level messages using `sendInfraredMessage`
- Sending bits with bit encoding information using `sendInfraredData`
- Sending arbitrary pulses using `sendInfraredSignal`

### Reference

- `func stopInfrared()`

  - Stops all infrared transmission

- `func sendInfraredMessage(protocol, address, command, extra)`

  - Sends an infrared message. _protocol_ is a string that controls
    how the message is encoded.

    Example: `sendInfraredMessage("sirc", 1, 18, 0); // Sony TV Volume Up

  - Protocol details:

    - `jvc`:
      - address: 8 bits
      - command: 8 bits
      - extra: unused
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `kaseikyo`:
      - address: 12 bits
      - command: 8 bits
      - extra: Vendor ID
        - Panasonic = 0x2002
        - Denon = 0x3254
        - Mitsubishi = 0xcb23
        - Sharp = 0x5aaa
        - JVC = 0x0103
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `nec`:
      - address: 8 bits
      - command: 8 bits
      - extra: unused
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `necx`:
      - address: 16 bits
      - command: 8 bits
      - extra: unused
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `rc5`:
      - address: 5 bits
      - command: 7 bits
      - extra: toggle flag
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `rc6`:
      - address: 8 bits
      - command: 8 bits
      - extra: 4 bit header
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `rca`:
      - address: 4 bits
      - command: 8 bits
      - extra: unused
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `samsung`:
      - address: 8 bits
      - command: 8 bits
      - extra: unused
      - This signal repeats, and must be stopped using `stopInfrared()`

    - `sirc`:
      - address: 5, 8 or 13 bits
      - command: 7 bits
      - extra: sirc variation.
        - 0 = auto
        - 12 = sirc12
        - 15 = sirc15
        - 20 = sirc20
      - This signal repeats, and must be stopped using `stopInfrared()`

- `func sendInfraredData(data, dataBits, configuration)`

  - Send _dataBits_ worth of _data_.
  - `sendInfraredData([[23 50]], 12, configuration);` will send
    `0010 0011 0101` encoded using _configuration_.

    - _configuration_ is a 16-bit list as follows:
      - `playbackCount`: Number of times the signal is sent. 0 = infinite.
      - `carrierFrequency`: Frequency in Hz of the carrier signal.
      - `dutyCycle`: Typically 33, representing 33% duty cycle.
      - `flags`: OR-ing of the following flags:
        - Repeat Delay Flags:
          * 0: repeatDelay represents start-to-start period of signal pulses
          * 1: repeatDelay represents time between signal pulses
        - Data Repeat Flags:
          * 0: Full data repeat each signal
          * 2: Repeat without header
        - Endian flags:
          * 0: Transmit most significant bit of data byte first
          * 0x8000: Transmit least significant bit of data byte first
      - `repeatDelayLow`, `repeatDelayHigh`: Repeat delay in microsecond increments
      - `headerTime`
      - `zeroBitTime`
      - `oneBitTime`
      - `trailerTime`

    - `repeatDelayLow` is the low 16 bits of the delay, and `repeatDelayHigh`
      is the upper 16 bits. e.g. A 100ms delay is represented by `34464, 1`

    - The last 4 values are specified as 15-bit values of on-time, off-time
      in half microsecond increments.
        - e.g. `4000, 1000` represents on for 2ms, off for 500µs.

      To represent a pulse with opposite order (off time then on time), specify
      the top bit in the first value:
        - e.g. `4000 | 0x8000, 1000` represents off for 2ms, then on for 500µs.

    - An example configuration that works with Mitsubishi air conditioners is:
    ```
      const MITSUBISHI_CONFIGURATION = [<
        2,            // Playback count
        38000, 33,    // Freq & Duty Cycle
        1, 34000, 0,  // 17ms delay between pulses
        6632, 3869,   // Header time
        737, 996,     // Zero bit time
        737, 2709,    // One bit time
        737, 0        // Trailer time
      >];
      ```

- `func sendInfraredSignal(timing, timeCount, configuration)`
  - Send a signal _timeCount_ times with times specified in _timing_.

  - _timing_ is a 16-bit list, with each value representing the time in
    half microsecond increments. The values must be between 4 and 32767.

  - `sendInfraredData([<1000, 500, 500>], 3, CONFIGURATION);` will send
    on for 500µs, off for 250µs, on for 250µs, then off.

    - CONFIGURATION is a 16-bit list as follows:
      - `playbackCount`: Number of times the signal is sent. 0 = infinite.
      - `carrierFrequency`: Frequency in Hz
      - `dutyCycle`: Typically 33, representing 33% duty cycle.
      - `flags`: Same flags as described for sendInfraredData.
                 Endian flags are not used.
      - `repeatDelayLow`, `repeatDelayHigh`: Repeat delay in microseconds.

## Debug Functions

These functions print information to the console. "Show All Data" option
needs to be enabled in Web Tools to be able to see this information.

- `func printValue(<name>, <value>)`

  - Prints _value_ to the console.
  - Example:
    ```go
      var myValue = 123;
      printValue("value", myValue);
    ```
  - This will show the following in the console:
    ```
    value: 123 (0x7b)
    ```

- `func printData(<name>, <data>, <dataLength>)`

  - Prints a hex dump of _data_ for _dataLength_ bytes to the console.
  - Example:
    ```go
      var bufferGlobalVar = createBuffer(50);
      ...
      printValue("buffer", bufferGlobalVar, 50);
    ```

## Miscellaneous Functions

- `func rand() var`

  - Returns a 32-bit pseudo-random value.

- `func console(<commandString>) var`

  - Sends commandString to the console.
  - Use `help` in the Console Tool on the web to find the available commands.
  - The string returned by the console command is overwritten by other console
    commands.
  - The string returned is matched and replaced with the script's string pool
    so that direct equals and not equals comparisons can be performed.

- `func getParameter(<parameterName>) var`

  - Shortcut to `console("get_parameter <parameterName>")`

- `func getLedStatus(<ledStatus>) var`

  - Returns whether the keyboard LED status is on.
  - Constants:

    ```
    const LED_STATUS_NUM_LOCK = 0;
    const LED_STATUS_CAPS_LOCK = 1;
    const LED_STATUS_SCROLL_LOCK = 2;
    const LED_STATUS_COMPOSE = 3;
    const LED_STATUS_KANA = 4;
    ```

- `func setScript(<scriptId>, <callbackFunc>)`

  - Sets callback script to the specified function:
  - Constants:

    ```
    const SCRIPT_ID_DISPLAY_OVERLAY = 0;
    const SCRIPT_ID_BATTERY_UPDATE = 1;
    const SCRIPT_ID_CONNECTION_UPDATE = 2;
    const SCRIPT_ID_PAIR_CONNECTION_UPDATE = 3;
    const SCRIPT_ID_KEYBOARD_LED_STATUS_UPDATE = 4;
    const SCRIPT_ID_BLE_ADVERTISING_UPDATE = 5;
    const SCRIPT_ID_BLE_SCANNING_UPDATE = 6;
    const SCRIPT_ID_U2F_STATUS_UPDATE = 7;
    const SCRIPT_ID_U2F_WINK = 8;
    const SCRIPT_ID_STENO_MODE_UPDATE = 9;
    const SCRIPT_ID_BLE_PAIR_COMPLETE = 10;

    const SCRIPT_ID_USER1 = 11;
    const SCRIPT_ID_USER2 = 12;
    const SCRIPT_ID_USER3 = 13;
    const SCRIPT_ID_USER4 = 14;
    const SCRIPT_ID_USER5 = 15;
    const SCRIPT_ID_USER6 = 16;
    const SCRIPT_ID_USER7 = 17;
    const SCRIPT_ID_USER8 = 18;
    ```

- `func formatString(<format>, <value>) var`

  - Intended only for use with `getAsset` and `drawText`.
  - _format_ must be a string constant and can contain C printf like formatters.
    Check [writer.cc](writer.cc) for a full list.
  - _value_ is a single integer value to format. Pointer values cannot be used
    and will likely crash the script.

- `func getAsset(<assetName>) var`

  - Returns the data for _assetName_ or 0 if not found.

- `func sendEvent(<eventString>)`

  - Sends a script event to the console.
  - This is used by the web tools to identify the active layer.
  - This will only be sent if script events have been enabled in the console.

- `func setEnableButtonStates(enabled)`

  - Enables sending of button states to the console.
  - This will only be sent if button state updates have also been enabled in
    the console.

- `func getWpm(<windowDurationInSeconds>) var`

  - Returns the weighted wpm in the time window.

- `func isInReinit() var`

  - Returns whether the script is reinitializing after a script upload.
  - This can be used to avoid disabling console and flash write access.

- `func createBuffer(<bytesToAllocate>) var`

  _This function is preliminary and may change at any stage._

  - Returns a pointer to a region that can be used as as mutable 8-bit, 16-bit
    or 32-bit buffer.
  - This function cannot be used and will return 0 if called outside of init()
    or a global variable declaration.
    - This restriction is intentional to avoid memory fragmentation and
      situations that cause scripts to fail unexpectedly that are difficult
      to debug.
      - Related reading: [Rule 3](https://en.wikipedia.org/wiki/The_Power_of_10:_Rules_for_Developing_Safety-Critical_Code) from NASA Coding Standards.
    - Suggested usage:
      ```
      // Allocate 18 bytes for infrared data.
      var mitsubishiAirConditionerInfraredData = createBuffer(18);

      // Allocate 42x 32-bit values for key colors.
      var keyColorData = createBuffer(42 * 4);
      ```
  - Data can be read/written using the following syntax:
    - `[n]` for 8-bit values
    - `[<n>]` for 16-bit values
    - `[{n}]` for 32-bit values

# Visual Editor Script Templates

The settings on each layer within the visual editor are provided by annotations.
Annotations in global scripts will appear in all layers, scripts in a layer
will just show in the settings for that layer.

```
#option(<attributeName>, <displayCategory>, <optionName>, <functionName>)

#dispatch(<"local" | "per_layer">, <attributeName>, <displayCategory>, <defaultFunctionName>)

#flag(<"local" | "per_layer">, <displayCategory>, <default: "true" | "false">)
```
