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

const size_t ENDER_HASH_MAP_SIZE = 512;
const JeffPhrasingEnder *const ENDER_HASH_MAP[] = {
    nullptr,      nullptr,      ENDERS + 123, nullptr,      ENDERS + 152,
    nullptr,      nullptr,      ENDERS + 140, ENDERS + 71,  nullptr,
    ENDERS + 122, ENDERS + 119, ENDERS + 150, ENDERS + 19,  nullptr,
    ENDERS + 138, ENDERS + 58,  ENDERS + 184, nullptr,      nullptr,
    nullptr,      ENDERS + 180, ENDERS + 112, nullptr,      ENDERS + 56,
    ENDERS + 182, nullptr,      ENDERS + 16,  nullptr,      ENDERS + 178,
    ENDERS + 110, nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 105, nullptr,
    nullptr,      ENDERS + 199, nullptr,      ENDERS + 109, ENDERS + 189,
    nullptr,      ENDERS + 103, ENDERS + 99,  ENDERS + 165, ENDERS + 197,
    nullptr,      ENDERS + 107, ENDERS + 187, nullptr,      nullptr,
    ENDERS + 52,  nullptr,      nullptr,      ENDERS + 88,  nullptr,
    nullptr,      ENDERS + 160, ENDERS + 155, ENDERS + 50,  nullptr,
    nullptr,      ENDERS + 86,  nullptr,      nullptr,      ENDERS + 158,
    ENDERS + 126, nullptr,      nullptr,      ENDERS + 44,  nullptr,
    ENDERS + 6,   ENDERS + 168, nullptr,      ENDERS + 124, nullptr,
    nullptr,      ENDERS + 42,  nullptr,      ENDERS + 4,   ENDERS + 136,
    nullptr,      ENDERS + 41,  nullptr,      nullptr,      ENDERS + 195,
    nullptr,      nullptr,      ENDERS + 37,  nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 173, nullptr,      ENDERS + 15,  ENDERS + 175, nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 171, nullptr,
    ENDERS + 13,  ENDERS + 149, nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 176, nullptr,
    nullptr,      ENDERS + 80,  nullptr,      ENDERS + 147, nullptr,
    nullptr,      ENDERS + 132, ENDERS + 156, nullptr,      nullptr,
    ENDERS + 198, ENDERS + 104, nullptr,      ENDERS + 188, nullptr,
    nullptr,      ENDERS + 108, ENDERS + 164, ENDERS + 196, ENDERS + 102,
    ENDERS + 98,  ENDERS + 186, nullptr,      nullptr,      ENDERS + 106,
    nullptr,      nullptr,      ENDERS + 73,  nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 59,  ENDERS + 185,
    ENDERS + 113, nullptr,      nullptr,      ENDERS + 181, nullptr,
    ENDERS + 17,  ENDERS + 57,  ENDERS + 183, ENDERS + 111, nullptr,
    nullptr,      ENDERS + 179, nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 130, nullptr,      nullptr,
    ENDERS + 190, ENDERS + 193, ENDERS + 93,  nullptr,      ENDERS + 35,
    ENDERS + 128, nullptr,      nullptr,      nullptr,      ENDERS + 172,
    nullptr,      nullptr,      ENDERS + 174, nullptr,      nullptr,
    ENDERS + 14,  nullptr,      ENDERS + 170, nullptr,      nullptr,
    ENDERS + 148, nullptr,      nullptr,      ENDERS + 12,  nullptr,
    nullptr,      ENDERS + 65,  nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 45,  ENDERS + 127, nullptr,      ENDERS + 169,
    nullptr,      nullptr,      ENDERS + 7,   nullptr,      ENDERS + 43,
    ENDERS + 125, nullptr,      ENDERS + 137, nullptr,      nullptr,
    ENDERS + 5,   nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 72,  nullptr,      nullptr,      ENDERS + 18,
    ENDERS + 151, ENDERS + 139, nullptr,      nullptr,      ENDERS + 70,
    ENDERS + 118, ENDERS + 78,  nullptr,      ENDERS + 10,  ENDERS + 76,
    nullptr,      ENDERS + 62,  nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 8,   ENDERS + 74,  ENDERS + 120, ENDERS + 0,
    ENDERS + 54,  nullptr,      ENDERS + 162, nullptr,      nullptr,
    ENDERS + 177, nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 69,  nullptr,      ENDERS + 39,  nullptr,
    nullptr,      ENDERS + 167, nullptr,      nullptr,      ENDERS + 67,
    ENDERS + 61,  ENDERS + 101, ENDERS + 91,  ENDERS + 29,  ENDERS + 3,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 64,  nullptr,      nullptr,      nullptr,      ENDERS + 87,
    ENDERS + 159, nullptr,      ENDERS + 51,  ENDERS + 154, nullptr,
    nullptr,      nullptr,      ENDERS + 26,  nullptr,      nullptr,
    ENDERS + 32,  nullptr,      nullptr,      ENDERS + 116, nullptr,
    ENDERS + 24,  ENDERS + 134, nullptr,      ENDERS + 30,  nullptr,
    nullptr,      ENDERS + 114, nullptr,      nullptr,      ENDERS + 131,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      ENDERS + 97,
    ENDERS + 49,  nullptr,      ENDERS + 145, nullptr,      nullptr,
    ENDERS + 85,  nullptr,      ENDERS + 95,  ENDERS + 47,  ENDERS + 21,
    ENDERS + 143, nullptr,      ENDERS + 23,  ENDERS + 83,  nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 133, nullptr,      nullptr,
    ENDERS + 157, nullptr,      ENDERS + 81,  nullptr,      ENDERS + 146,
    ENDERS + 68,  nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 166, ENDERS + 38,  nullptr,      ENDERS + 66,  ENDERS + 60,
    nullptr,      nullptr,      ENDERS + 28,  ENDERS + 2,   ENDERS + 100,
    ENDERS + 90,  ENDERS + 141, nullptr,      nullptr,      ENDERS + 153,
    nullptr,      ENDERS + 79,  nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 77,  nullptr,      nullptr,
    ENDERS + 11,  nullptr,      nullptr,      ENDERS + 63,  nullptr,
    ENDERS + 75,  ENDERS + 121, nullptr,      ENDERS + 9,   nullptr,
    ENDERS + 163, ENDERS + 1,   ENDERS + 55,  nullptr,      ENDERS + 36,
    nullptr,      nullptr,      ENDERS + 194, nullptr,      nullptr,
    ENDERS + 40,  ENDERS + 129, ENDERS + 34,  nullptr,      nullptr,
    ENDERS + 192, ENDERS + 191, nullptr,      ENDERS + 92,  ENDERS + 48,
    nullptr,      nullptr,      ENDERS + 96,  nullptr,      ENDERS + 84,
    ENDERS + 144, nullptr,      ENDERS + 46,  ENDERS + 20,  nullptr,
    ENDERS + 94,  ENDERS + 22,  ENDERS + 82,  ENDERS + 142, nullptr,
    ENDERS + 161, nullptr,      nullptr,      ENDERS + 89,  nullptr,
    nullptr,      ENDERS + 53,  nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      ENDERS + 27,
    nullptr,      ENDERS + 117, ENDERS + 33,  nullptr,      ENDERS + 135,
    nullptr,      nullptr,      ENDERS + 25,  nullptr,      ENDERS + 115,
    ENDERS + 31,  nullptr,
};
constexpr JeffPhrasingReverseHashMapEntry REVERSE_ENTRIES[] = {
    {0x00000000, 0x8707b2d8, StenoStroke(0x00000000), 0, 8, 7},
    {0x00000000, 0x710e300b, StenoStroke(0x000000a6), 0, 1, 13},
    {0x00000000, 0x710e300b, StenoStroke(0x000000fe), 0, 1, 13},
    {0x00000000, 0x8707b2d8, StenoStroke(0x00200000), 0, 8, 11},
    {0x02033737, 0x8707b2d8, StenoStroke(0x00194000), 0, 8, 7},
    {0x02984f45, 0x8707b2d8, StenoStroke(0x00040000), 1, 8, 7},
    {0x02984f45, 0x8707b2d8, StenoStroke(0x00240000), 0, 8, 11},
    {0x037d861e, 0x8707b2d8, StenoStroke(0x00124000), 1, 8, 7},
    {0x040092cb, 0x8707b2d8, StenoStroke(0x00098000), 0, 8, 7},
    {0x040092cb, 0x8707b2d8, StenoStroke(0x00298000), 0, 8, 11},
    {0x0505e520, 0x8707b2d8, StenoStroke(0x000a4000), 0, 8, 7},
    {0x0505e520, 0x8707b2d8, StenoStroke(0x002a4000), 0, 8, 11},
    {0x063120af, 0x0609009d, StenoStroke(0x00000400), 0, 2, 5},
    {0x0649e8c4, 0x8707b2d8, StenoStroke(0x000a4000), 0, 8, 7},
    {0x0649e8c4, 0x8707b2d8, StenoStroke(0x002a4000), 0, 8, 11},
    {0x06aa82f6, 0x8707b2d8, StenoStroke(0x00094000), 0, 8, 7},
    {0x06aa82f6, 0x8707b2d8, StenoStroke(0x00294000), 0, 8, 11},
    {0x06ee4070, 0x8707b2d8, StenoStroke(0x000f8000), 0, 8, 7},
    {0x06ee4070, 0x8707b2d8, StenoStroke(0x002f8000), 0, 8, 11},
    {0x070d8c31, 0x0609009d, StenoStroke(0x00000000), 1, 2, 5},
    {0x070d8c31, 0x8707b2d8, StenoStroke(0x0000c000), 0, 8, 7},
    {0x0743a13d, 0x8707b2d8, StenoStroke(0x00088000), 0, 8, 7},
    {0x07d4c6fa, 0x8707b2d8, StenoStroke(0x00540000), 1, 8, 11},
    {0x07f59b6d, 0x0609009d, StenoStroke(0x0000001a), 0, 2, 14},
    {0x0805fd75, 0x00000000, StenoStroke(0x00000000), 1, 0, 13},
    {0x081e2e34, 0x8707b2d8, StenoStroke(0x00408000), 0, 8, 7},
    {0x08f0effc, 0x8707b2d8, StenoStroke(0x00454000), 0, 8, 7},
    {0x08f0effc, 0x8707b2d8, StenoStroke(0x00654000), 0, 8, 11},
    {0x0966a6ff, 0x8707b2d8, StenoStroke(0x000e0000), 0, 8, 7},
    {0x0966a6ff, 0x8707b2d8, StenoStroke(0x002e0000), 0, 8, 11},
    {0x0a817e37, 0x8707b2d8, StenoStroke(0x0010c000), 1, 8, 7},
    {0x0a817e37, 0x8707b2d8, StenoStroke(0x0050c000), 0, 8, 11},
    {0x0c6f39ea, 0x0c6f39ea, StenoStroke(0x00002000), 1, 6, 5},
    {0x0c6f39ea, 0x0c6f39ea, StenoStroke(0x00002000), 0, 4, 7},
    {0x0c6f39ea, 0x0c6f39ea, StenoStroke(0x00002400), 0, 4, 5},
    {0x0c6f39ea, 0x8707b2d8, StenoStroke(0x00080000), 0, 8, 7},
    {0x0ce50045, 0x8707b2d8, StenoStroke(0x00140000), 1, 8, 7},
    {0x0ce50045, 0x8707b2d8, StenoStroke(0x00540000), 0, 8, 11},
    {0x0ceb84c6, 0x8707b2d8, StenoStroke(0x00460000), 0, 8, 7},
    {0x0ceb84c6, 0x8707b2d8, StenoStroke(0x00660000), 0, 8, 11},
    {0x0d2db8f0, 0x8707b2d8, StenoStroke(0x00014000), 0, 8, 7},
    {0x0d2db8f0, 0x8707b2d8, StenoStroke(0x00214000), 0, 8, 11},
    {0x0daeceeb, 0x8707b2d8, StenoStroke(0x00124000), 1, 8, 7},
    {0x0daeceeb, 0x8707b2d8, StenoStroke(0x00524000), 0, 8, 11},
    {0x0e15f2fe, 0x8707b2d8, StenoStroke(0x0001c000), 1, 8, 7},
    {0x0e15f2fe, 0x8707b2d8, StenoStroke(0x0021c000), 0, 8, 11},
    {0x0e3c55fc, 0x0e3c55fc, StenoStroke(0x00000800), 1, 4, 5},
    {0x0e3c55fc, 0x0e3c55fc, StenoStroke(0x00000800), 0, 6, 5},
    {0x0e3c55fc, 0x0e3c55fc, StenoStroke(0x00000c00), 0, 4, 5},
    {0x0e3c55fc, 0x8707b2d8, StenoStroke(0x00010000), 0, 8, 7},
    {0x0eb86064, 0x8707b2d8, StenoStroke(0x0019c000), 0, 8, 7},
    {0x0eb86064, 0x8707b2d8, StenoStroke(0x0079c000), 0, 8, 11},
    {0x0ec73fd6, 0x8707b2d8, StenoStroke(0x00190000), 0, 8, 7},
    {0x0ec73fd6, 0x8707b2d8, StenoStroke(0x00790000), 0, 8, 11},
    {0x0ee254bf, 0x8707b2d8, StenoStroke(0x00120000), 0, 8, 7},
    {0x0ee254bf, 0x8707b2d8, StenoStroke(0x00520000), 0, 8, 11},
    {0x0ef063bd, 0x8707b2d8, StenoStroke(0x00100000), 0, 8, 7},
    {0x0ef063bd, 0x8707b2d8, StenoStroke(0x00500000), 0, 8, 11},
    {0x0f3d9d9d, 0x8707b2d8, StenoStroke(0x00414000), 0, 8, 7},
    {0x0f3d9d9d, 0x8707b2d8, StenoStroke(0x00614000), 0, 8, 11},
    {0x102de0ab, 0x8707b2d8, StenoStroke(0x0000c000), 1, 8, 7},
    {0x102de0ab, 0x8707b2d8, StenoStroke(0x0020c000), 0, 8, 11},
    {0x104f239f, 0x8707b2d8, StenoStroke(0x000c4000), 0, 8, 7},
    {0x104f239f, 0x8707b2d8, StenoStroke(0x002c4000), 0, 8, 11},
    {0x10889d0f, 0x8707b2d8, StenoStroke(0x000f8000), 0, 8, 7},
    {0x10889d0f, 0x8707b2d8, StenoStroke(0x002f8000), 0, 8, 11},
    {0x11eb3640, 0x8707b2d8, StenoStroke(0x00440000), 0, 8, 7},
    {0x11eb3640, 0x8707b2d8, StenoStroke(0x00640000), 0, 8, 11},
    {0x12cfe2ee, 0x8707b2d8, StenoStroke(0x0008c000), 0, 8, 7},
    {0x12cfe2ee, 0x8707b2d8, StenoStroke(0x0028c000), 0, 8, 11},
    {0x14d30cbf, 0x8707b2d8, StenoStroke(0x0012c000), 0, 8, 7},
    {0x14d30cbf, 0x8707b2d8, StenoStroke(0x0052c000), 0, 8, 11},
    {0x15b823df, 0x8707b2d8, StenoStroke(0x00040000), 1, 8, 7},
    {0x17b46a1f, 0x8707b2d8, StenoStroke(0x000b8000), 0, 8, 7},
    {0x17b46a1f, 0x8707b2d8, StenoStroke(0x002b8000), 0, 8, 11},
    {0x1830eda6, 0x8707b2d8, StenoStroke(0x000f0000), 0, 8, 7},
    {0x19a74354, 0x8707b2d8, StenoStroke(0x00158000), 0, 8, 7},
    {0x19a74354, 0x8707b2d8, StenoStroke(0x00558000), 0, 8, 11},
    {0x19c072db, 0x8707b2d8, StenoStroke(0x00018000), 1, 8, 7},
    {0x1a25a210, 0x1a25a210, StenoStroke(0x00003800), 0, 6, 13},
    {0x1a25a210, 0x1a25a210, StenoStroke(0x00003800), 0, 4, 13},
    {0x1a473416, 0x8707b2d8, StenoStroke(0x00184000), 0, 8, 7},
    {0x1a473416, 0x8707b2d8, StenoStroke(0x00784000), 0, 8, 11},
    {0x1a834993, 0x8707b2d8, StenoStroke(0x00024000), 0, 8, 7},
    {0x1a834993, 0x8707b2d8, StenoStroke(0x00224000), 0, 8, 11},
    {0x1acc766e, 0x8707b2d8, StenoStroke(0x0003c000), 1, 8, 7},
    {0x1acc766e, 0x8707b2d8, StenoStroke(0x0023c000), 0, 8, 11},
    {0x1aeec2c5, 0x8707b2d8, StenoStroke(0x000dc000), 0, 8, 7},
    {0x1aeec2c5, 0x8707b2d8, StenoStroke(0x002dc000), 0, 8, 11},
    {0x1b15dda7, 0x1b15dda7, StenoStroke(0x00000800), 1, 4, 9},
    {0x1b15dda7, 0x1b15dda7, StenoStroke(0x00000800), 0, 6, 9},
    {0x1b15dda7, 0x1b15dda7, StenoStroke(0x00000800), 0, 4, 9},
    {0x1b15dda7, 0x1b15dda7, StenoStroke(0x00000c00), 0, 4, 9},
    {0x1b15dda7, 0x8707b2d8, StenoStroke(0x00210000), 0, 8, 11},
    {0x1b479518, 0x8707b2d8, StenoStroke(0x00088000), 0, 8, 7},
    {0x1b479518, 0x8707b2d8, StenoStroke(0x00288000), 0, 8, 11},
    {0x1b501b4e, 0x8707b2d8, StenoStroke(0x000f0000), 0, 8, 7},
    {0x1b501b4e, 0x8707b2d8, StenoStroke(0x002f0000), 0, 8, 11},
    {0x1b71b162, 0x8707b2d8, StenoStroke(0x00500000), 0, 8, 11},
    {0x1c7f6819, 0x710e300b, StenoStroke(0x000000a4), 0, 1, 13},
    {0x1c7f6819, 0x710e300b, StenoStroke(0x00001c00), 0, 1, 14},
    {0x1caaa576, 0x8707b2d8, StenoStroke(0x00078000), 1, 8, 7},
    {0x1cc2b052, 0x8707b2d8, StenoStroke(0x00148000), 1, 8, 7},
    {0x1cc2b052, 0x8707b2d8, StenoStroke(0x00548000), 0, 8, 11},
    {0x1d82eb26, 0x8707b2d8, StenoStroke(0x0005c000), 1, 8, 7},
    {0x1d82eb26, 0x8707b2d8, StenoStroke(0x0025c000), 0, 8, 11},
    {0x1d8f9346, 0x0609009d, StenoStroke(0x000000e2), 0, 2, 14},
    {0x1d90eeb5, 0x8707b2d8, StenoStroke(0x0001c000), 1, 8, 7},
    {0x1d90eeb5, 0x8707b2d8, StenoStroke(0x0021c000), 0, 8, 11},
    {0x1dd9069f, 0x8707b2d8, StenoStroke(0x000d0000), 0, 8, 7},
    {0x1dd9069f, 0x8707b2d8, StenoStroke(0x002d0000), 0, 8, 11},
    {0x1dd969bc, 0x8707b2d8, StenoStroke(0x00404000), 0, 8, 7},
    {0x1de6be4e, 0x0609009d, StenoStroke(0x00000300), 0, 2, 5},
    {0x1de6be4e, 0x8707b2d8, StenoStroke(0x00154000), 0, 8, 7},
    {0x1e151309, 0x8707b2d8, StenoStroke(0x00084000), 1, 8, 7},
    {0x1e151309, 0x8707b2d8, StenoStroke(0x00284000), 0, 8, 11},
    {0x1eafe305, 0x8707b2d8, StenoStroke(0x00138000), 0, 8, 7},
    {0x1eafe305, 0x8707b2d8, StenoStroke(0x00538000), 0, 8, 11},
    {0x1f3c70ad, 0x8707b2d8, StenoStroke(0x00408000), 0, 8, 7},
    {0x1f3c70ad, 0x8707b2d8, StenoStroke(0x00608000), 0, 8, 11},
    {0x1f522e50, 0x8707b2d8, StenoStroke(0x00030000), 1, 8, 7},
    {0x1f522e50, 0x8707b2d8, StenoStroke(0x00230000), 0, 8, 11},
    {0x1fe9e3df, 0x8707b2d8, StenoStroke(0x000d4000), 0, 8, 7},
    {0x2020b83d, 0x8707b2d8, StenoStroke(0x00458000), 0, 8, 7},
    {0x20663dce, 0x8707b2d8, StenoStroke(0x000c0000), 0, 8, 7},
    {0x20663dce, 0x8707b2d8, StenoStroke(0x002c0000), 0, 8, 11},
    {0x206859dc, 0x8707b2d8, StenoStroke(0x0019c000), 0, 8, 7},
    {0x206859dc, 0x8707b2d8, StenoStroke(0x0079c000), 0, 8, 11},
    {0x212afb55, 0x8707b2d8, StenoStroke(0x001a4000), 0, 8, 7},
    {0x212afb55, 0x8707b2d8, StenoStroke(0x007a4000), 0, 8, 11},
    {0x2171cdfd, 0x0609009d, StenoStroke(0x00000700), 0, 2, 5},
    {0x21f3e377, 0x8707b2d8, StenoStroke(0x000ac000), 0, 8, 7},
    {0x21f3e377, 0x8707b2d8, StenoStroke(0x002ac000), 0, 8, 11},
    {0x2273d1a5, 0x8707b2d8, StenoStroke(0x0003c000), 1, 8, 7},
    {0x2273d1a5, 0x8707b2d8, StenoStroke(0x0023c000), 0, 8, 11},
    {0x22bfa302, 0x8707b2d8, StenoStroke(0x0005c000), 1, 8, 7},
    {0x22bfa302, 0x8707b2d8, StenoStroke(0x0025c000), 0, 8, 11},
    {0x23e80e1c, 0x0609009d, StenoStroke(0x00000062), 0, 2, 14},
    {0x23eb5694, 0x8707b2d8, StenoStroke(0x00128000), 1, 8, 7},
    {0x249319ef, 0x8707b2d8, StenoStroke(0x001f8000), 0, 8, 7},
    {0x262a9340, 0x8707b2d8, StenoStroke(0x00110000), 1, 8, 7},
    {0x266fa23b, 0x8707b2d8, StenoStroke(0x00414000), 0, 8, 7},
    {0x266fa23b, 0x8707b2d8, StenoStroke(0x00614000), 0, 8, 11},
    {0x26aefbc0, 0x8707b2d8, StenoStroke(0x00180000), 0, 8, 7},
    {0x27684180, 0x8707b2d8, StenoStroke(0x00190000), 0, 8, 7},
    {0x27684180, 0x8707b2d8, StenoStroke(0x00790000), 0, 8, 11},
    {0x27d367ef, 0x8707b2d8, StenoStroke(0x002d0000), 0, 8, 11},
    {0x27de2e44, 0x0609009d, StenoStroke(0x00000200), 0, 2, 9},
    {0x27de2e44, 0x8707b2d8, StenoStroke(0x00234000), 0, 8, 11},
    {0x28218e08, 0x8707b2d8, StenoStroke(0x0025c000), 1, 8, 11},
    {0x28ec173f, 0x8707b2d8, StenoStroke(0x000d8000), 0, 8, 7},
    {0x28ec173f, 0x8707b2d8, StenoStroke(0x002d8000), 0, 8, 11},
    {0x29ee350f, 0x8707b2d8, StenoStroke(0x0012c000), 0, 8, 7},
    {0x2ae44f4f, 0x8707b2d8, StenoStroke(0x00440000), 0, 8, 7},
    {0x2b3ad402, 0x8707b2d8, StenoStroke(0x00020000), 0, 8, 7},
    {0x2b3ad402, 0x8707b2d8, StenoStroke(0x00220000), 0, 8, 11},
    {0x2bca8e0d, 0x2bca8e0d, StenoStroke(0x00000800), 1, 4, 13},
    {0x2bca8e0d, 0x2bca8e0d, StenoStroke(0x00000c00), 0, 4, 13},
    {0x2bca8e0d, 0x8707b2d8, StenoStroke(0x00010000), 0, 8, 7},
    {0x2bca8e0d, 0x8707b2d8, StenoStroke(0x00210000), 0, 8, 11},
    {0x2c316b9d, 0x8707b2d8, StenoStroke(0x002dc000), 0, 8, 11},
    {0x2d5aebbc, 0x0609009d, StenoStroke(0x00000600), 0, 2, 9},
    {0x2d9bcc99, 0x8707b2d8, StenoStroke(0x000a0000), 1, 8, 7},
    {0x2d9bcc99, 0x8707b2d8, StenoStroke(0x002a0000), 0, 8, 11},
    {0x2e086b3a, 0x8707b2d8, StenoStroke(0x000ac000), 0, 8, 7},
    {0x2e58da19, 0x8707b2d8, StenoStroke(0x001a8000), 0, 8, 7},
    {0x2e58da19, 0x8707b2d8, StenoStroke(0x007a8000), 0, 8, 11},
    {0x2f21896f, 0x8707b2d8, StenoStroke(0x00004000), 0, 8, 7},
    {0x2f21896f, 0x8707b2d8, StenoStroke(0x00204000), 0, 8, 11},
    {0x320ed901, 0x8707b2d8, StenoStroke(0x00414000), 0, 8, 7},
    {0x326428a8, 0x326428a8, StenoStroke(0x00000c00), 0, 6, 5},
    {0x32af1522, 0x8707b2d8, StenoStroke(0x00008000), 1, 8, 7},
    {0x32af1522, 0x8707b2d8, StenoStroke(0x00208000), 0, 8, 11},
    {0x32af1522, 0x8707b2d8, StenoStroke(0x00614000), 0, 8, 11},
    {0x32e49586, 0x8707b2d8, StenoStroke(0x00180000), 0, 8, 7},
    {0x32e49586, 0x8707b2d8, StenoStroke(0x00780000), 0, 8, 11},
    {0x3568a8c0, 0x8707b2d8, StenoStroke(0x00404000), 0, 8, 7},
    {0x3568a8c0, 0x8707b2d8, StenoStroke(0x00604000), 0, 8, 11},
    {0x35a6bdfe, 0x8707b2d8, StenoStroke(0x00240000), 1, 8, 11},
    {0x37dd6e7b, 0x8707b2d8, StenoStroke(0x00094000), 0, 8, 7},
    {0x37dd6e7b, 0x8707b2d8, StenoStroke(0x00294000), 0, 8, 11},
    {0x386f360b, 0x8707b2d8, StenoStroke(0x0008c000), 0, 8, 7},
    {0x386f360b, 0x8707b2d8, StenoStroke(0x0028c000), 0, 8, 11},
    {0x3896e72c, 0x8707b2d8, StenoStroke(0x0012c000), 0, 8, 7},
    {0x3896e72c, 0x8707b2d8, StenoStroke(0x0052c000), 0, 8, 11},
    {0x390a7b63, 0x710e300b, StenoStroke(0x000000ea), 0, 1, 13},
    {0x390a7b63, 0x710e300b, StenoStroke(0x00000c00), 0, 1, 14},
    {0x3a35c33d, 0x0609009d, StenoStroke(0x00000500), 0, 2, 9},
    {0x3a528d82, 0x8707b2d8, StenoStroke(0x002a8000), 0, 8, 11},
    {0x3cb73648, 0x8707b2d8, StenoStroke(0x000ac000), 0, 8, 7},
    {0x3cb73648, 0x8707b2d8, StenoStroke(0x002ac000), 0, 8, 11},
    {0x3cca9f20, 0x8707b2d8, StenoStroke(0x001a0000), 0, 8, 7},
    {0x3ceae297, 0x3ceae297, StenoStroke(0x00000800), 1, 4, 5},
    {0x3ceae297, 0x3ceae297, StenoStroke(0x00000800), 0, 6, 5},
    {0x3ceae297, 0x3ceae297, StenoStroke(0x00000c00), 0, 4, 5},
    {0x3ceae297, 0x8707b2d8, StenoStroke(0x00010000), 0, 8, 7},
    {0x3d5b5ba2, 0x8707b2d8, StenoStroke(0x00128000), 1, 8, 7},
    {0x3d5b5ba2, 0x8707b2d8, StenoStroke(0x00528000), 0, 8, 11},
    {0x3f30e086, 0x8707b2d8, StenoStroke(0x00428000), 0, 8, 7},
    {0x3f30e086, 0x8707b2d8, StenoStroke(0x00628000), 0, 8, 11},
    {0x3f31f002, 0x8707b2d8, StenoStroke(0x00038000), 1, 8, 7},
    {0x3f31f002, 0x8707b2d8, StenoStroke(0x00238000), 0, 8, 11},
    {0x3f3807c6, 0x0609009d, StenoStroke(0x00000300), 0, 2, 9},
    {0x3f3807c6, 0x8707b2d8, StenoStroke(0x00554000), 0, 8, 11},
    {0x4057fe20, 0x8707b2d8, StenoStroke(0x00458000), 0, 8, 7},
    {0x4057fe20, 0x8707b2d8, StenoStroke(0x00658000), 0, 8, 11},
    {0x410505e2, 0x8707b2d8, StenoStroke(0x0001c000), 1, 8, 7},
    {0x426bbd50, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x4286c9ef, 0x8707b2d8, StenoStroke(0x0009c000), 0, 8, 7},
    {0x4286c9ef, 0x8707b2d8, StenoStroke(0x0029c000), 0, 8, 11},
    {0x429b1a21, 0x0609009d, StenoStroke(0x00000200), 1, 2, 5},
    {0x429b1a21, 0x8707b2d8, StenoStroke(0x00034000), 0, 8, 7},
    {0x44630acd, 0x8707b2d8, StenoStroke(0x0009c000), 0, 8, 7},
    {0x44ac63b8, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x44ac63b8, 0x8707b2d8, StenoStroke(0x00290000), 0, 8, 11},
    {0x44be45f9, 0x44be45f9, StenoStroke(0x00000c00), 0, 6, 5},
    {0x453f0832, 0x8707b2d8, StenoStroke(0x00428000), 0, 8, 7},
    {0x456239f9, 0x8707b2d8, StenoStroke(0x00068000), 1, 8, 7},
    {0x456239f9, 0x8707b2d8, StenoStroke(0x00268000), 0, 8, 11},
    {0x46224440, 0x8707b2d8, StenoStroke(0x00454000), 0, 8, 7},
    {0x46224440, 0x8707b2d8, StenoStroke(0x00654000), 0, 8, 11},
    {0x4686967c, 0x8707b2d8, StenoStroke(0x00010000), 1, 8, 7},
    {0x4686967c, 0x8707b2d8, StenoStroke(0x00210000), 0, 8, 11},
    {0x47599c71, 0x47599c71, StenoStroke(0x00002400), 1, 6, 5},
    {0x48746d47, 0x8707b2d8, StenoStroke(0x00030000), 1, 8, 7},
    {0x49033525, 0x8707b2d8, StenoStroke(0x000c4000), 0, 8, 7},
    {0x49033525, 0x8707b2d8, StenoStroke(0x002c4000), 0, 8, 11},
    {0x49ca4e7d, 0x8707b2d8, StenoStroke(0x00070000), 1, 8, 7},
    {0x49e22396, 0x8707b2d8, StenoStroke(0x00440000), 0, 8, 7},
    {0x49e22396, 0x8707b2d8, StenoStroke(0x00640000), 0, 8, 11},
    {0x4a962ee9, 0x8707b2d8, StenoStroke(0x0010c000), 1, 8, 7},
    {0x4a962ee9, 0x8707b2d8, StenoStroke(0x0050c000), 0, 8, 11},
    {0x4b300576, 0x8707b2d8, StenoStroke(0x00024000), 0, 8, 7},
    {0x4b300576, 0x8707b2d8, StenoStroke(0x00224000), 0, 8, 11},
    {0x4bf68519, 0x8707b2d8, StenoStroke(0x00140000), 1, 8, 7},
    {0x4cf73ae2, 0x8707b2d8, StenoStroke(0x00010000), 1, 8, 7},
    {0x4cf73ae2, 0x8707b2d8, StenoStroke(0x00210000), 0, 8, 11},
    {0x4d8049c1, 0x4d8049c1, StenoStroke(0x00002c00), 0, 6, 9},
    {0x4d9a0d84, 0x8707b2d8, StenoStroke(0x00054000), 1, 8, 7},
    {0x4d9a0d84, 0x8707b2d8, StenoStroke(0x00254000), 0, 8, 11},
    {0x4daeda87, 0x8707b2d8, StenoStroke(0x00244000), 1, 8, 11},
    {0x4de06265, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x4e7b11cb, 0x8707b2d8, StenoStroke(0x00038000), 1, 8, 7},
    {0x4e7b11cb, 0x8707b2d8, StenoStroke(0x00238000), 0, 8, 11},
    {0x4e7fcf41, 0x0609009d, StenoStroke(0x00000000), 1, 2, 9},
    {0x4e7fcf41, 0x8707b2d8, StenoStroke(0x0020c000), 0, 8, 11},
    {0x4f91d53c, 0x8707b2d8, StenoStroke(0x00014000), 0, 8, 7},
    {0x4f91d53c, 0x8707b2d8, StenoStroke(0x00214000), 0, 8, 11},
    {0x5060b8c4, 0x8707b2d8, StenoStroke(0x00148000), 1, 8, 7},
    {0x5060b8c4, 0x8707b2d8, StenoStroke(0x00548000), 0, 8, 11},
    {0x5076a4c0, 0x8707b2d8, StenoStroke(0x00004000), 0, 8, 7},
    {0x5076a4c0, 0x8707b2d8, StenoStroke(0x00204000), 0, 8, 11},
    {0x508edb76, 0x8707b2d8, StenoStroke(0x00078000), 1, 8, 7},
    {0x508edb76, 0x8707b2d8, StenoStroke(0x00278000), 0, 8, 11},
    {0x508ffc5a, 0x8707b2d8, StenoStroke(0x001c0000), 0, 8, 7},
    {0x5137067c, 0x0609009d, StenoStroke(0x00000116), 0, 2, 14},
    {0x51469c33, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x51469c33, 0x8707b2d8, StenoStroke(0x00290000), 0, 8, 11},
    {0x5216322e, 0x8707b2d8, StenoStroke(0x000d4000), 0, 8, 7},
    {0x5216322e, 0x8707b2d8, StenoStroke(0x002d4000), 0, 8, 11},
    {0x52b1c121, 0x8707b2d8, StenoStroke(0x00184000), 0, 8, 7},
    {0x52b1c121, 0x8707b2d8, StenoStroke(0x00784000), 0, 8, 11},
    {0x52fa60a8, 0x8707b2d8, StenoStroke(0x00058000), 1, 8, 7},
    {0x52fa60a8, 0x8707b2d8, StenoStroke(0x00258000), 0, 8, 11},
    {0x530f2caf, 0x8707b2d8, StenoStroke(0x00420000), 0, 8, 7},
    {0x530f2caf, 0x8707b2d8, StenoStroke(0x00620000), 0, 8, 11},
    {0x5313f237, 0x5313f237, StenoStroke(0x00002400), 1, 6, 5},
    {0x532743cd, 0x8707b2d8, StenoStroke(0x00048000), 1, 8, 7},
    {0x532743cd, 0x8707b2d8, StenoStroke(0x00248000), 0, 8, 11},
    {0x534e6880, 0x8707b2d8, StenoStroke(0x00054000), 1, 8, 7},
    {0x534e6880, 0x8707b2d8, StenoStroke(0x00254000), 0, 8, 11},
    {0x5468790f, 0x8707b2d8, StenoStroke(0x000b0000), 0, 8, 7},
    {0x5468790f, 0x8707b2d8, StenoStroke(0x002b0000), 0, 8, 11},
    {0x54ce8b8c, 0x0609009d, StenoStroke(0x00000500), 0, 2, 5},
    {0x5559689d, 0x8707b2d8, StenoStroke(0x000cc000), 0, 8, 7},
    {0x5559689d, 0x8707b2d8, StenoStroke(0x002cc000), 0, 8, 11},
    {0x5647a3be, 0x5647a3be, StenoStroke(0x00001800), 0, 6, 13},
    {0x5647a3be, 0x5647a3be, StenoStroke(0x00001800), 0, 4, 13},
    {0x5647a3be, 0x5647a3be, StenoStroke(0x00001c00), 0, 4, 13},
    {0x567a7547, 0x8707b2d8, StenoStroke(0x00228000), 1, 8, 11},
    {0x5782501d, 0x8707b2d8, StenoStroke(0x00060000), 1, 8, 7},
    {0x5782501d, 0x8707b2d8, StenoStroke(0x00260000), 0, 8, 11},
    {0x578bb7bf, 0x8707b2d8, StenoStroke(0x000e8000), 0, 8, 7},
    {0x578bb7bf, 0x8707b2d8, StenoStroke(0x002e8000), 0, 8, 11},
    {0x57f87174, 0x8707b2d8, StenoStroke(0x00138000), 0, 8, 7},
    {0x57f87174, 0x8707b2d8, StenoStroke(0x00538000), 0, 8, 11},
    {0x5823daa0, 0x8707b2d8, StenoStroke(0x000f8000), 0, 8, 7},
    {0x5823daa0, 0x8707b2d8, StenoStroke(0x002f8000), 0, 8, 11},
    {0x58a6ad82, 0x8707b2d8, StenoStroke(0x000b8000), 0, 8, 7},
    {0x58a6ad82, 0x8707b2d8, StenoStroke(0x002b8000), 0, 8, 11},
    {0x59c13249, 0x8707b2d8, StenoStroke(0x00218000), 1, 8, 11},
    {0x5a2fe996, 0x710e300b, StenoStroke(0x000000c6), 0, 1, 13},
    {0x5a2fe996, 0x710e300b, StenoStroke(0x000000d6), 0, 1, 13},
    {0x5b754b18, 0x8707b2d8, StenoStroke(0x0018c000), 0, 8, 7},
    {0x5b754b18, 0x8707b2d8, StenoStroke(0x0078c000), 0, 8, 11},
    {0x5d347e5e, 0x8707b2d8, StenoStroke(0x00420000), 0, 8, 7},
    {0x5d9a8b18, 0x8707b2d8, StenoStroke(0x001a0000), 0, 8, 7},
    {0x5d9a8b18, 0x8707b2d8, StenoStroke(0x007a0000), 0, 8, 11},
    {0x5edc26af, 0x8707b2d8, StenoStroke(0x00114000), 1, 8, 7},
    {0x5edc26af, 0x8707b2d8, StenoStroke(0x00514000), 0, 8, 11},
    {0x5ee563df, 0x8707b2d8, StenoStroke(0x0019c000), 0, 8, 7},
    {0x605afb1e, 0x8707b2d8, StenoStroke(0x00088000), 0, 8, 7},
    {0x605afb1e, 0x8707b2d8, StenoStroke(0x00288000), 0, 8, 11},
    {0x6113a845, 0x8707b2d8, StenoStroke(0x00404000), 0, 8, 7},
    {0x6113a845, 0x8707b2d8, StenoStroke(0x00604000), 0, 8, 11},
    {0x6156e822, 0x8707b2d8, StenoStroke(0x000d0000), 0, 8, 7},
    {0x6156e822, 0x8707b2d8, StenoStroke(0x002d0000), 0, 8, 11},
    {0x62bfcab3, 0x8707b2d8, StenoStroke(0x000a0000), 1, 8, 7},
    {0x62bfcab3, 0x8707b2d8, StenoStroke(0x002a0000), 0, 8, 11},
    {0x633cbad2, 0x0609009d, StenoStroke(0x00000100), 0, 2, 5},
    {0x633cbad2, 0x8707b2d8, StenoStroke(0x00150000), 0, 8, 7},
    {0x64e2a26f, 0x8707b2d8, StenoStroke(0x0001c000), 1, 8, 7},
    {0x64e2a26f, 0x8707b2d8, StenoStroke(0x0021c000), 0, 8, 11},
    {0x6508f284, 0x8707b2d8, StenoStroke(0x00044000), 1, 8, 7},
    {0x6545e48f, 0x8707b2d8, StenoStroke(0x00454000), 0, 8, 7},
    {0x6545e48f, 0x8707b2d8, StenoStroke(0x00654000), 0, 8, 11},
    {0x66002635, 0x66002635, StenoStroke(0x00002800), 0, 4, 13},
    {0x66002635, 0x66002635, StenoStroke(0x00002800), 0, 6, 5},
    {0x66002635, 0x66002635, StenoStroke(0x00002c00), 0, 4, 13},
    {0x666b7816, 0x710e300b, StenoStroke(0x000000b8), 0, 1, 13},
    {0x666b7816, 0x710e300b, StenoStroke(0x00001000), 0, 1, 14},
    {0x666c145e, 0x8707b2d8, StenoStroke(0x00454000), 0, 8, 7},
    {0x680e9c5c, 0x8707b2d8, StenoStroke(0x0011c000), 1, 8, 7},
    {0x680e9c5c, 0x8707b2d8, StenoStroke(0x0051c000), 0, 8, 11},
    {0x6826eae0, 0x8707b2d8, StenoStroke(0x00014000), 0, 8, 7},
    {0x6826eae0, 0x8707b2d8, StenoStroke(0x00214000), 0, 8, 11},
    {0x6a2996a3, 0x6a2996a3, StenoStroke(0x00002800), 0, 4, 5},
    {0x6a2996a3, 0x6a2996a3, StenoStroke(0x00002800), 0, 6, 5},
    {0x6a2996a3, 0x6a2996a3, StenoStroke(0x00002c00), 0, 4, 5},
    {0x6a59beee, 0x8707b2d8, StenoStroke(0x0004c000), 1, 8, 7},
    {0x6b660a33, 0x8707b2d8, StenoStroke(0x00054000), 1, 8, 7},
    {0x6b660a33, 0x8707b2d8, StenoStroke(0x00254000), 0, 8, 11},
    {0x6b7a9467, 0x8707b2d8, StenoStroke(0x00138000), 0, 8, 7},
    {0x6c3bf144, 0x8707b2d8, StenoStroke(0x00414000), 0, 8, 7},
    {0x6e357c4a, 0x8707b2d8, StenoStroke(0x00088000), 0, 8, 7},
    {0x6e357c4a, 0x8707b2d8, StenoStroke(0x00288000), 0, 8, 11},
    {0x6e786c89, 0x8707b2d8, StenoStroke(0x00148000), 1, 8, 7},
    {0x6e786c89, 0x8707b2d8, StenoStroke(0x00548000), 0, 8, 11},
    {0x70718ee4, 0x8707b2d8, StenoStroke(0x001a8000), 0, 8, 7},
    {0x70f8efb0, 0x8707b2d8, StenoStroke(0x00038000), 1, 8, 7},
    {0x70f8efb0, 0x8707b2d8, StenoStroke(0x00238000), 0, 8, 11},
    {0x73044310, 0x8707b2d8, StenoStroke(0x00128000), 1, 8, 7},
    {0x73044310, 0x8707b2d8, StenoStroke(0x00528000), 0, 8, 11},
    {0x740e219e, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x740e219e, 0x8707b2d8, StenoStroke(0x00290000), 0, 8, 11},
    {0x7499292d, 0x8707b2d8, StenoStroke(0x000d0000), 0, 8, 7},
    {0x749ffdd0, 0x749ffdd0, StenoStroke(0x00003000), 0, 4, 13},
    {0x749ffdd0, 0x749ffdd0, StenoStroke(0x00003000), 0, 6, 13},
    {0x749ffdd0, 0x749ffdd0, StenoStroke(0x00003400), 0, 4, 13},
    {0x749ffdd0, 0x8707b2d8, StenoStroke(0x00578000), 0, 8, 7},
    {0x749ffdd0, 0x8707b2d8, StenoStroke(0x007f8000), 0, 8, 11},
    {0x74faa480, 0x74faa480, StenoStroke(0x00002800), 0, 4, 9},
    {0x74faa480, 0x74faa480, StenoStroke(0x00002800), 0, 6, 9},
    {0x74faa480, 0x74faa480, StenoStroke(0x00002c00), 0, 4, 9},
    {0x756ac9d5, 0x0609009d, StenoStroke(0x00000600), 0, 2, 5},
    {0x75abac61, 0x8707b2d8, StenoStroke(0x00194000), 0, 8, 7},
    {0x75abac61, 0x8707b2d8, StenoStroke(0x00794000), 0, 8, 11},
    {0x77c63782, 0x8707b2d8, StenoStroke(0x00048000), 1, 8, 7},
    {0x77c63782, 0x8707b2d8, StenoStroke(0x00248000), 0, 8, 11},
    {0x785f2182, 0x8707b2d8, StenoStroke(0x00060000), 1, 8, 7},
    {0x789b5ca3, 0x8707b2d8, StenoStroke(0x0002c000), 1, 8, 7},
    {0x79476318, 0x8707b2d8, StenoStroke(0x00400000), 1, 8, 7},
    {0x79476318, 0x8707b2d8, StenoStroke(0x00600000), 0, 8, 11},
    {0x7978904a, 0x8707b2d8, StenoStroke(0x0019c000), 0, 8, 7},
    {0x7978904a, 0x8707b2d8, StenoStroke(0x0079c000), 0, 8, 11},
    {0x79bbdc48, 0x8707b2d8, StenoStroke(0x0011c000), 1, 8, 7},
    {0x7a218af1, 0x8707b2d8, StenoStroke(0x00298000), 0, 8, 11},
    {0x7a67e768, 0x8707b2d8, StenoStroke(0x00460000), 0, 8, 7},
    {0x7a67e768, 0x8707b2d8, StenoStroke(0x00660000), 0, 8, 11},
    {0x7ac61e02, 0x8707b2d8, StenoStroke(0x00018000), 1, 8, 7},
    {0x7ac61e02, 0x8707b2d8, StenoStroke(0x00218000), 0, 8, 11},
    {0x7b2cc683, 0x8707b2d8, StenoStroke(0x00420000), 0, 8, 7},
    {0x7b2cc683, 0x8707b2d8, StenoStroke(0x00620000), 0, 8, 11},
    {0x7b6c0fed, 0x8707b2d8, StenoStroke(0x001c0000), 0, 8, 7},
    {0x7b6c0fed, 0x8707b2d8, StenoStroke(0x007c0000), 0, 8, 11},
    {0x7c796918, 0x8707b2d8, StenoStroke(0x00408000), 0, 8, 7},
    {0x7c796918, 0x8707b2d8, StenoStroke(0x00608000), 0, 8, 11},
    {0x7d5bd382, 0x8707b2d8, StenoStroke(0x00048000), 1, 8, 7},
    {0x7d5bd382, 0x8707b2d8, StenoStroke(0x00248000), 0, 8, 11},
    {0x7dbcdfb9, 0x8707b2d8, StenoStroke(0x00068000), 1, 8, 7},
    {0x7ed87395, 0x8707b2d8, StenoStroke(0x00204000), 0, 8, 11},
    {0x7f11e740, 0x8707b2d8, StenoStroke(0x00058000), 1, 8, 7},
    {0x7f11e740, 0x8707b2d8, StenoStroke(0x00258000), 0, 8, 11},
    {0x803a7b1f, 0x8707b2d8, StenoStroke(0x00004000), 0, 8, 7},
    {0x80ae03d9, 0x8707b2d8, StenoStroke(0x00124000), 1, 8, 7},
    {0x80ae03d9, 0x8707b2d8, StenoStroke(0x00524000), 0, 8, 11},
    {0x80aebb80, 0x8707b2d8, StenoStroke(0x000e8000), 0, 8, 7},
    {0x80aebb80, 0x8707b2d8, StenoStroke(0x002e8000), 0, 8, 11},
    {0x80d6d70e, 0x8707b2d8, StenoStroke(0x002c0000), 0, 8, 11},
    {0x81063908, 0x8707b2d8, StenoStroke(0x001a4000), 0, 8, 7},
    {0x81063908, 0x8707b2d8, StenoStroke(0x007a4000), 0, 8, 11},
    {0x82225168, 0x8707b2d8, StenoStroke(0x00050000), 1, 8, 7},
    {0x82225168, 0x8707b2d8, StenoStroke(0x00250000), 0, 8, 11},
    {0x840eabae, 0x8707b2d8, StenoStroke(0x00440000), 0, 8, 7},
    {0x840eabae, 0x8707b2d8, StenoStroke(0x00640000), 0, 8, 11},
    {0x8429f971, 0x8707b2d8, StenoStroke(0x00188000), 0, 8, 7},
    {0x843cba45, 0x8707b2d8, StenoStroke(0x00064000), 0, 8, 7},
    {0x843cba45, 0x8707b2d8, StenoStroke(0x00264000), 0, 8, 11},
    {0x85216679, 0x85216679, StenoStroke(0x00002c00), 0, 6, 5},
    {0x8701804a, 0x8707b2d8, StenoStroke(0x000a4000), 0, 8, 7},
    {0x87ae0156, 0x8707b2d8, StenoStroke(0x0018c000), 0, 8, 7},
    {0x87ae0156, 0x8707b2d8, StenoStroke(0x0078c000), 0, 8, 11},
    {0x8952fbf0, 0x8707b2d8, StenoStroke(0x0010c000), 1, 8, 7},
    {0x8a1dfa16, 0x8707b2d8, StenoStroke(0x00640000), 0, 8, 11},
    {0x8a97f357, 0x8a97f357, StenoStroke(0x00002400), 1, 6, 9},
    {0x8b4ba9e6, 0x8707b2d8, StenoStroke(0x000c8000), 0, 8, 7},
    {0x8b4ba9e6, 0x8707b2d8, StenoStroke(0x002c8000), 0, 8, 11},
    {0x8c17e6cb, 0x8707b2d8, StenoStroke(0x00158000), 0, 8, 7},
    {0x8ca4052e, 0x8707b2d8, StenoStroke(0x00100000), 0, 8, 7},
    {0x8ca4052e, 0x8707b2d8, StenoStroke(0x00500000), 0, 8, 11},
    {0x8eba60cc, 0x8707b2d8, StenoStroke(0x0010c000), 1, 8, 7},
    {0x8eba60cc, 0x8707b2d8, StenoStroke(0x0050c000), 0, 8, 11},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoStroke(0x00002000), 1, 4, 10},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoStroke(0x00002000), 0, 6, 9},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoStroke(0x00002000), 0, 4, 9},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoStroke(0x00002400), 0, 4, 9},
    {0x8fbcbc2d, 0x8707b2d8, StenoStroke(0x00080000), 0, 8, 7},
    {0x8fbcbc2d, 0x8707b2d8, StenoStroke(0x00280000), 0, 8, 11},
    {0x8ff6cc98, 0x8707b2d8, StenoStroke(0x00190000), 0, 8, 7},
    {0x914d2415, 0x8707b2d8, StenoStroke(0x001a8000), 0, 8, 7},
    {0x914d2415, 0x8707b2d8, StenoStroke(0x007a8000), 0, 8, 11},
    {0x91bb9f6c, 0x8707b2d8, StenoStroke(0x00058000), 1, 8, 7},
    {0x91bb9f6c, 0x8707b2d8, StenoStroke(0x00258000), 0, 8, 11},
    {0x92f07f27, 0x8707b2d8, StenoStroke(0x000a0000), 1, 8, 7},
    {0x9301d5fc, 0x8707b2d8, StenoStroke(0x000dc000), 0, 8, 7},
    {0x9301d5fc, 0x8707b2d8, StenoStroke(0x002dc000), 0, 8, 11},
    {0x9350acf9, 0x8707b2d8, StenoStroke(0x00180000), 0, 8, 7},
    {0x9350acf9, 0x8707b2d8, StenoStroke(0x00780000), 0, 8, 11},
    {0x93ff711d, 0x8707b2d8, StenoStroke(0x0002c000), 1, 8, 7},
    {0x93ff711d, 0x8707b2d8, StenoStroke(0x0022c000), 0, 8, 11},
    {0x948d45d1, 0x8707b2d8, StenoStroke(0x00130000), 0, 8, 7},
    {0x94b1cc4b, 0x8707b2d8, StenoStroke(0x00400000), 0, 8, 7},
    {0x94b1cc4b, 0x8707b2d8, StenoStroke(0x00600000), 0, 8, 11},
    {0x94e4ac3b, 0x8707b2d8, StenoStroke(0x0004c000), 1, 8, 7},
    {0x94e4ac3b, 0x8707b2d8, StenoStroke(0x0024c000), 0, 8, 11},
    {0x95989bc5, 0x8707b2d8, StenoStroke(0x000c8000), 0, 8, 7},
    {0x96445b9d, 0x8707b2d8, StenoStroke(0x000d8000), 0, 8, 7},
    {0x96445b9d, 0x8707b2d8, StenoStroke(0x002d8000), 0, 8, 11},
    {0x9685f6a6, 0x9685f6a6, StenoStroke(0x00000800), 1, 4, 5},
    {0x9685f6a6, 0x9685f6a6, StenoStroke(0x00000800), 0, 6, 5},
    {0x9685f6a6, 0x9685f6a6, StenoStroke(0x00000c00), 0, 4, 5},
    {0x9685f6a6, 0x8707b2d8, StenoStroke(0x00010000), 0, 8, 7},
    {0x96bb243e, 0x8707b2d8, StenoStroke(0x0018c000), 0, 8, 7},
    {0x98574167, 0x8707b2d8, StenoStroke(0x00104000), 0, 8, 7},
    {0x98574167, 0x8707b2d8, StenoStroke(0x00504000), 0, 8, 11},
    {0x9869cb72, 0x8707b2d8, StenoStroke(0x00084000), 1, 8, 7},
    {0x9869cb72, 0x8707b2d8, StenoStroke(0x00284000), 0, 8, 11},
    {0x9a1e494f, 0x8707b2d8, StenoStroke(0x00108000), 1, 8, 7},
    {0x9adfc86f, 0x8707b2d8, StenoStroke(0x000cc000), 0, 8, 7},
    {0x9b3d4b2f, 0x8707b2d8, StenoStroke(0x00024000), 0, 8, 7},
    {0x9b3d4b2f, 0x8707b2d8, StenoStroke(0x00224000), 0, 8, 11},
    {0x9b71cf23, 0x8707b2d8, StenoStroke(0x001a8000), 0, 8, 7},
    {0x9b71cf23, 0x8707b2d8, StenoStroke(0x007a8000), 0, 8, 11},
    {0x9bb080d0, 0x8707b2d8, StenoStroke(0x00460000), 0, 8, 7},
    {0x9bb080d0, 0x8707b2d8, StenoStroke(0x00660000), 0, 8, 11},
    {0x9c428b46, 0x8707b2d8, StenoStroke(0x00194000), 0, 8, 7},
    {0x9c428b46, 0x8707b2d8, StenoStroke(0x00794000), 0, 8, 11},
    {0x9cf70218, 0x8707b2d8, StenoStroke(0x00120000), 0, 8, 7},
    {0x9d45c095, 0x0609009d, StenoStroke(0x00000000), 1, 2, 5},
    {0x9d45c095, 0x8707b2d8, StenoStroke(0x0000c000), 0, 8, 7},
    {0x9d45c095, 0x8707b2d8, StenoStroke(0x0020c000), 0, 8, 11},
    {0x9d46ad5f, 0x8707b2d8, StenoStroke(0x00460000), 0, 8, 7},
    {0x9d546aa1, 0x8707b2d8, StenoStroke(0x0003c000), 1, 8, 7},
    {0x9d546aa1, 0x8707b2d8, StenoStroke(0x0023c000), 0, 8, 11},
    {0x9dc696ce, 0x8707b2d8, StenoStroke(0x00084000), 1, 8, 7},
    {0x9e0aec7b, 0x8707b2d8, StenoStroke(0x00404000), 0, 8, 7},
    {0x9e0aec7b, 0x8707b2d8, StenoStroke(0x00604000), 0, 8, 11},
    {0x9e3c5b8b, 0x8707b2d8, StenoStroke(0x000d8000), 0, 8, 7},
    {0x9e773d63, 0x8707b2d8, StenoStroke(0x00148000), 1, 8, 7},
    {0x9eecc4f0, 0x8707b2d8, StenoStroke(0x00090000), 0, 8, 7},
    {0x9f370ce5, 0x8707b2d8, StenoStroke(0x0002c000), 1, 8, 7},
    {0x9f370ce5, 0x8707b2d8, StenoStroke(0x0022c000), 0, 8, 11},
    {0x9f3fe7fd, 0x8707b2d8, StenoStroke(0x000c4000), 0, 8, 7},
    {0x9f74b898, 0x8707b2d8, StenoStroke(0x00130000), 0, 8, 7},
    {0x9f74b898, 0x8707b2d8, StenoStroke(0x00530000), 0, 8, 11},
    {0x9f7fae07, 0x8707b2d8, StenoStroke(0x00184000), 0, 8, 7},
    {0x9fee2525, 0x8707b2d8, StenoStroke(0x000a4000), 0, 8, 7},
    {0x9fee2525, 0x8707b2d8, StenoStroke(0x002a4000), 0, 8, 11},
    {0xa0047729, 0x8707b2d8, StenoStroke(0x00020000), 0, 8, 7},
    {0xa0047729, 0x8707b2d8, StenoStroke(0x00220000), 0, 8, 11},
    {0xa2232b07, 0x8707b2d8, StenoStroke(0x00024000), 0, 8, 7},
    {0xa2786530, 0x8707b2d8, StenoStroke(0x000b0000), 0, 8, 7},
    {0xa2786530, 0x8707b2d8, StenoStroke(0x002b0000), 0, 8, 11},
    {0xa28e7734, 0x710e300b, StenoStroke(0x00000078), 0, 1, 13},
    {0xa28e7734, 0x710e300b, StenoStroke(0x00000400), 0, 1, 14},
    {0xa32edac3, 0x8707b2d8, StenoStroke(0x00018000), 1, 8, 7},
    {0xa32edac3, 0x8707b2d8, StenoStroke(0x00218000), 0, 8, 11},
    {0xa3f33dfa, 0x8707b2d8, StenoStroke(0x00458000), 0, 8, 7},
    {0xa3f33dfa, 0x8707b2d8, StenoStroke(0x00658000), 0, 8, 11},
    {0xa407938f, 0x8707b2d8, StenoStroke(0x00018000), 1, 8, 7},
    {0xa407938f, 0x8707b2d8, StenoStroke(0x00218000), 0, 8, 11},
    {0xa4520a18, 0x8707b2d8, StenoStroke(0x00100000), 0, 8, 7},
    {0xa4520a18, 0x8707b2d8, StenoStroke(0x00500000), 0, 8, 11},
    {0xa50957cb, 0x8707b2d8, StenoStroke(0x001c8000), 0, 8, 7},
    {0xa50957cb, 0x8707b2d8, StenoStroke(0x007c8000), 0, 8, 11},
    {0xa5126bcf, 0x8707b2d8, StenoStroke(0x000e8000), 0, 8, 7},
    {0xa5126bcf, 0x8707b2d8, StenoStroke(0x002e8000), 0, 8, 11},
    {0xa51e47cc, 0x8707b2d8, StenoStroke(0x00138000), 0, 8, 7},
    {0xa51e47cc, 0x8707b2d8, StenoStroke(0x00538000), 0, 8, 11},
    {0xa5f91580, 0x8707b2d8, StenoStroke(0x00408000), 0, 8, 7},
    {0xa5f91580, 0x8707b2d8, StenoStroke(0x00608000), 0, 8, 11},
    {0xa606c573, 0x8707b2d8, StenoStroke(0x00074000), 0, 8, 7},
    {0xa606c573, 0x8707b2d8, StenoStroke(0x00274000), 0, 8, 11},
    {0xa67b83b8, 0x8707b2d8, StenoStroke(0x0000c000), 1, 8, 7},
    {0xa67b83b8, 0x8707b2d8, StenoStroke(0x0020c000), 0, 8, 11},
    {0xa6d78bd1, 0x8707b2d8, StenoStroke(0x00050000), 1, 8, 7},
    {0xa6d78bd1, 0x8707b2d8, StenoStroke(0x00250000), 0, 8, 11},
    {0xa7191336, 0x8707b2d8, StenoStroke(0x00078000), 1, 8, 7},
    {0xa7191336, 0x8707b2d8, StenoStroke(0x00278000), 0, 8, 11},
    {0xa8539d8c, 0x710e300b, StenoStroke(0x00000066), 0, 1, 13},
    {0xa8539d8c, 0x0609009d, StenoStroke(0x00000146), 0, 2, 14},
    {0xa9eb7789, 0x8707b2d8, StenoStroke(0x0012c000), 0, 8, 7},
    {0xa9eb7789, 0x8707b2d8, StenoStroke(0x0052c000), 0, 8, 11},
    {0xaa583684, 0x0609009d, StenoStroke(0x00000100), 0, 2, 9},
    {0xaa583684, 0x8707b2d8, StenoStroke(0x00550000), 0, 8, 11},
    {0xac49a6db, 0x8707b2d8, StenoStroke(0x00250000), 1, 8, 11},
    {0xac6340b3, 0x8707b2d8, StenoStroke(0x00070000), 1, 8, 7},
    {0xac6340b3, 0x8707b2d8, StenoStroke(0x00270000), 0, 8, 11},
    {0xac94610e, 0x8707b2d8, StenoStroke(0x000c0000), 0, 8, 7},
    {0xac94fac6, 0xac94fac6, StenoStroke(0x00000c00), 0, 6, 9},
    {0xacf0ef7f, 0x8707b2d8, StenoStroke(0x000bc000), 0, 8, 7},
    {0xacf0ef7f, 0x8707b2d8, StenoStroke(0x002bc000), 0, 8, 11},
    {0xad4aa09e, 0x8707b2d8, StenoStroke(0x00108000), 1, 8, 7},
    {0xad4aa09e, 0x8707b2d8, StenoStroke(0x00508000), 0, 8, 11},
    {0xae36585c, 0x8707b2d8, StenoStroke(0x000c0000), 0, 8, 7},
    {0xae36585c, 0x8707b2d8, StenoStroke(0x002c0000), 0, 8, 11},
    {0xae9089d4, 0x8707b2d8, StenoStroke(0x00108000), 1, 8, 7},
    {0xae9089d4, 0x8707b2d8, StenoStroke(0x00508000), 0, 8, 11},
    {0xaf39ccac, 0x8707b2d8, StenoStroke(0x000bc000), 0, 8, 7},
    {0xafeae2a7, 0x8707b2d8, StenoStroke(0x00458000), 0, 8, 7},
    {0xafeae2a7, 0x8707b2d8, StenoStroke(0x00658000), 0, 8, 11},
    {0xb0339683, 0x8707b2d8, StenoStroke(0x00020000), 0, 8, 7},
    {0xb0339683, 0x8707b2d8, StenoStroke(0x00220000), 0, 8, 11},
    {0xb04be33a, 0x8707b2d8, StenoStroke(0x000f8000), 0, 8, 7},
    {0xb0fa6411, 0x8707b2d8, StenoStroke(0x00044000), 1, 8, 7},
    {0xb0fa6411, 0x8707b2d8, StenoStroke(0x00244000), 0, 8, 11},
    {0xb11e4f22, 0x8707b2d8, StenoStroke(0x0003c000), 1, 8, 7},
    {0xb132feca, 0x0609009d, StenoStroke(0x00000072), 0, 2, 14},
    {0xb28860cb, 0x8707b2d8, StenoStroke(0x000f0000), 0, 8, 7},
    {0xb28860cb, 0x8707b2d8, StenoStroke(0x002f0000), 0, 8, 11},
    {0xb28e2435, 0x710e300b, StenoStroke(0x00000064), 0, 1, 13},
    {0xb28e2435, 0x710e300b, StenoStroke(0x00001400), 0, 1, 14},
    {0xb41668c7, 0x8707b2d8, StenoStroke(0x00114000), 1, 8, 7},
    {0xb41668c7, 0x8707b2d8, StenoStroke(0x00514000), 0, 8, 11},
    {0xb42b78da, 0x8707b2d8, StenoStroke(0x00068000), 1, 8, 7},
    {0xb42b78da, 0x8707b2d8, StenoStroke(0x00268000), 0, 8, 11},
    {0xb495d73d, 0x8707b2d8, StenoStroke(0x00188000), 0, 8, 7},
    {0xb495d73d, 0x8707b2d8, StenoStroke(0x00788000), 0, 8, 11},
    {0xb521e95a, 0x8707b2d8, StenoStroke(0x000c4000), 0, 8, 7},
    {0xb521e95a, 0x8707b2d8, StenoStroke(0x002c4000), 0, 8, 11},
    {0xb5d92c3f, 0xb5d92c3f, StenoStroke(0x00000c00), 0, 6, 9},
    {0xb6689356, 0x8707b2d8, StenoStroke(0x00040000), 1, 8, 7},
    {0xb6689356, 0x8707b2d8, StenoStroke(0x00240000), 0, 8, 11},
    {0xb741320d, 0x8707b2d8, StenoStroke(0x00044000), 1, 8, 7},
    {0xb741320d, 0x8707b2d8, StenoStroke(0x00244000), 0, 8, 11},
    {0xb8cb2493, 0x8707b2d8, StenoStroke(0x001c8000), 0, 8, 7},
    {0xb8cb2493, 0x8707b2d8, StenoStroke(0x007c8000), 0, 8, 11},
    {0xb961c051, 0xb961c051, StenoStroke(0x00000800), 1, 6, 9},
    {0xb961c051, 0xb961c051, StenoStroke(0x00000800), 0, 4, 9},
    {0xb961c051, 0xb961c051, StenoStroke(0x00000c00), 0, 4, 9},
    {0xb961c051, 0x8707b2d8, StenoStroke(0x00210000), 0, 8, 11},
    {0xba441fc2, 0x8707b2d8, StenoStroke(0x000c8000), 0, 8, 7},
    {0xba441fc2, 0x8707b2d8, StenoStroke(0x002c8000), 0, 8, 11},
    {0xba745a27, 0x8707b2d8, StenoStroke(0x00140000), 1, 8, 7},
    {0xba745a27, 0x8707b2d8, StenoStroke(0x00540000), 0, 8, 11},
    {0xbae63262, 0x0609009d, StenoStroke(0x00000162), 0, 2, 14},
    {0xbb255e97, 0x8707b2d8, StenoStroke(0x001a0000), 0, 8, 7},
    {0xbb255e97, 0x8707b2d8, StenoStroke(0x007a0000), 0, 8, 11},
    {0xbb48838e, 0x8707b2d8, StenoStroke(0x00098000), 0, 8, 7},
    {0xbb6c0b99, 0x8707b2d8, StenoStroke(0x00014000), 0, 8, 7},
    {0xbbc159ec, 0x8707b2d8, StenoStroke(0x000e8000), 0, 8, 7},
    {0xbe97ab72, 0x8707b2d8, StenoStroke(0x00294000), 0, 8, 11},
    {0xbfafd76c, 0x8707b2d8, StenoStroke(0x001c0000), 0, 8, 7},
    {0xbfafd76c, 0x8707b2d8, StenoStroke(0x007c0000), 0, 8, 11},
    {0xbfe8d608, 0x8707b2d8, StenoStroke(0x00094000), 0, 8, 7},
    {0xbfe8d608, 0x8707b2d8, StenoStroke(0x00294000), 0, 8, 11},
    {0xc11afc41, 0x8707b2d8, StenoStroke(0x00104000), 0, 8, 7},
    {0xc11afc41, 0x8707b2d8, StenoStroke(0x00504000), 0, 8, 11},
    {0xc1389bb2, 0x8707b2d8, StenoStroke(0x001a0000), 0, 8, 7},
    {0xc1389bb2, 0x8707b2d8, StenoStroke(0x007a0000), 0, 8, 11},
    {0xc2c4fcec, 0x0609009d, StenoStroke(0x00000700), 0, 2, 9},
    {0xc3511eb1, 0x8707b2d8, StenoStroke(0x00058000), 1, 8, 7},
    {0xc3551be2, 0x8707b2d8, StenoStroke(0x00044000), 1, 8, 7},
    {0xc3551be2, 0x8707b2d8, StenoStroke(0x00244000), 0, 8, 11},
    {0xc39d69da, 0x8707b2d8, StenoStroke(0x0004c000), 1, 8, 7},
    {0xc39d69da, 0x8707b2d8, StenoStroke(0x0024c000), 0, 8, 11},
    {0xc3e99418, 0x8707b2d8, StenoStroke(0x000c0000), 0, 8, 7},
    {0xc3e99418, 0x8707b2d8, StenoStroke(0x002c0000), 0, 8, 11},
    {0xc40e62d7, 0x8707b2d8, StenoStroke(0x00124000), 1, 8, 7},
    {0xc40e62d7, 0x8707b2d8, StenoStroke(0x00524000), 0, 8, 11},
    {0xc4a8c9d9, 0x8707b2d8, StenoStroke(0x000e0000), 0, 8, 7},
    {0xc55b700d, 0x8707b2d8, StenoStroke(0x000d8000), 0, 8, 7},
    {0xc55b700d, 0x8707b2d8, StenoStroke(0x002d8000), 0, 8, 11},
    {0xc571953b, 0x8707b2d8, StenoStroke(0x000b8000), 0, 8, 7},
    {0xc633348f, 0x8707b2d8, StenoStroke(0x000b0000), 0, 8, 7},
    {0xc6ec8df5, 0x8707b2d8, StenoStroke(0x00428000), 0, 8, 7},
    {0xc6ec8df5, 0x8707b2d8, StenoStroke(0x00628000), 0, 8, 11},
    {0xc75466c1, 0x8707b2d8, StenoStroke(0x00100000), 0, 8, 7},
    {0xc8406cb1, 0x8707b2d8, StenoStroke(0x00104000), 0, 8, 7},
    {0xc9317cf2, 0x8707b2d8, StenoStroke(0x00030000), 1, 8, 7},
    {0xc9317cf2, 0x8707b2d8, StenoStroke(0x00230000), 0, 8, 11},
    {0xc9ae6404, 0x8707b2d8, StenoStroke(0x00078000), 1, 8, 7},
    {0xc9ae6404, 0x8707b2d8, StenoStroke(0x00278000), 0, 8, 11},
    {0xca19cbba, 0x8707b2d8, StenoStroke(0x00070000), 1, 8, 7},
    {0xca19cbba, 0x8707b2d8, StenoStroke(0x00270000), 0, 8, 11},
    {0xcba7e880, 0x8707b2d8, StenoStroke(0x00030000), 1, 8, 7},
    {0xcba7e880, 0x8707b2d8, StenoStroke(0x00230000), 0, 8, 11},
    {0xcbe29615, 0x8707b2d8, StenoStroke(0x0002c000), 1, 8, 7},
    {0xcbe29615, 0x8707b2d8, StenoStroke(0x0022c000), 0, 8, 11},
    {0xcbf0480b, 0x8707b2d8, StenoStroke(0x00158000), 0, 8, 7},
    {0xcbf0480b, 0x8707b2d8, StenoStroke(0x00558000), 0, 8, 11},
    {0xcc2b13c3, 0x8707b2d8, StenoStroke(0x000d4000), 0, 8, 7},
    {0xcc2b13c3, 0x8707b2d8, StenoStroke(0x002d4000), 0, 8, 11},
    {0xcc4cef0e, 0x8707b2d8, StenoStroke(0x000bc000), 0, 8, 7},
    {0xcc4cef0e, 0x8707b2d8, StenoStroke(0x002bc000), 0, 8, 11},
    {0xcc8e2f3e, 0x8707b2d8, StenoStroke(0x00074000), 0, 8, 7},
    {0xcc8e2f3e, 0x8707b2d8, StenoStroke(0x00274000), 0, 8, 11},
    {0xcd2578ed, 0x0609009d, StenoStroke(0x00000400), 0, 2, 9},
    {0xce5c21fa, 0x8707b2d8, StenoStroke(0x00008000), 1, 8, 7},
    {0xcf05a4c2, 0x8707b2d8, StenoStroke(0x0011c000), 1, 8, 7},
    {0xcf05a4c2, 0x8707b2d8, StenoStroke(0x0051c000), 0, 8, 11},
    {0xcf401f72, 0xcf401f72, StenoStroke(0x00003c00), 0, 4, 13},
    {0xcfcd5811, 0x8707b2d8, StenoStroke(0x00194000), 0, 8, 7},
    {0xcfcd5811, 0x8707b2d8, StenoStroke(0x00794000), 0, 8, 11},
    {0xd01f8468, 0x8707b2d8, StenoStroke(0x00038000), 1, 8, 7},
    {0xd0cdcf54, 0x8707b2d8, StenoStroke(0x00080000), 1, 8, 7},
    {0xd0cdcf54, 0x8707b2d8, StenoStroke(0x00280000), 0, 8, 11},
    {0xd1256687, 0x710e300b, StenoStroke(0x000000e8), 0, 1, 13},
    {0xd1256687, 0x710e300b, StenoStroke(0x00000800), 0, 1, 14},
    {0xd2109e05, 0xd2109e05, StenoStroke(0x000014a6), 0, 7, 13},
    {0xd2109e05, 0xd2109e05, StenoStroke(0x000014fe), 0, 7, 13},
    {0xd27eea40, 0xd27eea40, StenoStroke(0x00002000), 1, 4, 13},
    {0xd27eea40, 0xd27eea40, StenoStroke(0x00002000), 0, 6, 5},
    {0xd27eea40, 0xd27eea40, StenoStroke(0x00002000), 0, 4, 6},
    {0xd27eea40, 0xd27eea40, StenoStroke(0x00002400), 0, 4, 13},
    {0xd27eea40, 0x8707b2d8, StenoStroke(0x00080000), 0, 8, 7},
    {0xd27eea40, 0x8707b2d8, StenoStroke(0x00280000), 0, 8, 11},
    {0xd5fe8280, 0x8707b2d8, StenoStroke(0x000f0000), 0, 8, 7},
    {0xd5fe8280, 0x8707b2d8, StenoStroke(0x002f0000), 0, 8, 11},
    {0xd62c7057, 0x8707b2d8, StenoStroke(0x00290000), 0, 8, 11},
    {0xd787d2c4, 0xd787d2c4, StenoStroke(0x000010a6), 0, 7, 13},
    {0xd787d2c4, 0xd787d2c4, StenoStroke(0x000010fe), 0, 7, 13},
    {0xd7d174c9, 0x8707b2d8, StenoStroke(0x00114000), 1, 8, 7},
    {0xd7d174c9, 0x8707b2d8, StenoStroke(0x00514000), 0, 8, 11},
    {0xd8e006ac, 0x8707b2d8, StenoStroke(0x000b0000), 0, 8, 7},
    {0xd8e006ac, 0x8707b2d8, StenoStroke(0x002b0000), 0, 8, 11},
    {0xd95f49c1, 0x8707b2d8, StenoStroke(0x00070000), 1, 8, 7},
    {0xd95f49c1, 0x8707b2d8, StenoStroke(0x00270000), 0, 8, 11},
    {0xd9dbbaac, 0x8707b2d8, StenoStroke(0x0028c000), 0, 8, 11},
    {0xd9e3b838, 0x8707b2d8, StenoStroke(0x00008000), 1, 8, 7},
    {0xd9e3b838, 0x8707b2d8, StenoStroke(0x00208000), 0, 8, 11},
    {0xd9e56117, 0x8707b2d8, StenoStroke(0x0009c000), 0, 8, 7},
    {0xd9e56117, 0x8707b2d8, StenoStroke(0x0029c000), 0, 8, 11},
    {0xdaa35c8f, 0x8707b2d8, StenoStroke(0x00074000), 0, 8, 7},
    {0xdac894e3, 0xdac894e3, StenoStroke(0x00002c00), 0, 6, 5},
    {0xdafa51be, 0x8707b2d8, StenoStroke(0x00008000), 1, 8, 7},
    {0xdafa51be, 0x8707b2d8, StenoStroke(0x00208000), 0, 8, 11},
    {0xdb36efc7, 0x8707b2d8, StenoStroke(0x000e0000), 0, 8, 7},
    {0xdb36efc7, 0x8707b2d8, StenoStroke(0x002e0000), 0, 8, 11},
    {0xdbb7ef0c, 0x8707b2d8, StenoStroke(0x000dc000), 0, 8, 7},
    {0xdbfaf0b0, 0x8707b2d8, StenoStroke(0x00084000), 1, 8, 7},
    {0xdbfaf0b0, 0x8707b2d8, StenoStroke(0x00284000), 0, 8, 11},
    {0xdcb980a8, 0x8707b2d8, StenoStroke(0x000cc000), 0, 8, 7},
    {0xdcb980a8, 0x8707b2d8, StenoStroke(0x002cc000), 0, 8, 11},
    {0xdceebad1, 0x8707b2d8, StenoStroke(0x00094000), 0, 8, 7},
    {0xdd0216b9, 0x710e300b, StenoStroke(0x000000a2), 0, 1, 13},
    {0xdd0216b9, 0x710e300b, StenoStroke(0x00001800), 0, 1, 14},
    {0xdd0fa368, 0x8707b2d8, StenoStroke(0x00114000), 1, 8, 7},
    {0xdd5c9ea2, 0x8707b2d8, StenoStroke(0x0009c000), 0, 8, 7},
    {0xdd5c9ea2, 0x8707b2d8, StenoStroke(0x0029c000), 0, 8, 11},
    {0xdd72bca0, 0x8707b2d8, StenoStroke(0x000c8000), 0, 8, 7},
    {0xdd72bca0, 0x8707b2d8, StenoStroke(0x002c8000), 0, 8, 11},
    {0xdd939ce8, 0x8707b2d8, StenoStroke(0x00098000), 0, 8, 7},
    {0xdd939ce8, 0x8707b2d8, StenoStroke(0x00298000), 0, 8, 11},
    {0xddd85bbd, 0x8707b2d8, StenoStroke(0x00184000), 0, 8, 7},
    {0xddd85bbd, 0x8707b2d8, StenoStroke(0x00784000), 0, 8, 11},
    {0xde51d5bf, 0x8707b2d8, StenoStroke(0x00098000), 0, 8, 7},
    {0xde51d5bf, 0x8707b2d8, StenoStroke(0x00298000), 0, 8, 11},
    {0xded63aa1, 0x0609009d, StenoStroke(0x00000262), 0, 2, 14},
    {0xdee7fb99, 0x8707b2d8, StenoStroke(0x00420000), 0, 8, 7},
    {0xdee7fb99, 0x8707b2d8, StenoStroke(0x00620000), 0, 8, 11},
    {0xe134d5ad, 0x8707b2d8, StenoStroke(0x00400000), 0, 8, 7},
    {0xe134d5ad, 0x8707b2d8, StenoStroke(0x00600000), 0, 8, 11},
    {0xe1bf2d5a, 0x8707b2d8, StenoStroke(0x00110000), 1, 8, 7},
    {0xe1bf2d5a, 0x8707b2d8, StenoStroke(0x00510000), 0, 8, 11},
    {0xe1dbf968, 0x8707b2d8, StenoStroke(0x00040000), 1, 8, 7},
    {0xe1dbf968, 0x8707b2d8, StenoStroke(0x00240000), 0, 8, 11},
    {0xe26e7f10, 0x8707b2d8, StenoStroke(0x001c8000), 0, 8, 7},
    {0xe26e7f10, 0x8707b2d8, StenoStroke(0x007c8000), 0, 8, 11},
    {0xe46e0c39, 0xe46e0c39, StenoStroke(0x00000c00), 0, 6, 5},
    {0xe50a8bf1, 0x8707b2d8, StenoStroke(0x0018c000), 0, 8, 7},
    {0xe50a8bf1, 0x8707b2d8, StenoStroke(0x0078c000), 0, 8, 11},
    {0xe50f412f, 0x8707b2d8, StenoStroke(0x00480000), 0, 8, 7},
    {0xe50f412f, 0x8707b2d8, StenoStroke(0x00680000), 0, 8, 11},
    {0xe593d6de, 0x8707b2d8, StenoStroke(0x000a8000), 0, 8, 7},
    {0xe61425dc, 0x8707b2d8, StenoStroke(0x00130000), 0, 8, 7},
    {0xe61425dc, 0x8707b2d8, StenoStroke(0x00530000), 0, 8, 11},
    {0xe6f46c44, 0x8707b2d8, StenoStroke(0x0004c000), 1, 8, 7},
    {0xe6f46c44, 0x8707b2d8, StenoStroke(0x0024c000), 0, 8, 11},
    {0xe875de63, 0x8707b2d8, StenoStroke(0x0008c000), 0, 8, 7},
    {0xe948c569, 0x8707b2d8, StenoStroke(0x00178000), 1, 8, 7},
    {0xea71121f, 0x8707b2d8, StenoStroke(0x00028000), 1, 8, 7},
    {0xea7cfac2, 0x8707b2d8, StenoStroke(0x00050000), 1, 8, 7},
    {0xead7e731, 0x8707b2d8, StenoStroke(0x0008c000), 0, 8, 7},
    {0xead7e731, 0x8707b2d8, StenoStroke(0x0028c000), 0, 8, 11},
    {0xeb6094e6, 0x8707b2d8, StenoStroke(0x00180000), 0, 8, 7},
    {0xeb6094e6, 0x8707b2d8, StenoStroke(0x00780000), 0, 8, 11},
    {0xebcdd0fb, 0x8707b2d8, StenoStroke(0x000cc000), 0, 8, 7},
    {0xebcdd0fb, 0x8707b2d8, StenoStroke(0x002cc000), 0, 8, 11},
    {0xebd87b09, 0x8707b2d8, StenoStroke(0x00188000), 0, 8, 7},
    {0xebd87b09, 0x8707b2d8, StenoStroke(0x00788000), 0, 8, 11},
    {0xec5b6e29, 0x8707b2d8, StenoStroke(0x001c8000), 0, 8, 7},
    {0xee12f24d, 0x8707b2d8, StenoStroke(0x001a4000), 0, 8, 7},
    {0xef35ef86, 0x8707b2d8, StenoStroke(0x000a0000), 1, 8, 7},
    {0xef35ef86, 0x8707b2d8, StenoStroke(0x002a0000), 0, 8, 11},
    {0xef3b04d6, 0x8707b2d8, StenoStroke(0x00110000), 1, 8, 7},
    {0xef3b04d6, 0x8707b2d8, StenoStroke(0x00510000), 0, 8, 11},
    {0xef3e3778, 0x8707b2d8, StenoStroke(0x00428000), 0, 8, 7},
    {0xef3e3778, 0x8707b2d8, StenoStroke(0x00628000), 0, 8, 11},
    {0xefe51ebf, 0x8707b2d8, StenoStroke(0x00128000), 1, 8, 7},
    {0xefe51ebf, 0x8707b2d8, StenoStroke(0x00528000), 0, 8, 11},
    {0xf0c1c06e, 0x8707b2d8, StenoStroke(0x001a4000), 0, 8, 7},
    {0xf0c1c06e, 0x8707b2d8, StenoStroke(0x007a4000), 0, 8, 11},
    {0xf16ec191, 0x8707b2d8, StenoStroke(0x00120000), 0, 8, 7},
    {0xf16ec191, 0x8707b2d8, StenoStroke(0x00520000), 0, 8, 11},
    {0xf3eb085e, 0x8707b2d8, StenoStroke(0x000ac000), 0, 8, 7},
    {0xf3eb085e, 0x8707b2d8, StenoStroke(0x002ac000), 0, 8, 11},
    {0xf46c3323, 0x8707b2d8, StenoStroke(0x00060000), 1, 8, 7},
    {0xf46c3323, 0x8707b2d8, StenoStroke(0x00260000), 0, 8, 11},
    {0xf493ff29, 0x8707b2d8, StenoStroke(0x000bc000), 0, 8, 7},
    {0xf493ff29, 0x8707b2d8, StenoStroke(0x002bc000), 0, 8, 11},
    {0xf51fcfcd, 0x8707b2d8, StenoStroke(0x007c0000), 0, 8, 11},
    {0xf5221184, 0x8707b2d8, StenoStroke(0x0005c000), 1, 8, 7},
    {0xf52b94f3, 0x8707b2d8, StenoStroke(0x00020000), 0, 8, 7},
    {0xf5984163, 0x8707b2d8, StenoStroke(0x00614000), 0, 8, 11},
    {0xf6e50243, 0x8707b2d8, StenoStroke(0x00054000), 1, 8, 7},
    {0xf6fb6275, 0x8707b2d8, StenoStroke(0x00190000), 0, 8, 7},
    {0xf6fb6275, 0x8707b2d8, StenoStroke(0x00790000), 0, 8, 11},
    {0xf7630619, 0x8707b2d8, StenoStroke(0x001c0000), 0, 8, 7},
    {0xf7630619, 0x8707b2d8, StenoStroke(0x007c0000), 0, 8, 11},
    {0xf770d380, 0x8707b2d8, StenoStroke(0x000d4000), 0, 8, 7},
    {0xf770d380, 0x8707b2d8, StenoStroke(0x002d4000), 0, 8, 11},
    {0xf7ccf8c1, 0x8707b2d8, StenoStroke(0x00290000), 0, 8, 11},
    {0xf998c2f6, 0x8707b2d8, StenoStroke(0x00074000), 0, 8, 7},
    {0xf998c2f6, 0x8707b2d8, StenoStroke(0x00274000), 0, 8, 11},
    {0xfa94e6df, 0x8707b2d8, StenoStroke(0x00400000), 0, 8, 7},
    {0xfb286a06, 0x8707b2d8, StenoStroke(0x00158000), 0, 8, 7},
    {0xfb286a06, 0x8707b2d8, StenoStroke(0x00558000), 0, 8, 11},
    {0xfb8ca445, 0x8707b2d8, StenoStroke(0x00060000), 1, 8, 7},
    {0xfb8ca445, 0x8707b2d8, StenoStroke(0x00260000), 0, 8, 11},
    {0xfc3b8d65, 0x8707b2d8, StenoStroke(0x0011c000), 1, 8, 7},
    {0xfc3b8d65, 0x8707b2d8, StenoStroke(0x0051c000), 0, 8, 11},
    {0xfc8b1c22, 0x0609009d, StenoStroke(0x00000400), 0, 2, 5},
    {0xfca1d0e5, 0x8707b2d8, StenoStroke(0x000e0000), 0, 8, 7},
    {0xfca1d0e5, 0x8707b2d8, StenoStroke(0x002e0000), 0, 8, 11},
    {0xfcc99c65, 0x8707b2d8, StenoStroke(0x00110000), 1, 8, 7},
    {0xfcc99c65, 0x8707b2d8, StenoStroke(0x00510000), 0, 8, 11},
    {0xfd3b2e70, 0x8707b2d8, StenoStroke(0x00140000), 1, 8, 7},
    {0xfd3b2e70, 0x8707b2d8, StenoStroke(0x00540000), 0, 8, 11},
    {0xfe6f5a7e, 0x8707b2d8, StenoStroke(0x00068000), 1, 8, 7},
    {0xfe6f5a7e, 0x8707b2d8, StenoStroke(0x00268000), 0, 8, 11},
    {0xfe885645, 0x8707b2d8, StenoStroke(0x00048000), 1, 8, 7},
    {0xfe8c34a4, 0x8707b2d8, StenoStroke(0x000b8000), 0, 8, 7},
    {0xfe8c34a4, 0x8707b2d8, StenoStroke(0x002b8000), 0, 8, 11},
    {0xfeee8227, 0x710e300b, StenoStroke(0x0000004e), 0, 1, 13},
    {0xff91f1fb, 0x8707b2d8, StenoStroke(0x002c4000), 0, 8, 11},
    {0x00000000, 0x00000000, StenoStroke(0x00000000), 0, 0, 0},
};

constexpr const JeffPhrasingReverseHashMapEntry *REVERSE_HASH_MAP[] = {
    REVERSE_ENTRIES + 0,
    REVERSE_ENTRIES + 1,
    REVERSE_ENTRIES + 200,
    REVERSE_ENTRIES + 766,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 609,
    nullptr,
    REVERSE_ENTRIES + 591,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 622,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 319,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 99,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 280,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 305,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 216,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 649,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 123,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 36,
    REVERSE_ENTRIES + 303,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 363,
    REVERSE_ENTRIES + 399,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 557,
    REVERSE_ENTRIES + 102,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 639,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 524,
    nullptr,
    REVERSE_ENTRIES + 725,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 50,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 721,
    REVERSE_ENTRIES + 448,
    REVERSE_ENTRIES + 17,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 269,
    REVERSE_ENTRIES + 605,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 198,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 458,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 473,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 274,
    REVERSE_ENTRIES + 522,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 170,
    REVERSE_ENTRIES + 262,
    REVERSE_ENTRIES + 662,
    REVERSE_ENTRIES + 60,
    nullptr,
    REVERSE_ENTRIES + 118,
    nullptr,
    REVERSE_ENTRIES + 12,
    REVERSE_ENTRIES + 660,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 516,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 176,
    REVERSE_ENTRIES + 742,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 13,
    REVERSE_ENTRIES + 248,
    nullptr,
    REVERSE_ENTRIES + 542,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 538,
    REVERSE_ENTRIES + 410,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 453,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 544,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 753,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 617,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 40,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 707,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 169,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 389,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 518,
    REVERSE_ENTRIES + 271,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 651,
    REVERSE_ENTRIES + 375,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 428,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 260,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 345,
    nullptr,
    REVERSE_ENTRIES + 692,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 21,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 333,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 447,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 400,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 548,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 98,
    REVERSE_ENTRIES + 734,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 443,
    REVERSE_ENTRIES + 391,
    REVERSE_ENTRIES + 685,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 167,
    nullptr,
    REVERSE_ENTRIES + 395,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 284,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 144,
    nullptr,
    REVERSE_ENTRIES + 359,
    nullptr,
    REVERSE_ENTRIES + 732,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 723,
    nullptr,
    REVERSE_ENTRIES + 83,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 291,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 343,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 133,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 111,
    nullptr,
    REVERSE_ENTRIES + 655,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 237,
    REVERSE_ENTRIES + 646,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 242,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 526,
    REVERSE_ENTRIES + 354,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 590,
    REVERSE_ENTRIES + 584,
    nullptr,
    REVERSE_ENTRIES + 126,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 405,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 32,
    nullptr,
    REVERSE_ENTRIES + 570,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 139,
    REVERSE_ENTRIES + 208,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 217,
    REVERSE_ENTRIES + 618,
    REVERSE_ENTRIES + 765,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 746,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 553,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 79,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 403,
    nullptr,
    REVERSE_ENTRIES + 457,
    REVERSE_ENTRIES + 165,
    REVERSE_ENTRIES + 491,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 73,
    REVERSE_ENTRIES + 701,
    REVERSE_ENTRIES + 210,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 563,
    REVERSE_ENTRIES + 764,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 258,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 330,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 266,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 141,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 631,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 735,
    nullptr,
    REVERSE_ENTRIES + 396,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 290,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 712,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 565,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 241,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 311,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 736,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 759,
    nullptr,
    REVERSE_ENTRIES + 637,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 313,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 240,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 192,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 286,
    REVERSE_ENTRIES + 462,
    REVERSE_ENTRIES + 678,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 529,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 648,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 307,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 702,
    REVERSE_ENTRIES + 485,
    REVERSE_ENTRIES + 640,
    REVERSE_ENTRIES + 87,
    REVERSE_ENTRIES + 519,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 8,
    nullptr,
    REVERSE_ENTRIES + 212,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 664,
    REVERSE_ENTRIES + 309,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 588,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 78,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 324,
    nullptr,
    REVERSE_ENTRIES + 235,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 68,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 366,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 15,
    REVERSE_ENTRIES + 743,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 44,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 135,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 116,
    nullptr,
    REVERSE_ENTRIES + 480,
    nullptr,
    REVERSE_ENTRIES + 114,
    REVERSE_ENTRIES + 709,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 341,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 293,
    REVERSE_ENTRIES + 296,
    REVERSE_ENTRIES + 361,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 301,
    REVERSE_ENTRIES + 383,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 727,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 104,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 449,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 507,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 164,
    REVERSE_ENTRIES + 533,
    nullptr,
    REVERSE_ENTRIES + 187,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 140,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 106,
    REVERSE_ENTRIES + 455,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 96,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 76,
    REVERSE_ENTRIES + 128,
    REVERSE_ENTRIES + 551,
    REVERSE_ENTRIES + 404,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 185,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 667,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 23,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 445,
    REVERSE_ENTRIES + 571,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 252,
    REVERSE_ENTRIES + 131,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 386,
    REVERSE_ENTRIES + 740,
    REVERSE_ENTRIES + 377,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 467,
    REVERSE_ENTRIES + 273,
    nullptr,
    REVERSE_ENTRIES + 568,
    REVERSE_ENTRIES + 489,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 380,
    REVERSE_ENTRIES + 228,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 569,
    REVERSE_ENTRIES + 679,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 160,
    REVERSE_ENTRIES + 436,
    REVERSE_ENTRIES + 62,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 196,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 393,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 578,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 213,
    REVERSE_ENTRIES + 503,
    REVERSE_ENTRIES + 603,
    nullptr,
    REVERSE_ENTRIES + 161,
    REVERSE_ENTRIES + 56,
    REVERSE_ENTRIES + 276,
    REVERSE_ENTRIES + 674,
    REVERSE_ENTRIES + 143,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 611,
    nullptr,
    REVERSE_ENTRIES + 435,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 267,
    nullptr,
    REVERSE_ENTRIES + 495,
    nullptr,
    REVERSE_ENTRIES + 505,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 384,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 72,
    REVERSE_ENTRIES + 122,
    REVERSE_ENTRIES + 300,
    REVERSE_ENTRIES + 582,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 402,
    REVERSE_ENTRIES + 690,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 154,
    nullptr,
    REVERSE_ENTRIES + 601,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 534,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 419,
    REVERSE_ENTRIES + 81,
    nullptr,
    REVERSE_ENTRIES + 586,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 752,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 412,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 19,
    nullptr,
    REVERSE_ENTRIES + 256,
    nullptr,
    REVERSE_ENTRIES + 540,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 689,
    nullptr,
    REVERSE_ENTRIES + 433,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 442,
    REVERSE_ENTRIES + 550,
    REVERSE_ENTRIES + 219,
    REVERSE_ENTRIES + 576,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 697,
    REVERSE_ENTRIES + 748,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 365,
    nullptr,
    REVERSE_ENTRIES + 334,
    REVERSE_ENTRIES + 431,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 254,
    nullptr,
    REVERSE_ENTRIES + 322,
    nullptr,
    REVERSE_ENTRIES + 321,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 355,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 755,
    nullptr,
    REVERSE_ENTRIES + 332,
    REVERSE_ENTRIES + 624,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 223,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 465,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 351,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 336,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 314,
    REVERSE_ENTRIES + 594,
    REVERSE_ENTRIES + 653,
    nullptr,
    REVERSE_ENTRIES + 555,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 418,
    REVERSE_ENTRIES + 162,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 670,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 360,
    REVERSE_ENTRIES + 762,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 528,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 264,
    nullptr,
    REVERSE_ENTRIES + 598,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 54,
    REVERSE_ENTRIES + 70,
    REVERSE_ENTRIES + 250,
    REVERSE_ENTRIES + 619,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 38,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 642,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 715,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 654,
    nullptr,
    REVERSE_ENTRIES + 470,
    REVERSE_ENTRIES + 705,
    nullptr,
    REVERSE_ENTRIES + 672,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 580,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 469,
    nullptr,
    REVERSE_ENTRIES + 599,
    REVERSE_ENTRIES + 733,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 426,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 64,
    REVERSE_ENTRIES + 152,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 94,
    REVERSE_ENTRIES + 234,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 10,
    nullptr,
    REVERSE_ENTRIES + 171,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 225,
    REVERSE_ENTRIES + 476,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 408,
    nullptr,
    REVERSE_ENTRIES + 481,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 593,
    REVERSE_ENTRIES + 246,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 224,
    REVERSE_ENTRIES + 279,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 207,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 683,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 461,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 468,
    nullptr,
    REVERSE_ENTRIES + 750,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 700,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 501,
    nullptr,
    REVERSE_ENTRIES + 24,
    REVERSE_ENTRIES + 101,
    REVERSE_ENTRIES + 232,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 499,
    nullptr,
    REVERSE_ENTRIES + 188,
    REVERSE_ENTRIES + 288,
    REVERSE_ENTRIES + 238,
    nullptr,
    REVERSE_ENTRIES + 174,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 509,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 58,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 75,
    REVERSE_ENTRIES + 89,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 681,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 676,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 124,
    nullptr,
    REVERSE_ENTRIES + 346,
    REVERSE_ENTRIES + 430,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 695,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 206,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 595,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 215,
    REVERSE_ENTRIES + 487,
    nullptr,
    REVERSE_ENTRIES + 46,
    REVERSE_ENTRIES + 130,
    REVERSE_ENTRIES + 178,
    REVERSE_ENTRIES + 424,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 369,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 629,
    nullptr,
    REVERSE_ENTRIES + 475,
    REVERSE_ENTRIES + 149,
    REVERSE_ENTRIES + 574,
    nullptr,
    REVERSE_ENTRIES + 181,
    nullptr,
    REVERSE_ENTRIES + 156,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 607,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 738,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 137,
    nullptr,
    REVERSE_ENTRIES + 7,
    nullptr,
    REVERSE_ENTRIES + 204,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 711,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 25,
    REVERSE_ENTRIES + 316,
    nullptr,
    REVERSE_ENTRIES + 30,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 66,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 147,
    REVERSE_ENTRIES + 761,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 189,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 112,
    nullptr,
    REVERSE_ENTRIES + 120,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 295,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 699,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 85,
    nullptr,
    REVERSE_ENTRIES + 757,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 398,
    nullptr,
    REVERSE_ENTRIES + 179,
    REVERSE_ENTRIES + 221,
    REVERSE_ENTRIES + 227,
    REVERSE_ENTRIES + 255,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 371,
    REVERSE_ENTRIES + 513,
    REVERSE_ENTRIES + 531,
    nullptr,
    REVERSE_ENTRIES + 627,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 138,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 566,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 109,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 668,
    REVERSE_ENTRIES + 326,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 438,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 644,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 298,
    nullptr,
    REVERSE_ENTRIES + 581,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 107,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 665,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 719,
    nullptr,
    REVERSE_ENTRIES + 597,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 537,
    REVERSE_ENTRIES + 407,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 464,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 515,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 694,
    REVERSE_ENTRIES + 745,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 338,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 230,
    nullptr,
    REVERSE_ENTRIES + 42,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 329,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 22,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 28,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 659,
    nullptr,
    REVERSE_ENTRIES + 388,
    REVERSE_ENTRIES + 613,
    REVERSE_ENTRIES + 687,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 191,
    nullptr,
    REVERSE_ENTRIES + 536,
    REVERSE_ENTRIES + 451,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 423,
    nullptr,
    REVERSE_ENTRIES + 478,
    REVERSE_ENTRIES + 729,
    nullptr,
    REVERSE_ENTRIES + 183,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 703,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 483,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 4,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 546,
    REVERSE_ENTRIES + 615,
    REVERSE_ENTRIES + 150,
    REVERSE_ENTRIES + 381,
    REVERSE_ENTRIES + 244,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 5,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 153,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 625,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 367,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 421,
    REVERSE_ENTRIES + 572,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 621,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 717,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 520,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 357,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 713,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 511,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 339,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 379,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 282,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 561,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 202,
    REVERSE_ENTRIES + 657,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 493,
    REVERSE_ENTRIES + 497,
    REVERSE_ENTRIES + 731,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 52,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 373,
    nullptr,
    REVERSE_ENTRIES + 146,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 26,
    REVERSE_ENTRIES + 472,
    nullptr,
    nullptr,
};

constexpr JeffPhrasingReverseStructureEntry REVERSE_STRUCTURES[] = {
    {0x04851ccd, StenoStroke(0x00002000), 5},
    {0x065238df, StenoStroke(0x00000800), 5},
    {0x0a8e1d6a, StenoStroke(0x00002000), 7},
    {0x0c5b397c, StenoStroke(0x00000800), 5},
    {0x123b84f3, StenoStroke(0x00003800), 13},
    {0x132bc08a, StenoStroke(0x00000800), 9},
    {0x18448590, StenoStroke(0x00003800), 13},
    {0x1934c127, StenoStroke(0x00000800), 9},
    {0x29e9718d, StenoStroke(0x00000800), 13},
    {0x2a7a0b8b, StenoStroke(0x00000c00), 5},
    {0x3500c57a, StenoStroke(0x00000800), 5},
    {0x3b09c617, StenoStroke(0x00000800), 5},
    {0x3cd428dc, StenoStroke(0x00000c00), 5},
    {0x3f6f7f54, StenoStroke(0x00002400), 5},
    {0x45962ca4, StenoStroke(0x00002c00), 9},
    {0x4b29d51a, StenoStroke(0x00002400), 5},
    {0x4e5d86a1, StenoStroke(0x00001800), 13},
    {0x5466873e, StenoStroke(0x00001800), 13},
    {0x591850dd, StenoStroke(0x000014a6), 13},
    {0x591850dd, StenoStroke(0x000014fe), 13},
    {0x5e160918, StenoStroke(0x00002800), 5},
    {0x5e8f859c, StenoStroke(0x000010a6), 13},
    {0x5e8f859c, StenoStroke(0x000010fe), 13},
    {0x623f7986, StenoStroke(0x00002800), 5},
    {0x641f09b5, StenoStroke(0x00002800), 13},
    {0x68487a23, StenoStroke(0x00002800), 5},
    {0x6cb5e0b3, StenoStroke(0x00003000), 13},
    {0x6d108763, StenoStroke(0x00002800), 9},
    {0x72bee150, StenoStroke(0x00003000), 13},
    {0x73198800, StenoStroke(0x00002800), 9},
    {0x7d37495c, StenoStroke(0x00002c00), 5},
    {0x82add63a, StenoStroke(0x00002400), 9},
    {0x87d29f10, StenoStroke(0x00002000), 9},
    {0x8ddb9fad, StenoStroke(0x00002000), 11},
    {0x8e9bd989, StenoStroke(0x00000800), 5},
    {0x94a4da26, StenoStroke(0x00000800), 5},
    {0xa4aadda9, StenoStroke(0x00000c00), 9},
    {0xadef0f22, StenoStroke(0x00000c00), 9},
    {0xb177a334, StenoStroke(0x00000800), 9},
    {0xb780a3d1, StenoStroke(0x00000800), 9},
    {0xca94cd23, StenoStroke(0x00002000), 5},
    {0xcd5f02f2, StenoStroke(0x00003800), 13},
    {0xd09dcdc0, StenoStroke(0x00002000), 15},
    {0xd2de77c6, StenoStroke(0x00002c00), 5},
    {0xdc83ef1c, StenoStroke(0x00000c00), 5},
    {0xf815e2e3, StenoStroke(0x00000000), 13},
    {0xfe1ee380, StenoStroke(0x00000000), 15},
    {0xfe1ee380, StenoStroke(0x00001000), 13},
    {0x00000000, StenoStroke(0x00000000), 0},
};

constexpr const JeffPhrasingReverseStructureEntry *REVERSE_STRUCTURES_MAP[] = {
    REVERSE_STRUCTURES + 29,
    REVERSE_STRUCTURES + 46,
    REVERSE_STRUCTURES + 48,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 23,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 34,
    REVERSE_STRUCTURES + 5,
    REVERSE_STRUCTURES + 9,
    nullptr,
    REVERSE_STRUCTURES + 8,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 6,
    REVERSE_STRUCTURES + 32,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 11,
    REVERSE_STRUCTURES + 20,
    nullptr,
    REVERSE_STRUCTURES + 15,
    nullptr,
    REVERSE_STRUCTURES + 21,
    REVERSE_STRUCTURES + 44,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 16,
    REVERSE_STRUCTURES + 37,
    REVERSE_STRUCTURES + 25,
    REVERSE_STRUCTURES + 14,
    REVERSE_STRUCTURES + 40,
    REVERSE_STRUCTURES + 35,
    REVERSE_STRUCTURES + 7,
    nullptr,
    REVERSE_STRUCTURES + 36,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 33,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 26,
    REVERSE_STRUCTURES + 38,
    REVERSE_STRUCTURES + 24,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 31,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 17,
    nullptr,
    REVERSE_STRUCTURES + 42,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 43,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 0,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 28,
    REVERSE_STRUCTURES + 39,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 13,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 12,
    REVERSE_STRUCTURES + 18,
    REVERSE_STRUCTURES + 30,
    REVERSE_STRUCTURES + 1,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 27,
    REVERSE_STRUCTURES + 45,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 2,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 41,
    REVERSE_STRUCTURES + 4,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 10,
    nullptr,
    REVERSE_STRUCTURES + 3,
    nullptr,
    nullptr,
    nullptr,
};

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
  printf("static const uint8_t VALID_CHARACTERS[32] = {\n");
  for (size_t i = 0; i < 32; ++i) {
    if (i != 0) {
      printf(", ");
    }
    printf("0x%02x", charactersPresent[i]);
  }
  printf("\n};\n");
  printf("\n");
}

void GenerateLookupTable() {
  printf("endersCount: %zu\n", phrasingData.endersCount);

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
  printf("\\0 CRC: 0x%08x\n", Crc32("\\0", 2));
  printf("\\1 CRC: 0x%08x\n", Crc32("\\1", 2));
  printf("\\2 CRC: 0x%08x\n", Crc32("\\2", 2));
  printf("\\3 CRC: 0x%08x\n", Crc32("\\3", 2));

  GenerateLookupTable();

  ReverseBuilder reverseBuilder;
  reverseBuilder.Generate();

  ReverseStructureBuilder reverseStructureBuilder;
  reverseStructureBuilder.Generate();

  PrintCharacters();
  return 0;
}

#endif
