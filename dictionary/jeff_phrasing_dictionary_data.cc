//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary_data.h"
#include "../crc.h"
#include "dictionary.h"
#include <string.h>

//---------------------------------------------------------------------------

// spellchecker: disable

// clang-format off
constexpr JeffPhrasingSimpleStarter SIMPLE_STARTERS[] = {
    {StenoStroke(0x146 /*STHA*/), {VerbForm::UNSPECIFIED, "that"}},
    {StenoStroke(0x116 /*STPA*/), {VerbForm::UNSPECIFIED, "if"}},
    {StenoStroke(0x62 /*SWH*/), {VerbForm::UNSPECIFIED, "when"}},
    {StenoStroke(0x162 /*SWHA*/), {VerbForm::UNSPECIFIED, "what"}},
    {StenoStroke(0xe2 /*SWHR*/), {VerbForm::UNSPECIFIED, "where"}},
    {StenoStroke(0x262 /*SWHO*/), {VerbForm::UNSPECIFIED, "who"}},
    {StenoStroke(0x72 /*SPWH*/), {VerbForm::UNSPECIFIED, "but"}},
    {StenoStroke(0x96 /*STPR*/), {VerbForm::UNSPECIFIED, "for"}},

    // # Remove the entry below if you don't want "and" phrases.
    {StenoStroke(0x1a /*SKP*/), {VerbForm::UNSPECIFIED, "and"}},
};

constexpr JeffPhrasingFullStarter FULL_STARTERS[] = {
  {StenoStroke(0xa2 /*SWR*/), {"I", VerbForm::FIRST_PERSON_SINGULAR, true}},
  {StenoStroke(0xb8 /*KPWR*/), {"you", VerbForm::SECOND_PERSON, true}},
  {StenoStroke(0xe8 /*KWHR*/), {"he", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xea /*SKWHR*/), {"she", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0x78 /*KPWH*/), {"it", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xa4 /*TWR*/), {"we", VerbForm::FIRST_PERSON_PLURAL, true}},
  {StenoStroke(0x64 /*TWH*/), {"they", VerbForm::THIRD_PERSON_PLURAL, true}},
  {StenoStroke(0x4e /*STKH*/), {"this", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0x66 /*STWH*/), {"that", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xc6 /*STHR*/), {"there", VerbForm::THIRD_PERSON_SINGULAR, false}},
  {StenoStroke(0xd6 /*STPHR*/), {"there", VerbForm::THIRD_PERSON_PLURAL, false}},
  {StenoStroke(0xfe /*STKPWHR*/), {"", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xa6 /*STWR*/), {"", VerbForm::THIRD_PERSON_PLURAL, true}},
};
// clang-format on

constexpr JeffPhrasingPronoun SIMPLE_PRONOUNS[] = {
    {}, // Unused
    {"it", VerbForm::THIRD_PERSON_SINGULAR, true},
    {"he", VerbForm::THIRD_PERSON_SINGULAR, true},
    {"she", VerbForm::THIRD_PERSON_SINGULAR, true},
    {"you", VerbForm::SECOND_PERSON, true},
    {"they", VerbForm::THIRD_PERSON_PLURAL, true},
    {"I", VerbForm::FIRST_PERSON_SINGULAR, true},
    {"we", VerbForm::FIRST_PERSON_PLURAL, true},
};

constexpr JeffPhrasingMapData<2> SIMPLE_STRUCTURE_HAVE_PRESENT = {
    .entries =
        {
            {0, "\\1 \\0 have\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\1 \\0 has\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> SIMPLE_STRUCTURE_HAVE = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &SIMPLE_STRUCTURE_HAVE_PRESENT},
            {(uint32_t)Tense::PAST, "\\1 \\0 had\\2\\3"},
        },
};

constexpr JeffPhrasingStructure SIMPLE_STRUCTURES[] = {
    {"\\1 \\0\\2\\3", true, VerbForm::UNSPECIFIED},
    {&SIMPLE_STRUCTURE_HAVE, true, VerbForm::PAST_PARTICIPLE},
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_PRESENT = {
    .entries =
        {
            {(uint32_t)VerbForm::UNSPECIFIED, " do"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " does"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &MIDDLES_DO_PRESENT},
            {(uint32_t)Tense::PAST, " did"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_SHALL = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " shall"},
            {(uint32_t)Tense::PAST, " should"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_CAN = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " can"},
            {(uint32_t)Tense::PAST, " could"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_WILL = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " will"},
            {(uint32_t)Tense::PAST, " would"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_NOT_PRESENT = {
    .entries =
        {
            {(uint32_t)VerbForm::UNSPECIFIED, " don't"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " doesn't"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &MIDDLES_DO_NOT_PRESENT},
            {(uint32_t)Tense::PAST, " didn't"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_SHALL_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " shall not"},
            {(uint32_t)Tense::PAST, " shouldn't"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_CAN_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " can't"},
            {(uint32_t)Tense::PAST, " couldn't"},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_WILL_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, " won't"},
            {(uint32_t)Tense::PAST, " wouldn't"},
        },
};

// Indexes are into `AO*`
constexpr JeffPhrasingMiddle MIDDLES[8] = {
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_DO)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_CAN)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_SHALL)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_WILL)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_DO_NOT)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_CAN_NOT)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_SHALL_NOT)},
    {VerbForm::ROOT, JeffPhrasingVariant(&MIDDLES_WILL_NOT)},
};

constexpr JeffPhrasingMapData<4> FULL_STRUCTURE_BE_PRESENT = {
    .entries =
        {
            {0, "\\0\\1 are\\2\\3"},
            {(uint32_t)VerbForm::ROOT, "\\0\\1 be\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0\\1 am\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0\\1 is\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<4> FULL_STRUCTURE_BE_PAST = {
    .entries =
        {
            {0, "\\0\\1 were\\2\\3"},
            {(uint32_t)VerbForm::ROOT, "\\0\\1 be\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0\\1 was\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0\\1 was\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_BE = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &FULL_STRUCTURE_BE_PRESENT},
            {(uint32_t)Tense::PAST, &FULL_STRUCTURE_BE_PAST},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_PRESENT = {
    .entries =
        {
            {0, "\\0\\1 have\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0\\1 has\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_PAST = {
    .entries =
        {
            {0, "\\0\\1 had\\2\\3"},
            {(uint32_t)VerbForm::ROOT, "\\0\\1 have\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &FULL_STRUCTURE_HAVE_PRESENT},
            {(uint32_t)Tense::PAST, &FULL_STRUCTURE_HAVE_PAST},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN_PRESENT = {
    .entries =
        {
            {0, "\\0\\1 have been\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR,
             "\\0\\1 has been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN_PAST = {
    .entries =
        {
            {0, "\\0\\1 had been\\2\\3"},
            {(uint32_t)VerbForm::ROOT, "\\0\\1 have been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &FULL_STRUCTURE_HAVE_BEEN_PRESENT},
            {(uint32_t)Tense::PAST, &FULL_STRUCTURE_HAVE_BEEN_PAST},
        },
};

// Indexed on `*EUF`
// \\0 is starter.
// \\1 is middle.
// \\2 is verb
// \\3 is suffix word
constexpr JeffPhrasingStructure STRUCTURES[16] = {
    {"\\0\\1\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0\\1\\2\\3", true, VerbForm::UNSPECIFIED},
    {&FULL_STRUCTURE_BE, true, VerbForm::PRESENT_PARTICIPLE},
    {&FULL_STRUCTURE_BE, true, VerbForm::PRESENT_PARTICIPLE},
    {"\\1 \\0\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\1 \\0\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0\\1 still\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0 still\\1\\2\\3", true, VerbForm::UNSPECIFIED},
    {&FULL_STRUCTURE_HAVE, true, VerbForm::PAST_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE, true, VerbForm::PAST_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE_BEEN, true, VerbForm::PRESENT_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE_BEEN, true, VerbForm::PRESENT_PARTICIPLE},
    {"\\0\\1 just\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0 just\\1\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0\\1 never\\2\\3", true, VerbForm::UNSPECIFIED},
    {"\\0\\1 even\\2\\3", true, VerbForm::UNSPECIFIED},
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_PRESENT = {
    .entries =
        {
            {0, "\\0 are\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0 am\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 is\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_PAST = {
    .entries =
        {
            {0, "\\0 were\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0 was\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 was\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_BE = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &STRUCTURE_EXCEPTION_BE_PRESENT},
            {(uint32_t)Tense::PAST, &STRUCTURE_EXCEPTION_BE_PAST},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_NOT_PRESENT = {
    .entries =
        {
            {0, "\\0 aren't\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0 am not\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 isn't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_NOT_PAST = {
    .entries =
        {
            {0, "\\0 weren't\\2\\3"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, "\\0 wasn't\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 wasn't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_BE_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &STRUCTURE_EXCEPTION_BE_NOT_PRESENT},
            {(uint32_t)Tense::PAST, &STRUCTURE_EXCEPTION_BE_NOT_PAST},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_PRESENT = {
    .entries =
        {
            {0, "\\0 have\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 has\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &STRUCTURE_EXCEPTION_HAVE_PRESENT},
            {(uint32_t)Tense::PAST, "\\0 had\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_PRESENT = {
    .entries =
        {
            {0, "\\0 haven't\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 hasn't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &STRUCTURE_EXCEPTION_HAVE_NOT_PRESENT},
            {(uint32_t)Tense::PAST, "\\0 hadn't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_BEEN_PRESENT = {
    .entries =
        {
            {0, "\\0 have been\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, "\\0 has been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_BEEN = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT, &STRUCTURE_EXCEPTION_HAVE_BEEN_PRESENT},
            {(uint32_t)Tense::PAST, "\\0 had been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_BEEN_PRESENT = {
    .entries =
        {
            {0, "\\0 haven't been\\2\\3"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR,
             "\\0 hasn't been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_BEEN = {
    .entries =
        {
            {(uint32_t)Tense::PRESENT,
             &STRUCTURE_EXCEPTION_HAVE_NOT_BEEN_PRESENT},
            {(uint32_t)Tense::PAST, "\\0 hadn't been\\2\\3"},
        },
};

// clang-format off
constexpr JeffPhrasingStructureException STRUCTURE_EXCEPTIONS[] = {
    {StenoStroke(), {"\\0\\2\\3", false, VerbForm::UNSPECIFIED}},

    {StenoStroke(0x800 /*E*/), {&STRUCTURE_EXCEPTION_BE, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0xC00 /**E*/), {&STRUCTURE_EXCEPTION_BE_NOT, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0x2000 /*F*/), {&STRUCTURE_EXCEPTION_HAVE, false, VerbForm::PAST_PARTICIPLE}},
    {StenoStroke(0x2400 /**F*/), {&STRUCTURE_EXCEPTION_HAVE_NOT, false, VerbForm::PAST_PARTICIPLE}},
    {StenoStroke(0x2800 /*EF*/), {&STRUCTURE_EXCEPTION_HAVE_BEEN, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0x2c00 /**EF*/), {&STRUCTURE_EXCEPTION_HAVE_NOT_BEEN, false, VerbForm::PRESENT_PARTICIPLE}},

    {StenoStroke(0x1800 /*EU*/), {"\\0 still\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoStroke(0x3800 /*EUF*/), {"\\0 never\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoStroke(0x3000 /*UF*/), {"\\0 just\\2\\3", false, VerbForm::UNSPECIFIED}},
};

constexpr JeffPhrasingStructureException UNIQUE_STARTERS[] = {
    {StenoStroke(0x10a6 /*STWRU*/), {"to\\2\\3", false, VerbForm::ROOT}},
    {StenoStroke(0x14a6 /*STWR*U*/), {"not to\\2\\3", false, VerbForm::ROOT}},
    {StenoStroke(0x10fe /*STKPWHRU*/), {"to\\2\\3", false, VerbForm::ROOT}},
    {StenoStroke(0x14fe /*STKPWHR*U*/), {"not to\\2\\3", false, VerbForm::ROOT}},
};

// clang-format on

constexpr JeffPhrasingMapData<4> ENDER_RB = {
    .entries =
        {
            {0, " ask"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " asks"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " asking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " asked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBD = {
    .entries =
        {
            {0, " asked"},
            {(uint32_t)VerbForm::ROOT, " ask"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " asking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " asked"},
        },
};

constexpr JeffPhrasingMapData<6> ENDER_B = {
    .entries =
        {
            {0, " are"},
            {(uint32_t)VerbForm::ROOT, " be"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, " am"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " is"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " being"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " been"},
        },
};

constexpr JeffPhrasingMapData<6> ENDER_BD = {
    .entries =
        {
            {0, " were"},
            {(uint32_t)VerbForm::ROOT, " be"},
            {(uint32_t)VerbForm::FIRST_PERSON_SINGULAR, " was"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " was"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " being"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " been"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBG = {
    .entries =
        {
            {0, " become"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " becomes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " becoming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " become"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBGD = {
    .entries =
        {
            {0, " became"},
            {(uint32_t)VerbForm::ROOT, " become"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " becoming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " become"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BL = {
    .entries =
        {
            {0, " believe"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " believes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " believing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " believed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLD = {
    .entries =
        {
            {0, " believed"},
            {(uint32_t)VerbForm::ROOT, " believe"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " believing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " believed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBLG = {
    .entries =
        {
            {0, " call"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " calls"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " calling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " called"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBLGD = {
    .entries =
        {
            {0, " called"},
            {(uint32_t)VerbForm::ROOT, " call"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " calling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " called"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RZ = {
    .entries =
        {
            {0, " care"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " cares"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " caring"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " cared"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RDZ = {
    .entries =
        {
            {0, " cared"},
            {(uint32_t)VerbForm::ROOT, " care"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " caring"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " cared"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGZ = {
    .entries =
        {
            {0, " change"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " changes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " changing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " changed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGDZ = {
    .entries =
        {
            {0, " changed"},
            {(uint32_t)VerbForm::ROOT, " change"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " changing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " changed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BG = {
    .entries =
        {
            {0, " come"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " comes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " coming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " come"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BGD = {
    .entries =
        {
            {0, " came"},
            {(uint32_t)VerbForm::ROOT, " come"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " coming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " come"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGZ = {
    .entries =
        {
            {0, " consider"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " considers"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " considering"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " considered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGDZ = {
    .entries =
        {
            {0, " considered"},
            {(uint32_t)VerbForm::ROOT, " consider"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " considering"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " considered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RP = {
    .entries =
        {
            {0, " do"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " does"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " doing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " done"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPD = {
    .entries =
        {
            {0, " did"},
            {(uint32_t)VerbForm::ROOT, " do"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " doing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " done"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGS = {
    .entries =
        {
            {0, " expect"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " expects"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " expecting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " expected"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGSZ = {
    .entries =
        {
            {0, " expected"},
            {(uint32_t)VerbForm::ROOT, " expect"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " expecting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " expected"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LT = {
    .entries =
        {
            {0, " feel"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " feels"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " feeling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " felt"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LTD = {
    .entries =
        {
            {0, " felt"},
            {(uint32_t)VerbForm::ROOT, " feel"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " feeling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " felt"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLG = {
    .entries =
        {
            {0, " find"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " finds"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " finding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " found"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLGD = {
    .entries =
        {
            {0, " found"},
            {(uint32_t)VerbForm::ROOT, " find"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " finding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " found"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RG = {
    .entries =
        {
            {0, " forget"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " forgets"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " forgetting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " forgotten"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RGD = {
    .entries =
        {
            {0, " forgot"},
            {(uint32_t)VerbForm::ROOT, " forget"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " forgetting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " forgotten"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GS = {
    .entries =
        {
            {0, " get"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " gets"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " getting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " gotten"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GSZ = {
    .entries =
        {
            {0, " got"},
            {(uint32_t)VerbForm::ROOT, " get"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " getting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " gotten"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GZ = {
    .entries =
        {
            {0, " give"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " gives"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " giving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " given"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GDZ = {
    .entries =
        {
            {0, " gave"},
            {(uint32_t)VerbForm::ROOT, " give"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " giving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " given"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_G = {
    .entries =
        {
            {0, " go"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " goes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " going"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " gone"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GD = {
    .entries =
        {
            {0, " went"},
            {(uint32_t)VerbForm::ROOT, " go"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " going"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " gone"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_T = {
    .entries =
        {
            {0, " have"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " has"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " having"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " had"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_TD = {
    .entries =
        {
            {0, " had"},
            {(uint32_t)VerbForm::ROOT, " have"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " having"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " had"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PZ = {
    .entries =
        {
            {0, " happen"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " happens"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " happening"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " happened"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PDZ = {
    .entries =
        {
            {0, " happened"},
            {(uint32_t)VerbForm::ROOT, " happen"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " happening"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " happened"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PG = {
    .entries =
        {
            {0, " hear"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " hears"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " hearing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " heard"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGD = {
    .entries =
        {
            {0, " heard"},
            {(uint32_t)VerbForm::ROOT, " hear"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " hearing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " heard"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPS = {
    .entries =
        {
            {0, " hope"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " hopes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " hoping"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " hoped"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPSZ = {
    .entries =
        {
            {0, " hoped"},
            {(uint32_t)VerbForm::ROOT, " hope"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " hoping"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " hoped"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLG = {
    .entries =
        {
            {0, " imagine"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " imagines"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " imagining"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " imagined"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLGD = {
    .entries =
        {
            {0, " imagined"},
            {(uint32_t)VerbForm::ROOT, " imagine"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " imagining"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " imagined"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGS = {
    .entries =
        {
            {0, " keep"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " keeps"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " keeping"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " kept"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGSZ = {
    .entries =
        {
            {0, " kept"},
            {(uint32_t)VerbForm::ROOT, " keep"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " keeping"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " kept"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PB = {
    .entries =
        {
            {0, " know"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " knows"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " knowing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " known"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBD = {
    .entries =
        {
            {0, " knew"},
            {(uint32_t)VerbForm::ROOT, " know"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " knowing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " known"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBS = {
    .entries =
        {
            {0, " learn"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " learns"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " learning"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " learned"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBSZ = {
    .entries =
        {
            {0, " learned"},
            {(uint32_t)VerbForm::ROOT, " learn"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " learning"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " learned"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGZ = {
    .entries =
        {
            {0, " leave"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " leaves"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " leaving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " left"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGDZ = {
    .entries =
        {
            {0, " left"},
            {(uint32_t)VerbForm::ROOT, " leave"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " leaving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " left"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LS = {
    .entries =
        {
            {0, " let"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " lets"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " letting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " let"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LSZ = {
    .entries =
        {
            {0, " let"},
            {(uint32_t)VerbForm::ROOT, " let"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " letting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " let"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLG = {
    .entries =
        {
            {0, " like"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " likes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " liking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " liked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLGD = {
    .entries =
        {
            {0, " liked"},
            {(uint32_t)VerbForm::ROOT, " like"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " liking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " liked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LZ = {
    .entries =
        {
            {0, " live"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " lives"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " living"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " lived"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LDZ = {
    .entries =
        {
            {0, " lived"},
            {(uint32_t)VerbForm::ROOT, " live"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " living"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " lived"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_L = {
    .entries =
        {
            {0, " look"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " looks"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " looking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " looked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LD = {
    .entries =
        {
            {0, " looked"},
            {(uint32_t)VerbForm::ROOT, " look"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " looking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " looked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LG = {
    .entries =
        {
            {0, " love"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " loves"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " loving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " loved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGD = {
    .entries =
        {
            {0, " loved"},
            {(uint32_t)VerbForm::ROOT, " love"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " loving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " loved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBL = {
    .entries =
        {
            {0, " make"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " makes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " making"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " made"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBLD = {
    .entries =
        {
            {0, " made"},
            {(uint32_t)VerbForm::ROOT, " make"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " making"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " made"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBL = {
    .entries =
        {
            {0, " mean"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " means"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " meaning"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " meant"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLD = {
    .entries =
        {
            {0, " meant"},
            {(uint32_t)VerbForm::ROOT, " mean"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " meaning"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " meant"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLS = {
    .entries =
        {
            {0, " mind"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " minds"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " minding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " minded"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLSZ = {
    .entries =
        {
            {0, " minded"},
            {(uint32_t)VerbForm::ROOT, " mind"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " minding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " minded"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLZ = {
    .entries =
        {
            {0, " move"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " moves"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " moving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " moved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLDZ = {
    .entries =
        {
            {0, " moved"},
            {(uint32_t)VerbForm::ROOT, " move"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " moving"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " moved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPG = {
    .entries =
        {
            {0, " need"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " needs"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " needing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " needed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPGD = {
    .entries =
        {
            {0, " needed"},
            {(uint32_t)VerbForm::ROOT, " need"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " needing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " needed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PS = {
    .entries =
        {
            {0, " put"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " puts"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " putting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " put"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PSZ = {
    .entries =
        {
            {0, " put"},
            {(uint32_t)VerbForm::ROOT, " put"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " putting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " put"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RS = {
    .entries =
        {
            {0, " read"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " reads"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " reading"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " read"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RSZ = {
    .entries =
        {
            {0, " read"},
            {(uint32_t)VerbForm::ROOT, " read"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " reading"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " read"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RL = {
    .entries =
        {
            {0, " recall"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " recalls"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " recalling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " recalled"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLD = {
    .entries =
        {
            {0, " recalled"},
            {(uint32_t)VerbForm::ROOT, " recall"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " recalling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " recalled"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLS = {
    .entries =
        {
            {0, " realize"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " realizes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " realizing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " realized"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLSZ = {
    .entries =
        {
            {0, " realized"},
            {(uint32_t)VerbForm::ROOT, " realize"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " realizing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " realized"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPL = {
    .entries =
        {
            {0, " remember"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " remembers"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " remembering"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " remembered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLD = {
    .entries =
        {
            {0, " remembered"},
            {(uint32_t)VerbForm::ROOT, " remember"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " remembering"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " remembered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLS = {
    .entries =
        {
            {0, " remain"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " remains"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " remaining"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " remained"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLSZ = {
    .entries =
        {
            {0, " remained"},
            {(uint32_t)VerbForm::ROOT, " remain"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " remaining"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " remained"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_R = {
    .entries =
        {
            {0, " run"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " runs"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " running"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " run"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RD = {
    .entries =
        {
            {0, " ran"},
            {(uint32_t)VerbForm::ROOT, " run"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " running"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " run"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BS = {
    .entries =
        {
            {0, " say"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " says"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " saying"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " said"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BSZ = {
    .entries =
        {
            {0, " said"},
            {(uint32_t)VerbForm::ROOT, " say"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " saying"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " said"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_S = {
    .entries =
        {
            {0, " see"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " sees"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " seeing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " seen"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_SZ = {
    .entries =
        {
            {0, " saw"},
            {(uint32_t)VerbForm::ROOT, " see"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " seeing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " seen"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLS = {
    .entries =
        {
            {0, " set"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " sets"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " setting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " set"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLSZ = {
    .entries =
        {
            {0, " set"},
            {(uint32_t)VerbForm::ROOT, " set"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " setting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " set"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLS = {
    .entries =
        {
            {0, " seem"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " seems"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " seeming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " seemed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLSZ = {
    .entries =
        {
            {0, " seemed"},
            {(uint32_t)VerbForm::ROOT, " seem"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " seeming"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " seemed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBZ = {
    .entries =
        {
            {0, " show"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " shows"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " showing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " shown"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBDZ = {
    .entries =
        {
            {0, " showed"},
            {(uint32_t)VerbForm::ROOT, " want"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " showing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " shown"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBT = {
    .entries =
        {
            {0, " take"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " takes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " taking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " taken"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBTD = {
    .entries =
        {
            {0, " took"},
            {(uint32_t)VerbForm::ROOT, " take"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " taking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " taken"},
        },
};

// constexpr JeffPhrasingMapData<4> ENDER_BLGT = {
//     .entries =
//         {
//             {0, " talk"},
//             {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " talks"},
//             {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " talking"},
//             {(uint32_t)VerbForm::PAST_PARTICIPLE, " talked"},
//         },
// };

// constexpr JeffPhrasingMapData<4> ENDER_BLGTD = {
//     .entries =
//         {
//             {0, " talked"},
//             {(uint32_t)VerbForm::ROOT, " talk"},
//             {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " talking"},
//             {(uint32_t)VerbForm::PAST_PARTICIPLE, " talked"},
//         },
// };

constexpr JeffPhrasingMapData<4> ENDER_RLT = {
    .entries =
        {
            {0, " tell"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " tells"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " telling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " told"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLTD = {
    .entries =
        {
            {0, " told"},
            {(uint32_t)VerbForm::ROOT, " tell"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " telling"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " told"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBG = {
    .entries =
        {
            {0, " think"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " thinks"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " thinking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " thought"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGD = {
    .entries =
        {
            {0, " thought"},
            {(uint32_t)VerbForm::ROOT, " think"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " thinking"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " thought"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RT = {
    .entries =
        {
            {0, " try"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " tries"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " trying"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " tried"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RTD = {
    .entries =
        {
            {0, " tried"},
            {(uint32_t)VerbForm::ROOT, " try"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " trying"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " tried"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPB = {
    .entries =
        {
            {0, " understand"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " understands"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " understanding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " understood"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBD = {
    .entries =
        {
            {0, " understood"},
            {(uint32_t)VerbForm::ROOT, " understand"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " understanding"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " understood"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_Z = {
    .entries =
        {
            {0, " use"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " uses"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " using"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " used"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_DZ = {
    .entries =
        {
            {0, " used"},
            {(uint32_t)VerbForm::ROOT, " use"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " using"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " used"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_P = {
    .entries =
        {
            {0, " want"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " wants"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " wanting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " wanted"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PD = {
    .entries =
        {
            {0, " wanted"},
            {(uint32_t)VerbForm::ROOT, " want"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " wanting"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " wanted"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBS = {
    .entries =
        {
            {0, " wish"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " wishes"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " wishing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " wished"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBSZ = {
    .entries =
        {
            {0, " wished"},
            {(uint32_t)VerbForm::ROOT, " wish"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " wishing"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " wished"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBG = {
    .entries =
        {
            {0, " work"},
            {(uint32_t)VerbForm::THIRD_PERSON_SINGULAR, " works"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " working"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " worked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGD = {
    .entries =
        {
            {0, " worked"},
            {(uint32_t)VerbForm::ROOT, " work"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " working"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " worked"},
        },
};

// clang-format off
constexpr JeffPhrasingEnder ENDERS[] = {
    {StenoStroke(), Tense::PRESENT, true, "", ""},
    {StenoStroke(0x200000 /*D*/), Tense::PAST, true, "", ""},

    // RB - To ask
    {StenoStroke(0x14000 /*RB*/), Tense::PRESENT, false, &ENDER_RB, ""},
    {StenoStroke(0x214000 /*-RBD*/), Tense::PAST, false, &ENDER_RBD, ""},

    // B - To be (a)
    {StenoStroke(0x10000 /*B*/), Tense::PRESENT, true, &ENDER_B, ""},
    {StenoStroke(0x210000 /*BD*/), Tense::PAST, true, &ENDER_BD, ""},
    {StenoStroke(0x90000 /*BT*/), Tense::PRESENT, true, &ENDER_B, " a"},
    {StenoStroke(0x290000 /*BTD*/), Tense::PAST, true, &ENDER_BD, " a"},

    // RPBG - To become (a)
    {StenoStroke(0x5c000 /*-RPBG*/), Tense::PRESENT, false, &ENDER_RPBG, ""},
    {StenoStroke(0x25c000 /*-RPBGD*/), Tense::PAST, false, &ENDER_RPBGD, ""},
    {StenoStroke(0xdc000 /*-RPBGT*/), Tense::PRESENT, false, &ENDER_RPBG, " a"},
    {StenoStroke(0x2dc000 /*-RPBGTD*/), Tense::PAST, false, &ENDER_RPBGD, " a"},

    // BL - To believe (that)
    {StenoStroke(0x30000 /*-BL*/), Tense::PRESENT, false, &ENDER_BL, ""},
    {StenoStroke(0x230000 /*-BLD*/), Tense::PAST, false, &ENDER_BLD, ""},
    {StenoStroke(0xb0000 /*-BLT*/), Tense::PRESENT, false, &ENDER_BL, " that"},
    {StenoStroke(0x2b0000 /*-BLTD*/), Tense::PAST, false, &ENDER_BLD, " that"},

    // RBLG - To call
    {StenoStroke(0x74000 /*-RBLG*/), Tense::PRESENT, false, &ENDER_RBLG, ""},
    {StenoStroke(0x274000 /*-RBLGD*/), Tense::PAST, false, &ENDER_RBLGD, ""},

    // BGS - Can -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x150000 /*-BGS*/), Tense::PRESENT, false, " can", ""},
    {StenoStroke(0x550000 /*-BGSZ*/), Tense::PAST, false, " could", ""},

    // RZ - To care
    {StenoStroke(0x404000 /*-RZ*/), Tense::PRESENT, false, &ENDER_RZ, ""},
    {StenoStroke(0x604000 /*-RDZ*/), Tense::PAST, false, &ENDER_RDZ, ""},

    // PBGZ - To change
    {StenoStroke(0x458000 /*-PBGZ*/), Tense::PRESENT, false, &ENDER_PBGZ, ""},
    {StenoStroke(0x658000 /*-PBGDZ*/), Tense::PAST, false, &ENDER_PBGDZ, ""},

    // BG - To come (to)
    {StenoStroke(0x50000 /*-BG*/), Tense::PRESENT, true, &ENDER_BG, ""},
    {StenoStroke(0x250000 /*-BGD*/), Tense::PAST, true, &ENDER_BGD, ""},
    {StenoStroke(0xd0000 /*-BGT*/), Tense::PRESENT, true, &ENDER_BG, " to"},
    {StenoStroke(0x2d0000 /*BGTD*/), Tense::PAST, true, &ENDER_BGD, " to"},

    // RBGZ - To consider
    {StenoStroke(0x454000 /*-RBGZ*/), Tense::PRESENT, false, &ENDER_RBGZ, ""},
    {StenoStroke(0x654000 /*-RBGDZ*/), Tense::PAST, false, &ENDER_RBGDZ, ""},

    // RP - To do (it)
    {StenoStroke(0xc000 /*-RP*/), Tense::PRESENT, false, &ENDER_RP, ""},
    {StenoStroke(0x20c000 /*-RPD*/), Tense::PAST, false, &ENDER_RPD, ""},
    {StenoStroke(0x8c000 /*-RPT*/), Tense::PRESENT, false, &ENDER_RP, " it"},
    {StenoStroke(0x28c000 /*-RPTD*/), Tense::PAST, false, &ENDER_RPD, " it"},

    // RP - To expect (that)
    {StenoStroke(0x148000 /*-PGS*/), Tense::PRESENT, false, &ENDER_PGS, ""},
    {StenoStroke(0x548000 /*-PGSZ*/), Tense::PAST, false, &ENDER_PGSZ, ""},
    {StenoStroke(0x1c8000 /*-PGTS*/), Tense::PRESENT, false, &ENDER_PGS, " that"},
    {StenoStroke(0x7c8000 /*-PGTSDZ*/), Tense::PAST, false, &ENDER_PGSZ, " that"},

    // LT - To feel (like)
    {StenoStroke(0xa0000 /*-LT*/), Tense::PRESENT, false, &ENDER_LT, ""},
    {StenoStroke(0x2a0000 /*-LTD*/), Tense::PAST, false, &ENDER_LTD, ""},
    {StenoStroke(0x1a0000 /*-LTS*/), Tense::PRESENT, false, &ENDER_LT, " like"},
    {StenoStroke(0x7a0000 /*-LTSDZ*/), Tense::PAST, false, &ENDER_LTD, " like"},

    // PBLG - To find (that)
    {StenoStroke(0x78000 /*-PBLG*/), Tense::PRESENT, false, &ENDER_PBLG, ""},
    {StenoStroke(0x278000 /*-PBLGD*/), Tense::PAST, false, &ENDER_PBLGD, ""},
    {StenoStroke(0xf8000 /*-PBLGT*/), Tense::PRESENT, false, &ENDER_PBLG, " that"},
    {StenoStroke(0x2f8000 /*-PBLGTD*/), Tense::PAST, false, &ENDER_PBLGD, " that"},

    // RG - To forget (to)
    {StenoStroke(0x44000 /*-RG*/), Tense::PRESENT, false, &ENDER_RG, ""},
    {StenoStroke(0x244000 /*-RGD*/), Tense::PAST, false, &ENDER_RGD, ""},
    {StenoStroke(0xc4000 /*-RGT*/), Tense::PRESENT, false, &ENDER_RG, " to"},
    {StenoStroke(0x2c4000 /*-RGTD*/), Tense::PAST, false, &ENDER_RGD, " to"},

    // GS - To get (to)
    {StenoStroke(0x140000 /*-GS*/), Tense::PRESENT, false, &ENDER_GS, ""},
    {StenoStroke(0x540000 /*-GSZ*/), Tense::PAST, false, &ENDER_GSZ, ""},
    {StenoStroke(0x1c0000 /*-GTS*/), Tense::PRESENT, false, &ENDER_GS, " to"},
    {StenoStroke(0x7c0000 /*-GTSDZ*/), Tense::PAST, false, &ENDER_GSZ, " to"},

    // GZ - To give
    {StenoStroke(0x440000 /*-GZ*/), Tense::PRESENT, false, &ENDER_GZ, ""},
    {StenoStroke(0x640000 /*-GDZ*/), Tense::PAST, false, &ENDER_GDZ, ""},

    // G - To go (to)
    {StenoStroke(0x40000 /*G*/), Tense::PRESENT, true, &ENDER_G, ""},
    {StenoStroke(0x240000 /*GD*/), Tense::PAST, true, &ENDER_GD, ""},
    {StenoStroke(0xC0000 /*GT*/), Tense::PRESENT, true, &ENDER_G, " to"},
    {StenoStroke(0x2C0000 /*GTD*/), Tense::PAST, true, &ENDER_GD, " to"},

    // PZ - To happen
    {StenoStroke(0x408000 /*-PZ*/), Tense::PRESENT, true, &ENDER_PZ, ""},
    {StenoStroke(0x608000 /*-PDZ*/), Tense::PAST, true, &ENDER_PDZ, ""},

    // H - To have (to)
    {StenoStroke(0x80000 /*T*/), Tense::PRESENT, true, &ENDER_T, ""},
    {StenoStroke(0x280000 /*TD*/), Tense::PAST, true, &ENDER_TD, ""},
    {StenoStroke(0x180000 /*TS*/), Tense::PRESENT, true, &ENDER_T, " to"},
    {StenoStroke(0x780000 /*TSDZ*/), Tense::PAST, true, &ENDER_TD, " to"},

    // PG - To hear (that)
    {StenoStroke(0x48000 /*-PG*/), Tense::PRESENT, false, &ENDER_PG, ""},
    {StenoStroke(0x248000 /*-PGD*/), Tense::PAST, false, &ENDER_PGD, ""},
    {StenoStroke(0xc8000 /*-PGT*/), Tense::PRESENT, false, &ENDER_PG, " that"},
    {StenoStroke(0x2c8000 /*-PGTD*/), Tense::PAST, false, &ENDER_PGD, " that"},

    // RPS - To hope (to)
    {StenoStroke(0x10c000 /*-RPS*/), Tense::PRESENT, false, &ENDER_RPS, ""},
    {StenoStroke(0x50c000 /*-RPSZ*/), Tense::PAST, false, &ENDER_RPSZ, ""},
    {StenoStroke(0x18c000 /*-RPTS*/), Tense::PRESENT, false, &ENDER_RPS, " to"},
    {StenoStroke(0x78c000 /*-RPTSDZ*/), Tense::PAST, false, &ENDER_RPSZ, " to"},

    // PLG - To imagine (that)
    {StenoStroke(0x68000 /*-PLG*/), Tense::PRESENT, false, &ENDER_PLG, ""},
    {StenoStroke(0x268000 /*-PLGD*/), Tense::PAST, false, &ENDER_PLGD, ""},
    {StenoStroke(0xe8000 /*-PLGT*/), Tense::PRESENT, false, &ENDER_PLG, " that"},
    {StenoStroke(0x2e8000 /*-PLGTD*/), Tense::PAST, false, &ENDER_PLGD, " that"},

    // PBLGSZ - just
    {StenoStroke(0x578000 /*-PBLGSZ*/), Tense::PRESENT, true, " just", ""},
    {StenoStroke(0x7f8000 /*-PBLGTSDZ*/), Tense::PAST, true, " just", ""},

    // PBGS - To keep
    {StenoStroke(0x158000 /*-PBGS*/), Tense::PRESENT, false, &ENDER_PBGS, ""},
    {StenoStroke(0x558000 /*-PBGSZ*/), Tense::PAST, false, &ENDER_PBGSZ, ""},

    // PB - To know (that)
    {StenoStroke(0x18000 /*-PB*/), Tense::PRESENT, false, &ENDER_PB, ""},
    {StenoStroke(0x218000 /*-PBD*/), Tense::PAST, false, &ENDER_PBD, ""},
    {StenoStroke(0x98000 /*-PBT*/), Tense::PRESENT, false, &ENDER_PB, " that"},
    {StenoStroke(0x298000 /*-PBTD*/), Tense::PAST, false, &ENDER_PBD, " that"},

    // RPBS - To learn (to)
    {StenoStroke(0x11c000 /*-RPBS*/), Tense::PRESENT, false, &ENDER_RPBS, ""},
    {StenoStroke(0x51c000 /*-RPBSZ*/), Tense::PAST, false, &ENDER_RPBSZ, ""},
    {StenoStroke(0x19c000 /*-RPBTS*/), Tense::PRESENT, false, &ENDER_RPBS, " to"},
    {StenoStroke(0x79c000 /*-RPBTSDZ*/), Tense::PAST, false, &ENDER_RPBSZ, " to"},

    // LGZ - To leave
    {StenoStroke(0x460000 /*-LGZ*/), Tense::PRESENT, false, &ENDER_LGZ, ""},
    {StenoStroke(0x660000 /*-LGDZ*/), Tense::PAST, false, &ENDER_LGDZ, ""},

    // LS - To let
    {StenoStroke(0x120000 /*-LS*/), Tense::PRESENT, false, &ENDER_LS, ""},
    {StenoStroke(0x520000 /*-LSZ*/), Tense::PAST, false, &ENDER_LSZ, ""},

    // BLG - To like (to)
    {StenoStroke(0x70000 /*-BLG*/), Tense::PRESENT, false, &ENDER_BLG, ""},
    {StenoStroke(0x270000 /*-BLGD*/), Tense::PAST, false, &ENDER_BLGD, ""},
    {StenoStroke(0xf0000 /*-BLGT*/), Tense::PRESENT, false, &ENDER_BLG, " to"},
    {StenoStroke(0x2f0000 /*-BLGTD*/), Tense::PAST, false, &ENDER_BLGD, " to"},

    // LZ - To live
    {StenoStroke(0x420000 /*-LZ*/), Tense::PRESENT, true, &ENDER_LZ, ""},
    {StenoStroke(0x620000 /*-LDZ*/), Tense::PAST, true, &ENDER_LDZ, ""},

    // L - To look
    {StenoStroke(0x20000 /*-L*/), Tense::PRESENT, false, &ENDER_L, ""},
    {StenoStroke(0x220000 /*-LD*/), Tense::PAST, false, &ENDER_LD, ""},

    // LG - To love (to)
    {StenoStroke(0x60000 /*-LG*/), Tense::PRESENT, false, &ENDER_LG, ""},
    {StenoStroke(0x260000 /*-LGD*/), Tense::PAST, false, &ENDER_LGD, ""},
    {StenoStroke(0xe0000 /*-LGT*/), Tense::PRESENT, false, &ENDER_LG, " to"},
    {StenoStroke(0x2e0000 /*-LGTD*/), Tense::PAST, false, &ENDER_LGD, " to"},

    // RPBL - To make (a)
    {StenoStroke(0x3c000 /*-RPBL*/), Tense::PRESENT, false, &ENDER_RPBL, ""},
    {StenoStroke(0x23c000 /*-RPBLD*/), Tense::PAST, false, &ENDER_RPBLD, ""},
    {StenoStroke(0xbc000 /*-RPBLT*/), Tense::PRESENT, false, &ENDER_RPBL, " a"},
    {StenoStroke(0x2bc000 /*-RPBLTD*/), Tense::PAST, false, &ENDER_RPBLD, " a"},

    // PL - may (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x28000 /*-PL*/), Tense::PRESENT, true, " may", ""},
    {StenoStroke(0x228000 /*-PLD*/), Tense::PAST, true, " might", ""},
    {StenoStroke(0xa8000 /*-PLT*/), Tense::PRESENT, true, " may", " be"},
    {StenoStroke(0x2a8000 /*-PLTD*/), Tense::PAST, true, " might", " be"},

    // PBL - To mean (to)
    {StenoStroke(0x38000 /*-PBL*/), Tense::PRESENT, false, &ENDER_PBL, ""},
    {StenoStroke(0x238000 /*-PBLD*/), Tense::PAST, false, &ENDER_PBLD, ""},
    {StenoStroke(0xb8000 /*-PBLT*/), Tense::PRESENT, false, &ENDER_PBL, " to"},
    {StenoStroke(0x2b8000 /*-PBLTD*/), Tense::PAST, false, &ENDER_PBLD, " to"},

    // PBLS - To mind
    {StenoStroke(0x138000 /*-PBLS*/), Tense::PRESENT, false, &ENDER_PBLS, ""},
    {StenoStroke(0x538000 /*-PBLSZ*/), Tense::PAST, false, &ENDER_PBLSZ, ""},

    // PLZ - To move
    {StenoStroke(0x428000 /*-PLZ*/), Tense::PRESENT, false, &ENDER_PLZ, ""},
    {StenoStroke(0x628000 /*-PLDZ*/), Tense::PAST, false, &ENDER_PLDZ, ""},

    // PBLGS - must (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x178000 /*-PBLGS*/), Tense::PRESENT, true, " must", ""},
    {StenoStroke(0x1f8000 /*-PBLGTS*/), Tense::PRESENT, true, " must", " be"},

    // RPG - To need (to)
    {StenoStroke(0x4c000 /*-RPG*/), Tense::PRESENT, true, &ENDER_RPG, ""},
    {StenoStroke(0x24c000 /*-RPGD*/), Tense::PAST, true, &ENDER_RPGD, ""},
    {StenoStroke(0xcc000 /*-RPGT*/), Tense::PRESENT, true, &ENDER_RPG, " to"},
    {StenoStroke(0x2cc000 /*-RPGTD*/), Tense::PAST, true, &ENDER_RPGD, " to"},

    // PS - To put (it)
    {StenoStroke(0x108000 /*-PS*/), Tense::PRESENT, false, &ENDER_PS, ""},
    {StenoStroke(0x508000 /*-PSZ*/), Tense::PAST, false, &ENDER_PSZ, ""},
    {StenoStroke(0x188000 /*-PTS*/), Tense::PRESENT, false, &ENDER_PS, " it"},
    {StenoStroke(0x788000 /*-PTSDZ*/), Tense::PAST, false, &ENDER_PSZ, " it"},

    // RS - To read
    {StenoStroke(0x104000 /*-RS*/), Tense::PRESENT, false, &ENDER_RS, ""},
    {StenoStroke(0x504000 /*-RSZ*/), Tense::PAST, false, &ENDER_RSZ, ""},

    // RLG - really
    {StenoStroke(0x64000 /*-RLG*/), Tense::PRESENT, true, " really", ""},
    {StenoStroke(0x264000 /*-RLG*/), Tense::PAST, true, " really", ""},

    // RL - To recall
    {StenoStroke(0x24000 /*-RL*/), Tense::PRESENT, false, &ENDER_RL, ""},
    {StenoStroke(0x224000 /*-RLD*/), Tense::PAST, false, &ENDER_RLD, ""},

    // RLS - To realize (that)
    {StenoStroke(0x124000 /*-RLS*/), Tense::PRESENT, false, &ENDER_RLS, ""},
    {StenoStroke(0x524000 /*-RLSZ*/), Tense::PAST, false, &ENDER_RLSZ, ""},
    {StenoStroke(0x1a4000 /*-RLTS*/), Tense::PRESENT, false, &ENDER_RLS, " that"},
    {StenoStroke(0x7a4000 /*-RLTSDZ*/), Tense::PAST, false, &ENDER_RLSZ, " that"},

    // RPL - To remember (that)
    {StenoStroke(0x2c000 /*-RPL*/), Tense::PRESENT, false, &ENDER_RPL, ""},
    {StenoStroke(0x22c000 /*-RPLD*/), Tense::PAST, false, &ENDER_RPLD, ""},
    {StenoStroke(0xac000 /*-RPLT*/), Tense::PRESENT, false, &ENDER_RPL, " that"},
    {StenoStroke(0x2ac000 /*-RPLTD*/), Tense::PAST, false, &ENDER_RPLD, " that"},

    // RPLS - To remain
    {StenoStroke(0x12c000 /*-RPLS*/), Tense::PRESENT, false, &ENDER_RPLS, ""},
    {StenoStroke(0x52c000 /*-RPLSZ*/), Tense::PAST, false, &ENDER_RPLSZ, ""},

    // R - To run
    {StenoStroke(0x4000 /*-R*/), Tense::PRESENT, false, &ENDER_R, ""},
    {StenoStroke(0x204000 /*-RD*/), Tense::PAST, false, &ENDER_RD, ""},

    // BS - To say (that)
    {StenoStroke(0x110000 /*-BS*/), Tense::PRESENT, false, &ENDER_BS, ""},
    {StenoStroke(0x510000 /*-BSZ*/), Tense::PAST, false, &ENDER_BSZ, ""},
    {StenoStroke(0x190000 /*-BTS*/), Tense::PRESENT, false, &ENDER_BS, " that"},
    {StenoStroke(0x790000 /*-BTSDZ*/), Tense::PAST, false, &ENDER_BSZ, " that"},

    // S - To see
    {StenoStroke(0x100000 /*-S*/), Tense::PRESENT, false, &ENDER_S, ""},
    {StenoStroke(0x500000 /*-SZ*/), Tense::PAST, false, &ENDER_SZ, ""},

    // BLS - To set
    {StenoStroke(0x130000 /*-BLS*/), Tense::PRESENT, false, &ENDER_BLS, ""},
    {StenoStroke(0x530000 /*-BLSZ*/), Tense::PAST, false, &ENDER_BLSZ, ""},

    // PLS - To seem (to)
    {StenoStroke(0x128000 /*-PLS*/), Tense::PRESENT, true, &ENDER_PLS, ""},
    {StenoStroke(0x528000 /*-PLSZ*/), Tense::PAST, true, &ENDER_PLSZ, ""},
    {StenoStroke(0x1a8000 /*-PLTS*/), Tense::PRESENT, true, &ENDER_PLS, " to"},
    {StenoStroke(0x7a8000 /*-PLTSDZ*/), Tense::PAST, true, &ENDER_PLSZ, " to"},

    // RBL - shall - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x34000 /*RBL*/), Tense::PRESENT, false, " shall", ""},
    {StenoStroke(0x234000 /*RBLD*/), Tense::PAST, false, " should", ""},

    // RBZ - To show
    {StenoStroke(0x414000 /*-RBZ*/), Tense::PRESENT, false, &ENDER_RBZ, ""},
    {StenoStroke(0x614000 /*-RBDZ*/), Tense::PAST, false, &ENDER_RBDZ, ""},

    // RBT - To take
    {StenoStroke(0x94000 /*-RBT*/), Tense::PRESENT, false, &ENDER_RBT, ""},
    {StenoStroke(0x294000 /*-RBTD*/), Tense::PAST, false, &ENDER_RBTD, ""},

    // // BLGT - To talk
    // {StenoStroke(0xf0000 /*-BLGT*/), Tense::PRESENT, false, &ENDER_BLGT, ""},
    // {StenoStroke(0x2f0000 /*-BLGTD*/), Tense::PAST, false, &ENDER_BLGTD, ""},

    // RLT - To tell
    {StenoStroke(0xa4000 /*-RLT*/), Tense::PRESENT, false, &ENDER_RLT, ""},
    {StenoStroke(0x2a4000 /*-RLTD*/), Tense::PAST, false, &ENDER_RLTD, ""},

    // PBG - To think (that)
    {StenoStroke(0x58000 /*-PBG*/), Tense::PRESENT, false, &ENDER_PBG, ""},
    {StenoStroke(0x258000 /*-PBGD*/), Tense::PAST, false, &ENDER_PBGD, ""},
    {StenoStroke(0xd8000 /*-PBGT*/), Tense::PRESENT, false, &ENDER_PBG, " that"},
    {StenoStroke(0x2d8000 /*-PBGTD*/), Tense::PAST, false, &ENDER_PBGD, " that"},

    // RT - To try (to)
    {StenoStroke(0x84000 /*-RT*/), Tense::PRESENT, false, &ENDER_RT, ""},
    {StenoStroke(0x284000 /*-RTD*/), Tense::PAST, false, &ENDER_RTD, ""},
    {StenoStroke(0x184000 /*-RTS*/), Tense::PRESENT, false, &ENDER_RT, " to"},
    {StenoStroke(0x784000 /*-RTSDZ*/), Tense::PAST, false, &ENDER_RTD, " to"},

    // RPB - To understand (the)
    {StenoStroke(0x1c000 /*-RPB*/), Tense::PRESENT, false, &ENDER_RPB, ""},
    {StenoStroke(0x21c000 /*-RPBD*/), Tense::PAST, false, &ENDER_RPBD, ""},
    {StenoStroke(0x9c000 /*-RPBT*/), Tense::PRESENT, false, &ENDER_RPB, " the"},
    {StenoStroke(0x29c000 /*-RPBTD*/), Tense::PAST, false, &ENDER_RPBD, " the"},

    // Z - To use
    {StenoStroke(0x400000 /*-Z*/), Tense::PRESENT, true, &ENDER_Z, ""},
    {StenoStroke(0x600000 /*-DZ*/), Tense::PAST, true, &ENDER_DZ, ""},
    // TZ - Special case
    {StenoStroke(0x480000 /*-TZ*/), Tense::PRESENT, true, " used", " to"},
    {StenoStroke(0x680000 /*-TDZ*/), Tense::PAST, true, " used", " to"},

    // P - To want (to)
    {StenoStroke(0x8000 /*P*/), Tense::PRESENT, false, &ENDER_P, ""},
    {StenoStroke(0x208000 /*PD*/), Tense::PAST, false, &ENDER_PD, ""},
    {StenoStroke(0x88000 /*PT*/), Tense::PRESENT, false, &ENDER_P, " to"},
    {StenoStroke(0x288000 /*PTD*/), Tense::PAST, false, &ENDER_PD, " to"},

    // RBGS - will -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x154000 /*RBGS*/), Tense::PRESENT, false, " will", ""},
    {StenoStroke(0x554000 /*RBGSZ*/), Tense::PAST, false, " would", ""},

    // RBS - To wish (to)
    {StenoStroke(0x114000 /*-RBS*/), Tense::PRESENT, false, &ENDER_RBS, ""},
    {StenoStroke(0x514000 /*-RBSZ*/), Tense::PAST, false, &ENDER_RBSZ, ""},
    {StenoStroke(0x194000 /*-RBTS*/), Tense::PRESENT, false, &ENDER_RBS, " to"},
    {StenoStroke(0x794000 /*-RBTSDZ*/), Tense::PAST, false, &ENDER_RBSZ, " to"},

    // RBG - To work (on)
    {StenoStroke(0x54000 /*-RBG*/), Tense::PRESENT, false, &ENDER_RBG, ""},
    {StenoStroke(0x254000 /*-RBGD*/), Tense::PAST, false, &ENDER_RBGD, ""},
    {StenoStroke(0xd4000 /*-RBGT*/), Tense::PRESENT, false, &ENDER_RBG, " on"},
    {StenoStroke(0x2d4000 /*-RBGTD*/), Tense::PAST, false, &ENDER_RBGD, " on"},
};
// clang-format on

constexpr StenoStroke NON_PHRASE_STROKES[] = {
    0xc6,    // STHR       - is there
    0x808c6, // STHRET     - stiletto
    0xa98c6, // STHREUPLT  - stimulate
    0xaa8d6, // STPHREFPLT - investment in
    0x501a,  // SKPUR      - and you're -- rather than 'and you run'
    0x2101a, // SKPUL      - and you'll -- rather than 'and you look'
    0x8181a, // SKPEUT     - and it     -- rather than 'and I have'
    0x41a,   // SKP*       - {&&}
};
// spellchecker: enable

//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary_generated.h"

const JeffPhrasingDictionaryData JeffPhrasingDictionaryData::instance = {
    .simpleStarterCount = sizeof(SIMPLE_STARTERS) / sizeof(*SIMPLE_STARTERS),
    .simpleStarters = SIMPLE_STARTERS,

    .simplePronouns = SIMPLE_PRONOUNS,
    .simpleStructures = SIMPLE_STRUCTURES,

    .fullStarterCount = sizeof(FULL_STARTERS) / sizeof(*FULL_STARTERS),
    .fullStarters = FULL_STARTERS,

    .fullMiddles = MIDDLES,
    .fullStructures = STRUCTURES,

    .structureExceptionCount =
        sizeof(STRUCTURE_EXCEPTIONS) / sizeof(*STRUCTURE_EXCEPTIONS),
    .structureExceptions = STRUCTURE_EXCEPTIONS,

    .endersCount = sizeof(ENDERS) / sizeof(*ENDERS),
    .enders = ENDERS,

    .nonPhraseStrokeCount =
        sizeof(NON_PHRASE_STROKES) / sizeof(*NON_PHRASE_STROKES),
    .nonPhraseStrokes = NON_PHRASE_STROKES,

    .uniqueStarterCount = sizeof(UNIQUE_STARTERS) / sizeof(*UNIQUE_STARTERS),
    .uniqueStarters = UNIQUE_STARTERS,

    .enderHashMapSize = ENDER_HASH_MAP_SIZE,
    .enderHashMap = ENDER_HASH_MAP,

    .reverseHashMapSize =
        sizeof(REVERSE_HASH_MAP) / sizeof(*REVERSE_HASH_MAP), // NOLINT
    .reverseHashMap = REVERSE_HASH_MAP,
    .reverseEntries = REVERSE_ENTRIES,

    .reverseStructureHashMapSize = sizeof(REVERSE_STRUCTURES_MAP) /
                                   sizeof(*REVERSE_STRUCTURES_MAP), // NOLINT
    .reverseStructureHashMap = REVERSE_STRUCTURES_MAP,
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define GENERATE_TABLES 0

#if GENERATE_TABLES

#define phrasingData JeffPhrasingDictionaryData::instance
#include <stdio.h>
#include <vector>

uint8_t charactersPresent[32] = {};

static void RegisterCharacters(const char *p, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    int c = p[i];
    charactersPresent[c / 8] |= 1 << c % 8;
  }
}

static void PrintCharacters() {
  printf("const uint8_t VALID_CHARACTERS[32] = {\n");
  for (size_t i = 0; i < 32; ++i) {
    if (i != 0) {
      printf(", ");
    }
    printf("0x%02x", charactersPresent[i]);
  }
  printf(",\n};\n");
  printf("\n");
}

void GenerateLookupTable() {
  // printf("endersCount: %zu\n", phrasingData.endersCount);

  static const size_t HASH_SIZE = 512;

  const JeffPhrasingEnder **enderMap = new const JeffPhrasingEnder *[512];
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    enderMap[i] = nullptr;
  }

  for (size_t i = 0; i < phrasingData.endersCount; ++i) {
    const JeffPhrasingEnder *ender = &phrasingData.enders[i];
    size_t index = ender->stroke.Hash() & (HASH_SIZE - 1);

    while (enderMap[index] != nullptr) {
      index = (index + 1) & (HASH_SIZE - 1);
    }
    enderMap[index] = ender;
  }

  printf("const size_t ENDER_HASH_MAP_SIZE = %zu;\n", HASH_SIZE);
  printf("const JeffPhrasingEnder *const ENDER_HASH_MAP[] = {\n");
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    if (enderMap[i] == nullptr) {
      printf("  nullptr,\n");
    } else {
      size_t offset = enderMap[i] - phrasingData.enders;
      printf("  ENDERS + %zu,\n", offset);
    }
  }
  printf("};\n");
}

class ReverseBuilder {
public:
  void Generate();

private:
  struct Entry {
    uint32_t hash;
    uint32_t replacementHash;
    StenoStroke stroke;
    bool checkNext;
    bool optionalReplace;
    uint8_t componentMask;
    uint8_t modeMask;
  };

  std::vector<Entry> entries;

  void RecurseVariant(const JeffPhrasingVariant &variant, StenoStroke stroke,
                      uint32_t replacement, uint8_t componentMask,
                      uint8_t modeMask, const char *suffix);

  void AddText(const char *text, StenoStroke stroke, uint32_t replacement,
               uint8_t componentMask, uint8_t modeMask);
};

void ReverseBuilder::Generate() {
  uint32_t starterReplacement = Crc32("\\0", 2);
  for (size_t i = 0; i < phrasingData.fullStarterCount; ++i) {
    AddText(phrasingData.fullStarters[i].pronoun.word,
            phrasingData.fullStarters[i].stroke, starterReplacement,
            ComponentMask::STARTER,
            ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (size_t i = 1; i < 8; ++i) {
    const JeffPhrasingPronoun &pronoun = phrasingData.simplePronouns[i];
    AddText(pronoun.word, StenoStroke(uint32_t(i) << StrokeBitIndex::STAR),
            starterReplacement, ComponentMask::STARTER,
            ModeMask::SIMPLE | ModeMask::PRESENT | ModeMask::PAST);
  }

  uint32_t middleReplacement = Crc32("\\1", 2);
  for (size_t i = 0; i < 8; ++i) {
    RecurseVariant(phrasingData.fullMiddles[i].word,
                   StenoStroke(uint32_t(i) << StrokeBitIndex::A),
                   middleReplacement, ComponentMask::MIDDLE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  for (size_t i = 0; i < phrasingData.simpleStarterCount; ++i) {
    RecurseVariant(phrasingData.simpleStarters[i].middle.word,
                   phrasingData.simpleStarters[i].stroke, middleReplacement,
                   ComponentMask::MIDDLE,
                   ModeMask::SIMPLE | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  for (size_t i = 0; i < 16; ++i) {
    const JeffPhrasingStructure &structure = phrasingData.fullStructures[i];
    RecurseVariant(
        structure.format, StenoStroke(uint32_t(i) << StrokeBitIndex::STAR), 0,
        ComponentMask::STRUCTURE,
        ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST, nullptr);
  }

  for (size_t i = 0; i < 2; ++i) {
    const JeffPhrasingStructure &structure = phrasingData.simpleStructures[i];
    RecurseVariant(
        structure.format, StenoStroke(uint32_t(i) << StrokeBitIndex::FR), 0,
        ComponentMask::STRUCTURE,
        ModeMask::SIMPLE | ModeMask::PRESENT | ModeMask::PAST, nullptr);
  }

  for (size_t i = 0; i < phrasingData.structureExceptionCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.structureExceptions[i];
    RecurseVariant(structure.structure.format, structure.stroke, 0,
                   ComponentMask::MIDDLE | ComponentMask::STRUCTURE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  for (size_t i = 0; i < phrasingData.uniqueStarterCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.uniqueStarters[i];
    RecurseVariant(structure.structure.format, structure.stroke, 0,
                   ComponentMask::STARTER | ComponentMask::MIDDLE |
                       ComponentMask::STRUCTURE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  uint32_t verbReplacement = Crc32("\\2", 2);
  uint32_t suffixReplacement = Crc32("\\3", 2);

  for (size_t i = 0; i < phrasingData.endersCount; ++i) {
    const JeffPhrasingEnder &ender = phrasingData.enders[i];
    uint8_t modeMask = ModeMask::FULL | ModeMask::SIMPLE;
    if (ender.tense == Tense::PAST) {
      modeMask |= ModeMask::PAST;
    } else {
      modeMask |= ModeMask::PRESENT;
    }
    RecurseVariant(ender.ender, ender.stroke,
                   verbReplacement + suffixReplacement, ComponentMask::VERB,
                   modeMask, ender.suffix);
  }

  std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
    if (a.hash != b.hash) {
      return a.hash < b.hash;
    }
    if (a.checkNext != b.checkNext) {
      return a.checkNext;
    }
    return a.stroke.GetKeyState() < b.stroke.GetKeyState();
  });
  for (size_t i = 0; i < entries.size() - 1; ++i) {
    Entry &entry = entries[i];
    Entry &next = entries[i + 1];
    if (entry.hash == next.hash && entry.stroke.IsEmpty() &&
        entry.replacementHash == 0 && entry.checkNext == true) {
      next.checkNext = true;
      entries.erase(entries.begin() + i);
      --i;
      continue;
    }

    if (entry.hash == next.hash && entry.stroke == next.stroke &&
        entry.replacementHash == next.replacementHash &&
        entry.componentMask == next.componentMask) {
      entry.modeMask |= next.modeMask;
      entries.erase(entries.begin() + i + 1);
      --i;
      continue;
    }
  }
  Entry sentinel = {};
  entries.push_back(sentinel);

  printf("constexpr JeffPhrasingReverseHashMapEntry REVERSE_ENTRIES[] = {\n");
  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry &entry = entries[i];
    printf("  { 0x%08x, 0x%08x, StenoStroke(0x%08x), %d, %d, "
           "%d},\n",
           entry.hash, entry.replacementHash, entry.stroke.GetKeyState(),
           entry.checkNext ? 1 : 0, entry.componentMask, entry.modeMask);
  }
  printf("};\n\n");

  size_t minimumHashMapSize = entries.size() + (entries.size() >> 1);
  size_t hashMapSize = 4;
  while (hashMapSize < minimumHashMapSize) {
    hashMapSize *= 2;
  }

  std::vector<Entry *> hashMap(hashMapSize, nullptr);
  for (size_t i = 0; i < entries.size(); ++i) {
    if (i > 1 && entries[i - 1].hash == entries[i].hash) {
      continue;
    }

    int index = entries[i].hash;
    for (;;) {
      if (hashMap[index & (hashMapSize - 1)] == nullptr) {
        hashMap[index & (hashMapSize - 1)] = &entries[i];
        break;
      }
      ++index;
    }
  }

  printf("constexpr const JeffPhrasingReverseHashMapEntry "
         "*REVERSE_HASH_MAP[] = {\n");
  for (size_t i = 0; i < hashMap.size(); ++i) {
    if (hashMap[i] == nullptr) {
      printf("  nullptr,\n");
    } else {
      printf("  REVERSE_ENTRIES+%zu,\n", hashMap[i] - &entries[0]);
    }
  }
  printf("};\n\n");
}

void ReverseBuilder::RecurseVariant(const JeffPhrasingVariant &variant,
                                    StenoStroke stroke, uint32_t replacement,
                                    uint8_t componentMask, uint8_t modeMask,
                                    const char *suffix) {
  switch (variant.type) {
  case JeffPhrasingVariant::Type::MAP: {
    const JeffPhrasingMap *map = variant.map;
    for (size_t i = 0; i < map->entryCount; ++i) {
      uint8_t localModeMask = modeMask;
      if (map->entries[i].key == (uint32_t)Tense::PRESENT) {
        localModeMask &= ~ModeMask::PAST;
      }
      if (map->entries[i].key == (uint32_t)Tense::PAST) {
        localModeMask &= ~ModeMask::PRESENT;
      }
      RecurseVariant(map->entries[i].value, stroke, replacement, componentMask,
                     localModeMask, suffix);
    }
  } break;

  case JeffPhrasingVariant::Type::UNKNOWN:
    break;

  case JeffPhrasingVariant::Type::TEXT:
    if (suffix != nullptr) {
      char buffer[64];
      snprintf(buffer, 63, "%s%s", variant.text, suffix);
      AddText(buffer, stroke, replacement, componentMask, modeMask);
    } else {
      AddText(variant.text, stroke, replacement, componentMask, modeMask);
    }
    break;
  }
}

void ReverseBuilder::AddText(const char *text, StenoStroke stroke,
                             uint32_t replacement, uint8_t componentMask,
                             uint8_t modeMask) {
  const char *p = text;
  while (*p) {
    if (*p == ' ') {
      ++p;
      continue;
    }
    if (p[0] == '\\' && '0' <= p[1] && p[1] <= '3') {
      p += 2;
      continue;
    }
    break;
  }

  const char *pEnd = p;
  while (*pEnd) {
    if (pEnd[0] == '\\' && '0' <= pEnd[1] && pEnd[1] <= '3') {
      break;
    }
    ++pEnd;
  }

  if (p == pEnd && replacement == 0) {
    return;
  }

  Entry entry = {};
  entry.hash = Crc32(p, pEnd - p);
  RegisterCharacters(p, pEnd - p);
  entry.replacementHash = replacement == 0 ? entry.hash : replacement;
  entry.stroke = stroke;
  entry.componentMask = componentMask;
  entry.modeMask = modeMask;
  entries.push_back(entry);

  const char *space = (const char *)memchr(p, ' ', pEnd - p);

  if (space) {
    Entry entry = {};
    entry.modeMask = modeMask;
    entry.hash = Crc32(p, space - p);
    RegisterCharacters(p, space - p);
    entry.checkNext = true;
    entries.push_back(entry);
  }
}

class ReverseStructureBuilder {
public:
  void Generate();

private:
  struct Entry {
    uint32_t hash;
    StenoStroke stroke;
    uint8_t modeMask;
  };

  std::vector<Entry> entries;

  void RecurseVariant(const JeffPhrasingVariant &variant, StenoStroke stroke,
                      uint8_t modeMask);
};

void ReverseStructureBuilder::Generate() {
  for (size_t i = 0; i < 16; ++i) {
    RecurseVariant(phrasingData.fullStructures[i].format,
                   StenoStroke(uint32_t(i & ~1) << StrokeBitIndex::STAR),
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (size_t i = 0; i < 2; ++i) {
    RecurseVariant(phrasingData.simpleStructures[i].format,
                   StenoStroke(uint32_t(i) << StrokeBitIndex::FR),
                   ModeMask::SIMPLE | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (size_t i = 0; i < phrasingData.structureExceptionCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.structureExceptions[i];
    RecurseVariant(structure.structure.format, structure.stroke,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (size_t i = 0; i < phrasingData.uniqueStarterCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.uniqueStarters[i];
    RecurseVariant(structure.structure.format, structure.stroke,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
    if (a.hash != b.hash) {
      return a.hash < b.hash;
    }
    if (a.stroke.GetKeyState() != b.stroke.GetKeyState()) {
      return a.stroke.GetKeyState() < b.stroke.GetKeyState();
    }
    return a.modeMask < b.modeMask;
  });
  for (size_t i = 0; i < entries.size() - 1; ++i) {
    Entry &entry = entries[i];
    Entry &next = entries[i + 1];
    if (entry.hash == next.hash && entry.stroke == next.stroke) {
      entry.modeMask |= next.modeMask;
      entries.erase(entries.begin() + i + 1);
      --i;
    }
  }
  Entry sentinel;
  sentinel.hash = 0;
  sentinel.stroke = StenoStroke();
  sentinel.modeMask = 0;
  entries.push_back(sentinel);

  printf(
      "constexpr JeffPhrasingReverseStructureEntry REVERSE_STRUCTURES[] = {\n");
  for (size_t i = 0; i < entries.size(); ++i) {
    printf("  { 0x%08x, StenoStroke(0x%08x), %d},\n", entries[i].hash,
           entries[i].stroke.GetKeyState(), entries[i].modeMask);
  }
  printf("};\n\n");

  size_t minimumHashMapSize = entries.size() + (entries.size() >> 1);
  size_t hashMapSize = 4;
  while (hashMapSize < minimumHashMapSize) {
    hashMapSize *= 2;
  }

  std::vector<Entry *> hashMap(hashMapSize, nullptr);
  for (size_t i = 0; i < entries.size(); ++i) {
    if (i > 1 && entries[i - 1].hash == entries[i].hash) {
      continue;
    }

    int index = entries[i].hash;
    for (;;) {
      if (hashMap[index & (hashMapSize - 1)] == nullptr) {
        hashMap[index & (hashMapSize - 1)] = &entries[i];
        break;
      }
      ++index;
    }
  }

  printf("constexpr const JeffPhrasingReverseStructureEntry "
         "*REVERSE_STRUCTURES_MAP[] = {\n");
  for (size_t i = 0; i < hashMap.size(); ++i) {
    if (hashMap[i] == nullptr) {
      printf("  nullptr,\n");
    } else {
      printf("  REVERSE_STRUCTURES+%zu,\n", hashMap[i] - &entries[0]);
    }
  }
  printf("};\n\n");
}

void ReverseStructureBuilder::RecurseVariant(const JeffPhrasingVariant &variant,
                                             StenoStroke stroke,
                                             uint8_t modeMask) {
  switch (variant.type) {
  case JeffPhrasingVariant::Type::MAP: {
    const JeffPhrasingMap *map = variant.map;
    for (size_t i = 0; i < map->entryCount; ++i) {
      uint8_t localModeMask = modeMask;
      if (map->entries[i].key == (uint32_t)Tense::PRESENT) {
        localModeMask &= ~ModeMask::PAST;
      }
      if (map->entries[i].key == (uint32_t)Tense::PAST) {
        localModeMask &= ~ModeMask::PRESENT;
      }
      RecurseVariant(map->entries[i].value, stroke, localModeMask);
    }
  } break;

  case JeffPhrasingVariant::Type::UNKNOWN:
    break;

  case JeffPhrasingVariant::Type::TEXT: {
    uint32_t hash = 0;
    const char *p = variant.text;
    for (;;) {
      // Skip leading spaces.
      while (*p == ' ' && *p != '\0') {
        ++p;
      }

      if (*p == '\0')
        break;

      if (p[0] == '\\' && ('0' <= p[1] && p[1] <= '3')) {
        hash += Crc32(p, 2);
        p += 2;
        continue;
      }

      const char *pEnd = p;
      while (*pEnd != '\\' && *pEnd != '\0') {
        ++pEnd;
      }
      RegisterCharacters(p, pEnd - p);
      if (pEnd[-1] == ' ') {
        --pEnd;
      }
      hash += Crc32(p, pEnd - p);
      p = pEnd;
    }

    Entry entry;
    entry.stroke = stroke;
    entry.modeMask = modeMask;
    entry.hash = hash;
    entries.push_back(entry);

  } break;
  }
}

int main(int, const char **) {
  printf("#pragma once\n");
  printf("#include \"jeff_phrasing_dictionary_data.h\"\n");
  printf("\n");
  printf("extern const JeffPhrasingEnder ENDERS[];\n");
  printf("\n");

  printf(
      "const uint32_t STARTER_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\0\"\n",
      Crc32("\\0", 2));
  printf("const uint32_t MIDDLE_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\1\"\n",
         Crc32("\\1", 2));
  printf("const uint32_t VERB_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\2\"\n",
         Crc32("\\2", 2));
  printf("const uint32_t SUFFIX_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\3\"\n",
         Crc32("\\3", 2));

  GenerateLookupTable();

  ReverseBuilder reverseBuilder;
  reverseBuilder.Generate();

  ReverseStructureBuilder reverseStructureBuilder;
  reverseStructureBuilder.Generate();

  PrintCharacters();
  return 0;
}

#endif
