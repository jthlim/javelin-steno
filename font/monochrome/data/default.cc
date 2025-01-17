#include "../font.h"

constexpr uint8_t GLYPH_DATA[1201] = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0xf8, 0x13,
    0x00, 0x00, 0x03, 0x38, 0x00, 0x00, 0x00, 0x38, 0x00, 0x08, 0x00, 0x02,
    0x20, 0x1e, 0xe0, 0x03, 0x38, 0x02, 0x20, 0x1e, 0xe0, 0x03, 0x38, 0x02,
    0x20, 0x00, 0x07, 0x30, 0x08, 0x48, 0x10, 0x88, 0x10, 0xfe, 0x7f, 0x08,
    0x11, 0x08, 0x12, 0x10, 0x0c, 0x08, 0x30, 0x00, 0x48, 0x10, 0x30, 0x0c,
    0x00, 0x03, 0xc0, 0x00, 0x30, 0x0c, 0x08, 0x12, 0x00, 0x0c, 0x08, 0x00,
    0x06, 0x30, 0x19, 0xc8, 0x10, 0x88, 0x10, 0x48, 0x13, 0x30, 0x0e, 0x00,
    0x0c, 0x00, 0x13, 0x03, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x03, 0xc0,
    0x03, 0x30, 0x0c, 0x0c, 0x30, 0x03, 0x0c, 0x30, 0x30, 0x0c, 0xc0, 0x03,
    0x07, 0x10, 0x02, 0x20, 0x01, 0xc0, 0x00, 0xf0, 0x03, 0xc0, 0x00, 0x20,
    0x01, 0x10, 0x02, 0x05, 0x80, 0x00, 0x80, 0x00, 0xe0, 0x03, 0x80, 0x00,
    0x80, 0x00, 0x02, 0x00, 0x40, 0x00, 0x30, 0x04, 0x80, 0x00, 0x80, 0x00,
    0x80, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x03,
    0x00, 0x1c, 0xc0, 0x03, 0x38, 0x00, 0x07, 0xe0, 0x07, 0x10, 0x08, 0x08,
    0x10, 0x08, 0x10, 0x08, 0x10, 0x10, 0x08, 0xe0, 0x07, 0x04, 0x20, 0x00,
    0x10, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x06, 0x30, 0x18, 0x08, 0x14, 0x08,
    0x12, 0x08, 0x11, 0x88, 0x10, 0x70, 0x10, 0x06, 0x10, 0x08, 0x08, 0x10,
    0x88, 0x10, 0x88, 0x10, 0x50, 0x09, 0x20, 0x06, 0x06, 0x00, 0x03, 0x80,
    0x02, 0x60, 0x02, 0x10, 0x02, 0xf8, 0x1f, 0x00, 0x02, 0x07, 0x78, 0x04,
    0x48, 0x08, 0x48, 0x10, 0x48, 0x10, 0x48, 0x10, 0x88, 0x08, 0x00, 0x07,
    0x07, 0xe0, 0x07, 0x90, 0x08, 0x48, 0x10, 0x48, 0x10, 0x48, 0x10, 0x90,
    0x08, 0x00, 0x07, 0x06, 0x08, 0x00, 0x08, 0x18, 0x08, 0x07, 0xc8, 0x00,
    0x28, 0x00, 0x18, 0x00, 0x07, 0x00, 0x06, 0x70, 0x09, 0x88, 0x10, 0x88,
    0x10, 0x88, 0x10, 0x70, 0x09, 0x00, 0x06, 0x07, 0x00, 0x00, 0xf0, 0x08,
    0x08, 0x11, 0x08, 0x11, 0x08, 0x11, 0x90, 0x08, 0xe0, 0x07, 0x03, 0x00,
    0x00, 0x60, 0x06, 0x00, 0x00, 0x03, 0x00, 0x08, 0x60, 0x06, 0x00, 0x00,
    0x06, 0x80, 0x00, 0x40, 0x01, 0x40, 0x01, 0x20, 0x02, 0x20, 0x02, 0x10,
    0x04, 0x07, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02,
    0x40, 0x02, 0x40, 0x02, 0x06, 0x10, 0x04, 0x20, 0x02, 0x20, 0x02, 0x40,
    0x01, 0x40, 0x01, 0x80, 0x00, 0x06, 0x10, 0x00, 0x08, 0x00, 0x08, 0x17,
    0x88, 0x00, 0x50, 0x00, 0x20, 0x00, 0x0d, 0x80, 0x0f, 0x60, 0x10, 0x10,
    0x20, 0x88, 0x4f, 0x48, 0x90, 0x28, 0x90, 0x28, 0x88, 0x48, 0x8c, 0xc8,
    0x9f, 0x08, 0x90, 0x10, 0x48, 0x20, 0x26, 0xc0, 0x01, 0x07, 0x00, 0x1e,
    0x80, 0x03, 0x60, 0x02, 0x18, 0x02, 0x60, 0x02, 0x80, 0x03, 0x00, 0x1e,
    0x07, 0xf8, 0x1f, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x88,
    0x10, 0x70, 0x0f, 0x08, 0xc0, 0x03, 0x30, 0x0c, 0x08, 0x10, 0x08, 0x10,
    0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x10, 0x08, 0x07, 0xf8, 0x1f, 0x08,
    0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x10, 0x08, 0xe0, 0x07, 0x07,
    0xf8, 0x1f, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10,
    0x08, 0x10, 0x07, 0xf8, 0x1f, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88,
    0x00, 0x88, 0x00, 0x08, 0x00, 0x09, 0xc0, 0x03, 0x30, 0x0c, 0x08, 0x10,
    0x08, 0x10, 0x08, 0x10, 0x08, 0x11, 0x08, 0x11, 0x10, 0x09, 0x00, 0x07,
    0x07, 0xf8, 0x1f, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
    0x00, 0xf8, 0x1f, 0x03, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x05, 0x00,
    0x0c, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0xf8, 0x0f, 0x07, 0xf8, 0x1f,
    0x80, 0x00, 0xc0, 0x00, 0x20, 0x01, 0x10, 0x06, 0x08, 0x08, 0x00, 0x10,
    0x06, 0xf8, 0x1f, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00,
    0x10, 0x09, 0xf8, 0x1f, 0x30, 0x00, 0xc0, 0x01, 0x00, 0x06, 0x00, 0x18,
    0x00, 0x06, 0xc0, 0x01, 0x30, 0x00, 0xf8, 0x1f, 0x08, 0xf8, 0x1f, 0x10,
    0x00, 0x60, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x06, 0x00, 0x08, 0xf8,
    0x1f, 0x09, 0xc0, 0x03, 0x30, 0x0c, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
    0x08, 0x10, 0x08, 0x10, 0x30, 0x0c, 0xc0, 0x03, 0x07, 0xf8, 0x1f, 0x08,
    0x01, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0xf0, 0x00, 0x09,
    0xc0, 0x03, 0x30, 0x0c, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x14,
    0x08, 0x18, 0x30, 0x0c, 0xc0, 0x13, 0x07, 0xf8, 0x1f, 0x88, 0x00, 0x88,
    0x00, 0x88, 0x00, 0x88, 0x01, 0x88, 0x02, 0x70, 0x1c, 0x08, 0x20, 0x04,
    0x50, 0x08, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x88, 0x10, 0x10, 0x09,
    0x00, 0x06, 0x07, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0xf8, 0x1f, 0x08,
    0x00, 0x08, 0x00, 0x08, 0x00, 0x07, 0xf8, 0x07, 0x00, 0x08, 0x00, 0x10,
    0x00, 0x10, 0x00, 0x10, 0x00, 0x08, 0xf8, 0x07, 0x08, 0x18, 0x00, 0xe0,
    0x00, 0x00, 0x07, 0x00, 0x18, 0x00, 0x18, 0x00, 0x07, 0xe0, 0x00, 0x18,
    0x00, 0x0d, 0x18, 0x00, 0xe0, 0x00, 0x00, 0x07, 0x00, 0x18, 0x00, 0x07,
    0xe0, 0x00, 0x18, 0x00, 0xe0, 0x00, 0x00, 0x07, 0x00, 0x18, 0x00, 0x07,
    0xe0, 0x00, 0x18, 0x00, 0x07, 0x08, 0x10, 0x30, 0x0c, 0x40, 0x02, 0x80,
    0x01, 0x40, 0x02, 0x30, 0x0c, 0x08, 0x10, 0x07, 0x08, 0x00, 0x30, 0x00,
    0xc0, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x30, 0x00, 0x08, 0x00, 0x08, 0x08,
    0x18, 0x08, 0x14, 0x08, 0x12, 0x08, 0x11, 0x88, 0x10, 0x48, 0x10, 0x28,
    0x10, 0x18, 0x10, 0x03, 0xfc, 0x3f, 0x04, 0x20, 0x04, 0x20, 0x03, 0x38,
    0x00, 0xc0, 0x03, 0x00, 0x1c, 0x03, 0x04, 0x20, 0x04, 0x20, 0xfc, 0x3f,
    0x05, 0x20, 0x00, 0x10, 0x00, 0x08, 0x00, 0x10, 0x00, 0x20, 0x00, 0x06,
    0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10,
    0x03, 0x00, 0x00, 0x18, 0x00, 0x20, 0x00, 0x06, 0x40, 0x0e, 0x20, 0x12,
    0x20, 0x12, 0x20, 0x11, 0x20, 0x09, 0xc0, 0x1f, 0x06, 0xf8, 0x1f, 0x40,
    0x08, 0x20, 0x10, 0x20, 0x10, 0x40, 0x08, 0x80, 0x07, 0x06, 0x80, 0x07,
    0x40, 0x08, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x40, 0x08, 0x06, 0x80,
    0x07, 0x40, 0x08, 0x20, 0x10, 0x20, 0x10, 0x40, 0x08, 0xf8, 0x1f, 0x07,
    0x80, 0x07, 0x40, 0x09, 0x20, 0x11, 0x20, 0x11, 0x20, 0x11, 0x40, 0x11,
    0x80, 0x09, 0x04, 0x20, 0x00, 0xf0, 0x1f, 0x28, 0x00, 0x28, 0x00, 0x06,
    0x80, 0x47, 0x40, 0x88, 0x20, 0x90, 0x20, 0x90, 0x40, 0x48, 0xe0, 0x3f,
    0x05, 0xf8, 0x1f, 0x40, 0x00, 0x20, 0x00, 0x20, 0x00, 0xc0, 0x1f, 0x03,
    0x00, 0x00, 0xe8, 0x1f, 0x00, 0x00, 0x04, 0x00, 0x80, 0x00, 0x40, 0xe8,
    0x3f, 0x00, 0x00, 0x05, 0xf8, 0x1f, 0x00, 0x01, 0x80, 0x02, 0x40, 0x04,
    0x20, 0x18, 0x03, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x09, 0xe0, 0x1f,
    0x40, 0x00, 0x20, 0x00, 0x20, 0x00, 0xc0, 0x1f, 0x40, 0x00, 0x20, 0x00,
    0x20, 0x00, 0xc0, 0x1f, 0x05, 0xe0, 0x1f, 0x40, 0x00, 0x20, 0x00, 0x20,
    0x00, 0xc0, 0x1f, 0x06, 0x80, 0x07, 0x40, 0x08, 0x20, 0x10, 0x20, 0x10,
    0x40, 0x08, 0x80, 0x07, 0x06, 0xe0, 0xff, 0x40, 0x08, 0x20, 0x10, 0x20,
    0x10, 0x40, 0x08, 0x80, 0x07, 0x06, 0x80, 0x0f, 0x40, 0x10, 0x20, 0x10,
    0x20, 0x10, 0x40, 0x08, 0xe0, 0xff, 0x04, 0xe0, 0x1f, 0x40, 0x00, 0x20,
    0x00, 0x20, 0x00, 0x05, 0xc0, 0x08, 0x20, 0x11, 0x20, 0x11, 0x20, 0x12,
    0x40, 0x0c, 0x03, 0x20, 0x00, 0xf0, 0x0f, 0x20, 0x10, 0x06, 0xe0, 0x07,
    0x00, 0x08, 0x00, 0x10, 0x00, 0x10, 0x00, 0x08, 0xe0, 0x1f, 0x05, 0xe0,
    0x01, 0x00, 0x06, 0x00, 0x18, 0x00, 0x06, 0xe0, 0x01, 0x09, 0xe0, 0x00,
    0x00, 0x07, 0x00, 0x18, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0x00, 0x18,
    0x00, 0x07, 0xe0, 0x00, 0x05, 0x60, 0x18, 0x80, 0x04, 0x00, 0x03, 0x80,
    0x04, 0x60, 0x18, 0x05, 0xe0, 0x80, 0x00, 0x47, 0x00, 0x38, 0x00, 0x07,
    0xe0, 0x00, 0x06, 0x20, 0x18, 0x20, 0x14, 0x20, 0x12, 0x20, 0x11, 0xa0,
    0x10, 0x60, 0x10, 0x03, 0x00, 0x01, 0xf8, 0x3e, 0x04, 0x40, 0x03, 0x00,
    0x00, 0xfc, 0x7f, 0x00, 0x00, 0x03, 0x04, 0x40, 0xf8, 0x3e, 0x00, 0x01,
    0x06, 0x00, 0x01, 0x80, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x01, 0x80,
    0x00,
};

constexpr uint16_t DATA_OFFSETS[95] = {
    1,    8,    15,   22,   39,   54,   71,   88,   95,   102,  109,  124,
    135,  140,  149,  156,  163,  178,  187,  200,  213,  226,  241,  256,
    269,  284,  299,  306,  313,  326,  341,  354,  367,  394,  409,  424,
    441,  456,  471,  486,  505,  520,  527,  538,  553,  566,  585,  602,
    621,  636,  655,  670,  687,  702,  717,  734,  761,  776,  791,  808,
    815,  822,  829,  840,  853,  860,  873,  886,  899,  912,  927,  936,
    949,  960,  967,  976,  987,  994,  1013, 1024, 1037, 1050, 1063, 1072,
    1083, 1090, 1103, 1114, 1133, 1144, 1155, 1168, 1175, 1182, 1189,
};

constexpr Font Font::DEFAULT = {
    32,  // startUnicode
    127, // endUnicode
    0,   // characterWidth
    16,  // height
    12,  // baseline
    1,   // spacing
    DATA_OFFSETS,
    GLYPH_DATA,
};
