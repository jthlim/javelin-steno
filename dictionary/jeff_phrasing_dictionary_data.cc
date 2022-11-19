//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary_data.h"
#include "../crc32.h"
#include "dictionary.h"
#include <string.h>

//---------------------------------------------------------------------------

// spellchecker: disable

// clang-format off
constexpr JeffPhrasingSimpleStarter SIMPLE_STARTERS[] = {
    {StenoChord(0x146 /*STHA*/), {VerbForm::UNSPECIFIED, "that"}},
    {StenoChord(0x116 /*STPA*/), {VerbForm::UNSPECIFIED, "if"}},
    {StenoChord(0x62 /*SWH*/), {VerbForm::UNSPECIFIED, "when"}},
    {StenoChord(0x162 /*SWHA*/), {VerbForm::UNSPECIFIED, "what"}},
    {StenoChord(0xe2 /*SWHR*/), {VerbForm::UNSPECIFIED, "where"}},
    {StenoChord(0x146 /*STHA*/), {VerbForm::UNSPECIFIED, "that"}},

    // # Remove the entry below if you don't want "and" phrases.
    {StenoChord(0x1a /*SKP*/), {VerbForm::UNSPECIFIED, "and"}},
};

constexpr JeffPhrasingFullStarter FULL_STARTERS[] = {
  {StenoChord(0xa2 /*SWR*/), {"I", VerbForm::FIRST_PERSON_SINGULAR, true}},
  {StenoChord(0xb8 /*KPWR*/), {"you", VerbForm::SECOND_PERSON, true}},
  {StenoChord(0xe8 /*KWHR*/), {"he", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0xea /*SKWHR*/), {"she", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0x78 /*KPWH*/), {"it", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0xa4 /*TWR*/), {"we", VerbForm::FIRST_PERSON_PLURAL, true}},
  {StenoChord(0x64 /*TWH*/), {"they", VerbForm::THIRD_PERSON_PLURAL, true}},
  {StenoChord(0x4e /*STKH*/), {"this", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0x66 /*STWH*/), {"that", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0xc6 /*STHR*/), {"there", VerbForm::THIRD_PERSON_SINGULAR, false}},
  {StenoChord(0xd6 /*STPHR*/), {"there", VerbForm::THIRD_PERSON_PLURAL, false}},
  {StenoChord(0xfe /*STKPWHR*/), {"", VerbForm::THIRD_PERSON_SINGULAR, true}},
  {StenoChord(0xa6 /*STWR*/), {"", VerbForm::THIRD_PERSON_PLURAL, true}},
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
    {StenoChord(), {"\\0\\2\\3", false, VerbForm::UNSPECIFIED}},

    {StenoChord(0x800 /*E*/), {&STRUCTURE_EXCEPTION_BE, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoChord(0xC00 /**E*/), {&STRUCTURE_EXCEPTION_BE_NOT, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoChord(0x2000 /*F*/), {&STRUCTURE_EXCEPTION_HAVE, false, VerbForm::PAST_PARTICIPLE}},
    {StenoChord(0x2400 /**F*/), {&STRUCTURE_EXCEPTION_HAVE_NOT, false, VerbForm::PAST_PARTICIPLE}},
    {StenoChord(0x2800 /*EF*/), {&STRUCTURE_EXCEPTION_HAVE_BEEN, false, VerbForm::PRESENT_PARTICIPLE}},
    {StenoChord(0x2c00 /**EF*/), {&STRUCTURE_EXCEPTION_HAVE_NOT_BEEN, false, VerbForm::PRESENT_PARTICIPLE}},

    {StenoChord(0x1800 /*EU*/), {"\\0 still\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x3800 /*EUF*/), {"\\0 never\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x3000 /*UF*/), {"\\0 just\\2\\3", false, VerbForm::UNSPECIFIED}},
};

constexpr JeffPhrasingStructureException UNIQUE_STARTERS[] = {
    {StenoChord(0x10a6 /*STWRU*/), {"to\\2\\3", false, VerbForm::ROOT}},
    {StenoChord(0x14a6 /*STWR*U*/), {"not to\\2\\3", false, VerbForm::ROOT}},
    {StenoChord(0x10fe /*STKPWHRU*/), {"to\\2\\3", false, VerbForm::ROOT}},
    {StenoChord(0x14fe /*STKPWHR*U*/), {"not to\\2\\3", false, VerbForm::ROOT}},
    {StenoChord(0x30a6 /*STWRUF*/), {"just\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x34a6 /*STWR*UF*/), {"just\\1\\2\\3", true, VerbForm::UNSPECIFIED}},
    {StenoChord(0x30fe /*STKPWHRUF*/), {"just\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x34fe /*STKPWHR*UF*/), {"just\\1\\2\\3", true, VerbForm::UNSPECIFIED}},
    {StenoChord(0x18a6 /*STWREU*/), {"still\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x1ca6 /*STWR*EU*/), {"still\\1\\2\\3", true, VerbForm::UNSPECIFIED}},
    {StenoChord(0x18fe /*STKPWHREU*/), {"still\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x1cfe /*STKPWHR*EU*/), {"still\\1\\2\\3", true, VerbForm::UNSPECIFIED}},
    {StenoChord(0x38a6 /*STWREUF*/), {"never\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x3ca6 /*STWR*EUF*/), {"even\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x38fe /*STKPWHREUF*/), {"never\\2\\3", false, VerbForm::UNSPECIFIED}},
    {StenoChord(0x3cfe /*STKPWHR*EUF*/), {"even\\2\\3", false, VerbForm::UNSPECIFIED}},
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
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " learnt"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBSZ = {
    .entries =
        {
            {0, " learnt"},
            {(uint32_t)VerbForm::ROOT, " learn"},
            {(uint32_t)VerbForm::PRESENT_PARTICIPLE, " learning"},
            {(uint32_t)VerbForm::PAST_PARTICIPLE, " learnt"},
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
    {StenoChord(), Tense::PRESENT, true, "", ""},
    {StenoChord(0x200000 /*D*/), Tense::PAST, true, "", ""},

    // RB - To ask
    {StenoChord(0x14000 /*RB*/), Tense::PRESENT, false, &ENDER_RB, ""},
    {StenoChord(0x214000 /*-RBD*/), Tense::PAST, false, &ENDER_RBD, ""},

    // B - To be (a)
    {StenoChord(0x10000 /*B*/), Tense::PRESENT, true, &ENDER_B, ""},
    {StenoChord(0x210000 /*BD*/), Tense::PAST, true, &ENDER_BD, ""},
    {StenoChord(0x90000 /*BT*/), Tense::PRESENT, true, &ENDER_B, " a"},
    {StenoChord(0x290000 /*BTD*/), Tense::PAST, true, &ENDER_BD, " a"},

    // RPBG - To become (a)
    {StenoChord(0x5c000 /*-RPBG*/), Tense::PRESENT, false, &ENDER_RPBG, ""},
    {StenoChord(0x25c000 /*-RPBGD*/), Tense::PAST, false, &ENDER_RPBGD, ""},
    {StenoChord(0xdc000 /*-RPBGT*/), Tense::PRESENT, false, &ENDER_RPBG, " a"},
    {StenoChord(0x2dc000 /*-RPBGTD*/), Tense::PAST, false, &ENDER_RPBGD, " a"},

    // BL - To believe (that)
    {StenoChord(0x30000 /*-BL*/), Tense::PRESENT, false, &ENDER_BL, ""},
    {StenoChord(0x230000 /*-BLD*/), Tense::PAST, false, &ENDER_BLD, ""},
    {StenoChord(0xb0000 /*-BLT*/), Tense::PRESENT, false, &ENDER_BL, " that"},
    {StenoChord(0x2b0000 /*-BLTD*/), Tense::PAST, false, &ENDER_BLD, " that"},

    // RBLG - To call
    {StenoChord(0x74000 /*-RBLG*/), Tense::PRESENT, false, &ENDER_RBLG, ""},
    {StenoChord(0x274000 /*-RBLGD*/), Tense::PAST, false, &ENDER_RBLGD, ""},

    // BGS - Can -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoChord(0x150000 /*-BGS*/), Tense::PRESENT, false, " can", ""},
    {StenoChord(0x550000 /*-BGSZ*/), Tense::PAST, false, " could", ""},

    // RZ - To care
    {StenoChord(0x404000 /*-RZ*/), Tense::PRESENT, false, &ENDER_RZ, ""},
    {StenoChord(0x604000 /*-RDZ*/), Tense::PAST, false, &ENDER_RDZ, ""},

    // BG - To come (to)
    {StenoChord(0x50000 /*-BG*/), Tense::PRESENT, true, &ENDER_BG, ""},
    {StenoChord(0x250000 /*-BGD*/), Tense::PAST, true, &ENDER_BGD, ""},
    {StenoChord(0xd0000 /*-BGT*/), Tense::PRESENT, true, &ENDER_BG, " to"},
    {StenoChord(0x2d0000 /*BGTD*/), Tense::PAST, true, &ENDER_BGD, " to"},

    // RP - To do (it)
    {StenoChord(0xc000 /*-RP*/), Tense::PRESENT, false, &ENDER_RP, ""},
    {StenoChord(0x20c000 /*-RPD*/), Tense::PAST, false, &ENDER_RPD, ""},
    {StenoChord(0x8c000 /*-RPT*/), Tense::PRESENT, false, &ENDER_RP, " it"},
    {StenoChord(0x28c000 /*-RPTD*/), Tense::PAST, false, &ENDER_RPD, " it"},

    // RP - To expect (that)
    {StenoChord(0x148000 /*-PGS*/), Tense::PRESENT, false, &ENDER_PGS, ""},
    {StenoChord(0x548000 /*-PGSZ*/), Tense::PAST, false, &ENDER_PGSZ, ""},
    {StenoChord(0x1c8000 /*-PGTS*/), Tense::PRESENT, false, &ENDER_PGS, " that"},
    {StenoChord(0x7c8000 /*-PGTSDZ*/), Tense::PAST, false, &ENDER_PGSZ, " that"},

    // LT - To feel (like)
    {StenoChord(0xa0000 /*-LT*/), Tense::PRESENT, false, &ENDER_LT, ""},
    {StenoChord(0x2a0000 /*-LTD*/), Tense::PAST, false, &ENDER_LTD, ""},
    {StenoChord(0x1a0000 /*-LTS*/), Tense::PRESENT, false, &ENDER_LT, " like"},
    {StenoChord(0x7a0000 /*-LTSDZ*/), Tense::PAST, false, &ENDER_LTD, " like"},

    // PBLG - To find (that)
    {StenoChord(0x78000 /*-PBLG*/), Tense::PRESENT, false, &ENDER_PBLG, ""},
    {StenoChord(0x278000 /*-PBLGD*/), Tense::PAST, false, &ENDER_PBLGD, ""},
    {StenoChord(0xf8000 /*-PBLGT*/), Tense::PRESENT, false, &ENDER_PBLG, " that"},
    {StenoChord(0x2f8000 /*-PBLGTD*/), Tense::PAST, false, &ENDER_PBLGD, " that"},

    // RG - To forget (to)
    {StenoChord(0x44000 /*-RG*/), Tense::PRESENT, false, &ENDER_RG, ""},
    {StenoChord(0x244000 /*-RGD*/), Tense::PAST, false, &ENDER_RGD, ""},
    {StenoChord(0xc4000 /*-RGT*/), Tense::PRESENT, false, &ENDER_RG, " to"},
    {StenoChord(0x2c4000 /*-RGTD*/), Tense::PAST, false, &ENDER_RGD, " to"},

    // GS - To get (to)
    {StenoChord(0x140000 /*-GS*/), Tense::PRESENT, false, &ENDER_GS, ""},
    {StenoChord(0x540000 /*-GSZ*/), Tense::PAST, false, &ENDER_GSZ, ""},
    {StenoChord(0x1c0000 /*-GTS*/), Tense::PRESENT, false, &ENDER_GS, " to"},
    {StenoChord(0x7c0000 /*-GTSDZ*/), Tense::PAST, false, &ENDER_GSZ, " to"},

    // GZ - To give
    {StenoChord(0x440000 /*-GZ*/), Tense::PRESENT, false, &ENDER_GZ, ""},
    {StenoChord(0x640000 /*-GDZ*/), Tense::PAST, false, &ENDER_GDZ, ""},

    // G - To go (to)
    {StenoChord(0x40000 /*G*/), Tense::PRESENT, true, &ENDER_G, ""},
    {StenoChord(0x240000 /*GD*/), Tense::PAST, true, &ENDER_GD, ""},
    {StenoChord(0xC0000 /*GT*/), Tense::PRESENT, true, &ENDER_G, " to"},
    {StenoChord(0x2C0000 /*GTD*/), Tense::PAST, true, &ENDER_GD, " to"},

    // PZ - To happen
    {StenoChord(0x408000 /*-PZ*/), Tense::PRESENT, true, &ENDER_PZ, ""},
    {StenoChord(0x608000 /*-PDZ*/), Tense::PAST, true, &ENDER_PDZ, ""},

    // H - To have (to)
    {StenoChord(0x80000 /*T*/), Tense::PRESENT, true, &ENDER_T, ""},
    {StenoChord(0x280000 /*TD*/), Tense::PAST, true, &ENDER_TD, ""},
    {StenoChord(0x180000 /*TS*/), Tense::PRESENT, true, &ENDER_T, " to"},
    {StenoChord(0x780000 /*TSDZ*/), Tense::PAST, true, &ENDER_TD, " to"},

    // PG - To hear (that)
    {StenoChord(0x48000 /*-PG*/), Tense::PRESENT, false, &ENDER_PG, ""},
    {StenoChord(0x248000 /*-PGD*/), Tense::PAST, false, &ENDER_PGD, ""},
    {StenoChord(0xc8000 /*-PGT*/), Tense::PRESENT, false, &ENDER_PG, " that"},
    {StenoChord(0x2c8000 /*-PGTD*/), Tense::PAST, false, &ENDER_PGD, " that"},

    // RPS - To hope (to)
    {StenoChord(0x10c000 /*-RPS*/), Tense::PRESENT, false, &ENDER_RPS, ""},
    {StenoChord(0x50c000 /*-RPSZ*/), Tense::PAST, false, &ENDER_RPSZ, ""},
    {StenoChord(0x18c000 /*-RPTS*/), Tense::PRESENT, false, &ENDER_RPS, " to"},
    {StenoChord(0x78c000 /*-RPTSDZ*/), Tense::PAST, false, &ENDER_RPSZ, " to"},

    // PLG - To imagine (that)
    {StenoChord(0x68000 /*-PLG*/), Tense::PRESENT, false, &ENDER_PLG, ""},
    {StenoChord(0x268000 /*-PLGD*/), Tense::PAST, false, &ENDER_PLGD, ""},
    {StenoChord(0xe8000 /*-PLGT*/), Tense::PRESENT, false, &ENDER_PLG, " that"},
    {StenoChord(0x2e8000 /*-PLGTD*/), Tense::PAST, false, &ENDER_PLGD, " that"},

    // PBGS - To keep
    {StenoChord(0x158000 /*-PBGS*/), Tense::PRESENT, false, &ENDER_PBGS, ""},
    {StenoChord(0x558000 /*-PBGSZ*/), Tense::PAST, false, &ENDER_PBGSZ, ""},

    // PB - To know (that)
    {StenoChord(0x18000 /*-PB*/), Tense::PRESENT, false, &ENDER_PB, ""},
    {StenoChord(0x218000 /*-PBD*/), Tense::PAST, false, &ENDER_PBD, ""},
    {StenoChord(0x98000 /*-PBT*/), Tense::PRESENT, false, &ENDER_PB, " that"},
    {StenoChord(0x298000 /*-PBTD*/), Tense::PAST, false, &ENDER_PBD, " that"},

    // RPBS - To learn (to)
    {StenoChord(0x11c000 /*-RPBS*/), Tense::PRESENT, false, &ENDER_RPBS, ""},
    {StenoChord(0x51c000 /*-RPBSZ*/), Tense::PAST, false, &ENDER_RPBSZ, ""},
    {StenoChord(0x19c000 /*-RPBTS*/), Tense::PRESENT, false, &ENDER_RPBS, " to"},
    {StenoChord(0x79c000 /*-RPBTSDZ*/), Tense::PAST, false, &ENDER_RPBSZ, " to"},

    // LGZ - To leave
    {StenoChord(0x460000 /*-LGZ*/), Tense::PRESENT, false, &ENDER_LGZ, ""},
    {StenoChord(0x660000 /*-LGDZ*/), Tense::PAST, false, &ENDER_LGDZ, ""},

    // LS - To let
    {StenoChord(0x120000 /*-LS*/), Tense::PRESENT, false, &ENDER_LS, ""},
    {StenoChord(0x520000 /*-LSZ*/), Tense::PAST, false, &ENDER_LSZ, ""},

    // BLG - To like (to)
    {StenoChord(0x70000 /*-BLG*/), Tense::PRESENT, false, &ENDER_BLG, ""},
    {StenoChord(0x270000 /*-BLGD*/), Tense::PAST, false, &ENDER_BLGD, ""},
    {StenoChord(0xf0000 /*-BLGT*/), Tense::PRESENT, false, &ENDER_BLG, " to"},
    {StenoChord(0x2f0000 /*-BLGTD*/), Tense::PAST, false, &ENDER_BLGD, " to"},

    // LZ - To live
    {StenoChord(0x420000 /*-LZ*/), Tense::PRESENT, true, &ENDER_LZ, ""},
    {StenoChord(0x620000 /*-LDZ*/), Tense::PAST, true, &ENDER_LDZ, ""},

    // L - To look
    {StenoChord(0x20000 /*-L*/), Tense::PRESENT, false, &ENDER_L, ""},
    {StenoChord(0x220000 /*-LD*/), Tense::PAST, false, &ENDER_LD, ""},

    // LG - To love (to)
    {StenoChord(0x60000 /*-LG*/), Tense::PRESENT, false, &ENDER_LG, ""},
    {StenoChord(0x260000 /*-LGD*/), Tense::PAST, false, &ENDER_LGD, ""},
    {StenoChord(0xe0000 /*-LGT*/), Tense::PRESENT, false, &ENDER_LG, " to"},
    {StenoChord(0x2e0000 /*-LGTD*/), Tense::PAST, false, &ENDER_LGD, " to"},

    // RPBL - To make (the)
    {StenoChord(0x3c000 /*-RPBL*/), Tense::PRESENT, false, &ENDER_RPBL, ""},
    {StenoChord(0x23c000 /*-RPBLD*/), Tense::PAST, false, &ENDER_RPBLD, ""},
    {StenoChord(0xbc000 /*-RPBLT*/), Tense::PRESENT, false, &ENDER_RPBL, " the"},
    {StenoChord(0x2bc000 /*-RPBLTD*/), Tense::PAST, false, &ENDER_RPBLD, " the"},

    // PL - may (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoChord(0x28000 /*-PL*/), Tense::PRESENT, true, " may", ""},
    {StenoChord(0x228000 /*-PLD*/), Tense::PAST, true, " might", ""},
    {StenoChord(0xa8000 /*-PLT*/), Tense::PRESENT, true, " may", " be"},
    {StenoChord(0x2a8000 /*-PLTD*/), Tense::PAST, true, " might", " be"},

    // PBL - To mean (to)
    {StenoChord(0x38000 /*-PBL*/), Tense::PRESENT, false, &ENDER_PBL, ""},
    {StenoChord(0x238000 /*-PBLD*/), Tense::PAST, false, &ENDER_PBLD, ""},
    {StenoChord(0xb8000 /*-PBLT*/), Tense::PRESENT, false, &ENDER_PBL, " to"},
    {StenoChord(0x2b8000 /*-PBLTD*/), Tense::PAST, false, &ENDER_PBLD, " to"},

    // PLZ - To move
    {StenoChord(0x428000 /*-PLZ*/), Tense::PRESENT, false, &ENDER_PLZ, ""},
    {StenoChord(0x628000 /*-PLDZ*/), Tense::PAST, false, &ENDER_PLDZ, ""},

    // PBLGS - must (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoChord(0x178000 /*-PBLGS*/), Tense::PRESENT, true, " must", ""},
    {StenoChord(0x1f8000 /*-PBLGTS*/), Tense::PRESENT, true, " must", " be"},

    // RPG - To need (to)
    {StenoChord(0x4c000 /*-RPG*/), Tense::PRESENT, true, &ENDER_RPG, ""},
    {StenoChord(0x24c000 /*-RPGD*/), Tense::PAST, true, &ENDER_RPGD, ""},
    {StenoChord(0xcc000 /*-RPGT*/), Tense::PRESENT, true, &ENDER_RPG, " to"},
    {StenoChord(0x2cc000 /*-RPGTD*/), Tense::PAST, true, &ENDER_RPGD, " to"},

    // PS - To put (it)
    {StenoChord(0x108000 /*-PS*/), Tense::PRESENT, false, &ENDER_PS, ""},
    {StenoChord(0x508000 /*-PSZ*/), Tense::PAST, false, &ENDER_PSZ, ""},
    {StenoChord(0x188000 /*-PTS*/), Tense::PRESENT, false, &ENDER_PS, " it"},
    {StenoChord(0x788000 /*-PTSDZ*/), Tense::PAST, false, &ENDER_PSZ, " it"},

    // RS - To read
    {StenoChord(0x104000 /*-RS*/), Tense::PRESENT, false, &ENDER_RS, ""},
    {StenoChord(0x504000 /*-RSZ*/), Tense::PAST, false, &ENDER_RSZ, ""},

    // RLG - really
    {StenoChord(0x64000 /*-RLG*/), Tense::PRESENT, true, " really", ""},
    {StenoChord(0x264000 /*-RLG*/), Tense::PAST, true, " really", ""},

    // RL - To recall
    {StenoChord(0x24000 /*-RL*/), Tense::PRESENT, false, &ENDER_RL, ""},
    {StenoChord(0x224000 /*-RLD*/), Tense::PAST, false, &ENDER_RLD, ""},

    // RLS - To realize (that)
    {StenoChord(0x124000 /*-RLS*/), Tense::PRESENT, false, &ENDER_RLS, ""},
    {StenoChord(0x524000 /*-RLSZ*/), Tense::PAST, false, &ENDER_RLSZ, ""},
    {StenoChord(0x1a4000 /*-RLTS*/), Tense::PRESENT, false, &ENDER_RLS, " that"},
    {StenoChord(0x7a4000 /*-RLTSDZ*/), Tense::PAST, false, &ENDER_RLSZ, " that"},

    // RPL - To remember (that)
    {StenoChord(0x2c000 /*-RPL*/), Tense::PRESENT, false, &ENDER_RPL, ""},
    {StenoChord(0x22c000 /*-RPLD*/), Tense::PAST, false, &ENDER_RPLD, ""},
    {StenoChord(0xac000 /*-RPLT*/), Tense::PRESENT, false, &ENDER_RPL, " that"},
    {StenoChord(0x2ac000 /*-RPLTD*/), Tense::PAST, false, &ENDER_RPLD, " that"},

    // RPLS - To remain
    {StenoChord(0x12c000 /*-RPLS*/), Tense::PRESENT, false, &ENDER_RPLS, ""},
    {StenoChord(0x52c000 /*-RPLSZ*/), Tense::PAST, false, &ENDER_RPLSZ, ""},

    // R - To run
    {StenoChord(0x4000 /*-R*/), Tense::PRESENT, false, &ENDER_R, ""},
    {StenoChord(0x204000 /*-RD*/), Tense::PAST, false, &ENDER_RD, ""},

    // BS - To say (that)
    {StenoChord(0x110000 /*-BS*/), Tense::PRESENT, false, &ENDER_BS, ""},
    {StenoChord(0x510000 /*-BSZ*/), Tense::PAST, false, &ENDER_BSZ, ""},
    {StenoChord(0x190000 /*-BTS*/), Tense::PRESENT, false, &ENDER_BS, " that"},
    {StenoChord(0x790000 /*-BTSDZ*/), Tense::PAST, false, &ENDER_BSZ, " that"},

    // S - To see
    {StenoChord(0x100000 /*-S*/), Tense::PRESENT, false, &ENDER_S, ""},
    {StenoChord(0x500000 /*-SZ*/), Tense::PAST, false, &ENDER_SZ, ""},

    // BLS - To set
    {StenoChord(0x130000 /*-BLS*/), Tense::PRESENT, false, &ENDER_BLS, ""},
    {StenoChord(0x530000 /*-BLSZ*/), Tense::PAST, false, &ENDER_BLSZ, ""},

    // PLS - To seem (to)
    {StenoChord(0x128000 /*-PLS*/), Tense::PRESENT, true, &ENDER_PLS, ""},
    {StenoChord(0x528000 /*-PLSZ*/), Tense::PAST, true, &ENDER_PLSZ, ""},
    {StenoChord(0x1a8000 /*-PLTS*/), Tense::PRESENT, true, &ENDER_PLS, " to"},
    {StenoChord(0x7a8000 /*-PLTSDZ*/), Tense::PAST, true, &ENDER_PLSZ, " to"},

    // RBL - shall - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoChord(0x34000 /*RBL*/), Tense::PRESENT, false, " shall", ""},
    {StenoChord(0x234000 /*RBLD*/), Tense::PAST, false, " should", ""},

    // RBZ - To show
    {StenoChord(0x414000 /*-RBZ*/), Tense::PRESENT, false, &ENDER_RBZ, ""},
    {StenoChord(0x614000 /*-RBDZ*/), Tense::PAST, false, &ENDER_RBDZ, ""},

    // RBT - To take
    {StenoChord(0x94000 /*-RBT*/), Tense::PRESENT, false, &ENDER_RBT, ""},
    {StenoChord(0x294000 /*-RBTD*/), Tense::PAST, false, &ENDER_RBTD, ""},

    // // BLGT - To talk
    // {StenoChord(0xf0000 /*-BLGT*/), Tense::PRESENT, false, &ENDER_BLGT, ""},
    // {StenoChord(0x2f0000 /*-BLGTD*/), Tense::PAST, false, &ENDER_BLGTD, ""},

    // RLT - To tell
    {StenoChord(0xa4000 /*-RLT*/), Tense::PRESENT, false, &ENDER_RLT, ""},
    {StenoChord(0x2a4000 /*-RLTD*/), Tense::PAST, false, &ENDER_RLTD, ""},

    // PBG - To think (that)
    {StenoChord(0x58000 /*-PBG*/), Tense::PRESENT, false, &ENDER_PBG, ""},
    {StenoChord(0x258000 /*-PBGD*/), Tense::PAST, false, &ENDER_PBGD, ""},
    {StenoChord(0xd8000 /*-PBGT*/), Tense::PRESENT, false, &ENDER_PBG, " that"},
    {StenoChord(0x2d8000 /*-PBGTD*/), Tense::PAST, false, &ENDER_PBGD, " that"},

    // RT - To try (to)
    {StenoChord(0x84000 /*-RT*/), Tense::PRESENT, false, &ENDER_RT, ""},
    {StenoChord(0x284000 /*-RTD*/), Tense::PAST, false, &ENDER_RTD, ""},
    {StenoChord(0x184000 /*-RTS*/), Tense::PRESENT, false, &ENDER_RT, " to"},
    {StenoChord(0x784000 /*-RTSDZ*/), Tense::PAST, false, &ENDER_RTD, " to"},

    // RPB - To understand (the)
    {StenoChord(0x1c000 /*-RPB*/), Tense::PRESENT, false, &ENDER_RPB, ""},
    {StenoChord(0x21c000 /*-RPBD*/), Tense::PAST, false, &ENDER_RPBD, ""},
    {StenoChord(0x9c000 /*-RPBT*/), Tense::PRESENT, false, &ENDER_RPB, " the"},
    {StenoChord(0x29c000 /*-RPBTD*/), Tense::PAST, false, &ENDER_RPBD, " the"},

    // Z - To use
    {StenoChord(0x400000 /*-Z*/), Tense::PRESENT, true, &ENDER_Z, ""},
    {StenoChord(0x600000 /*-DZ*/), Tense::PAST, true, &ENDER_DZ, ""},
    // TZ - Special case
    {StenoChord(0x480000 /*-TZ*/), Tense::PRESENT, true, " used", " to"},
    {StenoChord(0x680000 /*-TDZ*/), Tense::PAST, true, " used", " to"},

    // P - To want (to)
    {StenoChord(0x8000 /*P*/), Tense::PRESENT, false, &ENDER_P, ""},
    {StenoChord(0x208000 /*PD*/), Tense::PAST, false, &ENDER_PD, ""},
    {StenoChord(0x88000 /*PT*/), Tense::PRESENT, false, &ENDER_P, " to"},
    {StenoChord(0x288000 /*PTD*/), Tense::PAST, false, &ENDER_PD, " to"},

    // RBGS - will -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoChord(0x154000 /*RBGS*/), Tense::PRESENT, false, " will", ""},
    {StenoChord(0x554000 /*RBGSZ*/), Tense::PAST, false, " would", ""},

    // RBS - To wish (to)
    {StenoChord(0x114000 /*-RBS*/), Tense::PRESENT, false, &ENDER_RBS, ""},
    {StenoChord(0x514000 /*-RBSZ*/), Tense::PAST, false, &ENDER_RBSZ, ""},
    {StenoChord(0x194000 /*-RBTS*/), Tense::PRESENT, false, &ENDER_RBS, " to"},
    {StenoChord(0x794000 /*-RBTSDZ*/), Tense::PAST, false, &ENDER_RBSZ, " to"},

    // RBG - To work (on)
    {StenoChord(0x54000 /*-RBG*/), Tense::PRESENT, false, &ENDER_RBG, ""},
    {StenoChord(0x254000 /*-RBGD*/), Tense::PAST, false, &ENDER_RBGD, ""},
    {StenoChord(0xd4000 /*-RBGT*/), Tense::PRESENT, false, &ENDER_RBG, " on"},
    {StenoChord(0x2d4000 /*-RBGTD*/), Tense::PAST, false, &ENDER_RBGD, " on"},
};
// clang-format on

constexpr StenoChord NON_PHRASE_STROKES[] = {
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
    nullptr,      nullptr,      ENDERS + 115, nullptr,      ENDERS + 144,
    nullptr,      nullptr,      ENDERS + 132, ENDERS + 67,  nullptr,
    ENDERS + 114, nullptr,      ENDERS + 142, ENDERS + 19,  nullptr,
    ENDERS + 130, ENDERS + 54,  ENDERS + 176, nullptr,      nullptr,
    nullptr,      ENDERS + 172, ENDERS + 106, nullptr,      ENDERS + 52,
    ENDERS + 174, nullptr,      ENDERS + 16,  nullptr,      ENDERS + 170,
    ENDERS + 104, nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 99,  nullptr,
    nullptr,      ENDERS + 191, nullptr,      ENDERS + 103, ENDERS + 181,
    nullptr,      ENDERS + 97,  ENDERS + 93,  ENDERS + 157, ENDERS + 189,
    nullptr,      ENDERS + 101, ENDERS + 179, nullptr,      nullptr,
    ENDERS + 48,  nullptr,      nullptr,      ENDERS + 82,  nullptr,
    nullptr,      ENDERS + 152, ENDERS + 147, ENDERS + 46,  nullptr,
    nullptr,      ENDERS + 80,  nullptr,      nullptr,      ENDERS + 150,
    ENDERS + 118, nullptr,      nullptr,      ENDERS + 40,  nullptr,
    ENDERS + 6,   ENDERS + 160, nullptr,      ENDERS + 116, nullptr,
    nullptr,      ENDERS + 38,  nullptr,      ENDERS + 4,   ENDERS + 128,
    nullptr,      ENDERS + 37,  nullptr,      nullptr,      ENDERS + 187,
    nullptr,      nullptr,      ENDERS + 33,  nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 165, nullptr,      ENDERS + 15,  ENDERS + 167, nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 163, nullptr,
    ENDERS + 13,  ENDERS + 141, nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 168, nullptr,
    nullptr,      ENDERS + 74,  nullptr,      ENDERS + 139, nullptr,
    nullptr,      ENDERS + 124, ENDERS + 148, nullptr,      nullptr,
    ENDERS + 190, ENDERS + 98,  nullptr,      ENDERS + 180, nullptr,
    nullptr,      ENDERS + 102, ENDERS + 156, ENDERS + 188, ENDERS + 96,
    ENDERS + 92,  ENDERS + 178, nullptr,      nullptr,      ENDERS + 100,
    nullptr,      nullptr,      ENDERS + 69,  nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      ENDERS + 55,  ENDERS + 177,
    ENDERS + 107, nullptr,      nullptr,      ENDERS + 173, nullptr,
    ENDERS + 17,  ENDERS + 53,  ENDERS + 175, ENDERS + 105, nullptr,
    nullptr,      ENDERS + 171, nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 122, nullptr,      nullptr,
    ENDERS + 182, ENDERS + 185, ENDERS + 87,  nullptr,      ENDERS + 31,
    ENDERS + 120, nullptr,      nullptr,      nullptr,      ENDERS + 164,
    nullptr,      nullptr,      ENDERS + 166, nullptr,      nullptr,
    ENDERS + 14,  nullptr,      ENDERS + 162, nullptr,      nullptr,
    ENDERS + 140, nullptr,      nullptr,      ENDERS + 12,  nullptr,
    nullptr,      ENDERS + 61,  nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 41,  ENDERS + 119, nullptr,      ENDERS + 161,
    nullptr,      nullptr,      ENDERS + 7,   nullptr,      ENDERS + 39,
    ENDERS + 117, nullptr,      ENDERS + 129, nullptr,      nullptr,
    ENDERS + 5,   nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 68,  nullptr,      nullptr,      ENDERS + 18,
    ENDERS + 143, ENDERS + 131, nullptr,      nullptr,      ENDERS + 66,
    nullptr,      nullptr,      nullptr,      ENDERS + 10,  ENDERS + 72,
    nullptr,      ENDERS + 58,  nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 8,   ENDERS + 70,  ENDERS + 112, ENDERS + 0,
    ENDERS + 50,  nullptr,      ENDERS + 154, nullptr,      nullptr,
    ENDERS + 169, nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      ENDERS + 65,  nullptr,      ENDERS + 35,  nullptr,
    nullptr,      ENDERS + 159, nullptr,      nullptr,      ENDERS + 63,
    ENDERS + 57,  ENDERS + 95,  ENDERS + 85,  nullptr,      ENDERS + 3,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 60,  nullptr,      nullptr,      nullptr,      ENDERS + 81,
    ENDERS + 151, nullptr,      ENDERS + 47,  ENDERS + 146, nullptr,
    nullptr,      nullptr,      ENDERS + 24,  nullptr,      nullptr,
    ENDERS + 28,  nullptr,      nullptr,      ENDERS + 110, nullptr,
    ENDERS + 22,  ENDERS + 126, nullptr,      ENDERS + 26,  nullptr,
    nullptr,      ENDERS + 108, nullptr,      nullptr,      ENDERS + 123,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      ENDERS + 91,
    ENDERS + 45,  nullptr,      ENDERS + 137, nullptr,      nullptr,
    ENDERS + 79,  nullptr,      ENDERS + 89,  ENDERS + 43,  ENDERS + 21,
    ENDERS + 135, nullptr,      nullptr,      ENDERS + 77,  nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 125, nullptr,      nullptr,
    ENDERS + 149, nullptr,      ENDERS + 75,  nullptr,      ENDERS + 138,
    ENDERS + 64,  nullptr,      nullptr,      nullptr,      nullptr,
    ENDERS + 158, ENDERS + 34,  nullptr,      ENDERS + 62,  ENDERS + 56,
    nullptr,      nullptr,      nullptr,      ENDERS + 2,   ENDERS + 94,
    ENDERS + 84,  ENDERS + 133, nullptr,      nullptr,      ENDERS + 145,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      ENDERS + 73,  nullptr,      nullptr,
    ENDERS + 11,  nullptr,      nullptr,      ENDERS + 59,  nullptr,
    ENDERS + 71,  ENDERS + 113, nullptr,      ENDERS + 9,   nullptr,
    ENDERS + 155, ENDERS + 1,   ENDERS + 51,  nullptr,      ENDERS + 32,
    nullptr,      nullptr,      ENDERS + 186, nullptr,      nullptr,
    ENDERS + 36,  ENDERS + 121, ENDERS + 30,  nullptr,      nullptr,
    ENDERS + 184, ENDERS + 183, nullptr,      ENDERS + 86,  ENDERS + 44,
    nullptr,      nullptr,      ENDERS + 90,  nullptr,      ENDERS + 78,
    ENDERS + 136, nullptr,      ENDERS + 42,  ENDERS + 20,  nullptr,
    ENDERS + 88,  nullptr,      ENDERS + 76,  ENDERS + 134, nullptr,
    ENDERS + 153, nullptr,      nullptr,      ENDERS + 83,  nullptr,
    nullptr,      ENDERS + 49,  nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      nullptr,
    nullptr,      nullptr,      nullptr,      nullptr,      ENDERS + 25,
    nullptr,      ENDERS + 111, ENDERS + 29,  nullptr,      ENDERS + 127,
    nullptr,      nullptr,      ENDERS + 23,  nullptr,      ENDERS + 109,
    ENDERS + 27,  nullptr,
};
constexpr JeffPhrasingReverseHashMapEntry REVERSE_ENTRIES[] = {
    {0x00000000, 0x8707b2d8, StenoChord(0x00000000), 0, 8, 3},
    {0x00000000, 0x710e300b, StenoChord(0x000000a6), 0, 1, 1},
    {0x00000000, 0x710e300b, StenoChord(0x000000fe), 0, 1, 1},
    {0x00000000, 0x8707b2d8, StenoChord(0x00200000), 0, 136, 3},
    {0x02033737, 0x8707b2d8, StenoChord(0x00194000), 0, 8, 3},
    {0x02984f45, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x02984f45, 0x8707b2d8, StenoChord(0x00040000), 0, 8, 3},
    {0x02984f45, 0x8707b2d8, StenoChord(0x00240000), 0, 136, 3},
    {0x037d861e, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x037d861e, 0x8707b2d8, StenoChord(0x00124000), 0, 8, 3},
    {0x040092cb, 0x8707b2d8, StenoChord(0x00098000), 0, 8, 3},
    {0x040092cb, 0x8707b2d8, StenoChord(0x00298000), 0, 136, 3},
    {0x0505e520, 0x8707b2d8, StenoChord(0x000a4000), 0, 8, 3},
    {0x0505e520, 0x8707b2d8, StenoChord(0x002a4000), 0, 136, 3},
    {0x063120af, 0x0609009d, StenoChord(0x00000400), 0, 2, 1},
    {0x0649e8c4, 0x8707b2d8, StenoChord(0x000a4000), 0, 8, 3},
    {0x0649e8c4, 0x8707b2d8, StenoChord(0x002a4000), 0, 136, 3},
    {0x06aa82f6, 0x8707b2d8, StenoChord(0x00094000), 0, 8, 3},
    {0x06aa82f6, 0x8707b2d8, StenoChord(0x00294000), 0, 136, 3},
    {0x06ee4070, 0x8707b2d8, StenoChord(0x000f8000), 0, 8, 3},
    {0x06ee4070, 0x8707b2d8, StenoChord(0x002f8000), 0, 136, 3},
    {0x070d8c31, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x070d8c31, 0x0609009d, StenoChord(0x00000000), 0, 2, 1},
    {0x070d8c31, 0x8707b2d8, StenoChord(0x0000c000), 0, 8, 3},
    {0x0743a13d, 0x8707b2d8, StenoChord(0x00088000), 0, 8, 3},
    {0x07d4c6fa, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x07d4c6fa, 0x8707b2d8, StenoChord(0x00540000), 0, 136, 3},
    {0x07f59b6d, 0x0609009d, StenoChord(0x0000001a), 0, 2, 2},
    {0x0805fd75, 0x00000000, StenoChord(0x00000000), 1, 0, 1},
    {0x081e2e34, 0x8707b2d8, StenoChord(0x00408000), 0, 8, 3},
    {0x0966a6ff, 0x8707b2d8, StenoChord(0x000e0000), 0, 8, 3},
    {0x0966a6ff, 0x8707b2d8, StenoChord(0x002e0000), 0, 136, 3},
    {0x0a817e37, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0a817e37, 0x8707b2d8, StenoChord(0x0010c000), 0, 8, 3},
    {0x0a817e37, 0x8707b2d8, StenoChord(0x0050c000), 0, 136, 3},
    {0x0c6f39ea, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0c6f39ea, 0x0c6f39ea, StenoChord(0x00002000), 0, 4, 2},
    {0x0c6f39ea, 0x0c6f39ea, StenoChord(0x00002000), 0, 6, 1},
    {0x0c6f39ea, 0x0c6f39ea, StenoChord(0x00002000), 0, 4, 1},
    {0x0c6f39ea, 0x0c6f39ea, StenoChord(0x00002400), 0, 4, 1},
    {0x0c6f39ea, 0x8707b2d8, StenoChord(0x00080000), 0, 8, 3},
    {0x0ce50045, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0ce50045, 0x8707b2d8, StenoChord(0x00140000), 0, 8, 3},
    {0x0ce50045, 0x8707b2d8, StenoChord(0x00540000), 0, 136, 3},
    {0x0ceb84c6, 0x8707b2d8, StenoChord(0x00460000), 0, 8, 3},
    {0x0ceb84c6, 0x8707b2d8, StenoChord(0x00660000), 0, 136, 3},
    {0x0d2db8f0, 0x8707b2d8, StenoChord(0x00014000), 0, 8, 3},
    {0x0d2db8f0, 0x8707b2d8, StenoChord(0x00214000), 0, 136, 3},
    {0x0daeceeb, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0daeceeb, 0x8707b2d8, StenoChord(0x00124000), 0, 8, 3},
    {0x0daeceeb, 0x8707b2d8, StenoChord(0x00524000), 0, 136, 3},
    {0x0e15f2fe, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0e15f2fe, 0x8707b2d8, StenoChord(0x0001c000), 0, 8, 3},
    {0x0e15f2fe, 0x8707b2d8, StenoChord(0x0021c000), 0, 136, 3},
    {0x0e3c55fc, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x0e3c55fc, 0x0e3c55fc, StenoChord(0x00000800), 0, 4, 1},
    {0x0e3c55fc, 0x0e3c55fc, StenoChord(0x00000800), 0, 6, 1},
    {0x0e3c55fc, 0x0e3c55fc, StenoChord(0x00000c00), 0, 4, 1},
    {0x0e3c55fc, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x0eb86064, 0x8707b2d8, StenoChord(0x0019c000), 0, 8, 3},
    {0x0eb86064, 0x8707b2d8, StenoChord(0x0079c000), 0, 136, 3},
    {0x0ec73fd6, 0x8707b2d8, StenoChord(0x00190000), 0, 8, 3},
    {0x0ec73fd6, 0x8707b2d8, StenoChord(0x00790000), 0, 136, 3},
    {0x0ee254bf, 0x8707b2d8, StenoChord(0x00120000), 0, 8, 3},
    {0x0ee254bf, 0x8707b2d8, StenoChord(0x00520000), 0, 136, 3},
    {0x0ef063bd, 0x8707b2d8, StenoChord(0x00100000), 0, 8, 3},
    {0x0ef063bd, 0x8707b2d8, StenoChord(0x00500000), 0, 136, 3},
    {0x0f3d9d9d, 0x8707b2d8, StenoChord(0x00414000), 0, 8, 3},
    {0x0f3d9d9d, 0x8707b2d8, StenoChord(0x00614000), 0, 136, 3},
    {0x102de0ab, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x102de0ab, 0x8707b2d8, StenoChord(0x0000c000), 0, 8, 3},
    {0x102de0ab, 0x8707b2d8, StenoChord(0x0020c000), 0, 136, 3},
    {0x104f239f, 0x8707b2d8, StenoChord(0x000c4000), 0, 8, 3},
    {0x104f239f, 0x8707b2d8, StenoChord(0x002c4000), 0, 136, 3},
    {0x10889d0f, 0x8707b2d8, StenoChord(0x000f8000), 0, 8, 3},
    {0x10889d0f, 0x8707b2d8, StenoChord(0x002f8000), 0, 136, 3},
    {0x11eb3640, 0x8707b2d8, StenoChord(0x00440000), 0, 8, 3},
    {0x11eb3640, 0x8707b2d8, StenoChord(0x00640000), 0, 136, 3},
    {0x12cfe2ee, 0x8707b2d8, StenoChord(0x0008c000), 0, 8, 3},
    {0x12cfe2ee, 0x8707b2d8, StenoChord(0x0028c000), 0, 136, 3},
    {0x14d30cbf, 0x8707b2d8, StenoChord(0x0012c000), 0, 8, 3},
    {0x14d30cbf, 0x8707b2d8, StenoChord(0x0052c000), 0, 136, 3},
    {0x15b823df, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x15b823df, 0x8707b2d8, StenoChord(0x00040000), 0, 8, 3},
    {0x17b46a1f, 0x8707b2d8, StenoChord(0x000b8000), 0, 8, 3},
    {0x17b46a1f, 0x8707b2d8, StenoChord(0x002b8000), 0, 136, 3},
    {0x1830eda6, 0x8707b2d8, StenoChord(0x000f0000), 0, 8, 3},
    {0x19a74354, 0x8707b2d8, StenoChord(0x00158000), 0, 8, 3},
    {0x19a74354, 0x8707b2d8, StenoChord(0x00558000), 0, 136, 3},
    {0x19c072db, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x19c072db, 0x8707b2d8, StenoChord(0x00018000), 0, 8, 3},
    {0x1a25a210, 0x1a25a210, StenoChord(0x00003800), 0, 4, 1},
    {0x1a25a210, 0x1a25a210, StenoChord(0x00003800), 0, 6, 1},
    {0x1a25a210, 0x1a25a210, StenoChord(0x000038a6), 0, 7, 1},
    {0x1a25a210, 0x1a25a210, StenoChord(0x000038fe), 0, 7, 1},
    {0x1a473416, 0x8707b2d8, StenoChord(0x00184000), 0, 8, 3},
    {0x1a473416, 0x8707b2d8, StenoChord(0x00784000), 0, 136, 3},
    {0x1a834993, 0x8707b2d8, StenoChord(0x00024000), 0, 8, 3},
    {0x1a834993, 0x8707b2d8, StenoChord(0x00224000), 0, 136, 3},
    {0x1acc766e, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1acc766e, 0x8707b2d8, StenoChord(0x0003c000), 0, 8, 3},
    {0x1acc766e, 0x8707b2d8, StenoChord(0x0023c000), 0, 136, 3},
    {0x1aeec2c5, 0x8707b2d8, StenoChord(0x000dc000), 0, 8, 3},
    {0x1aeec2c5, 0x8707b2d8, StenoChord(0x002dc000), 0, 136, 3},
    {0x1b15dda7, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1b15dda7, 0x1b15dda7, StenoChord(0x00000800), 0, 132, 1},
    {0x1b15dda7, 0x1b15dda7, StenoChord(0x00000800), 0, 134, 1},
    {0x1b15dda7, 0x1b15dda7, StenoChord(0x00000c00), 0, 132, 1},
    {0x1b15dda7, 0x8707b2d8, StenoChord(0x00210000), 0, 136, 3},
    {0x1b479518, 0x8707b2d8, StenoChord(0x00088000), 0, 8, 3},
    {0x1b479518, 0x8707b2d8, StenoChord(0x00288000), 0, 136, 3},
    {0x1b501b4e, 0x8707b2d8, StenoChord(0x000f0000), 0, 8, 3},
    {0x1b501b4e, 0x8707b2d8, StenoChord(0x002f0000), 0, 136, 3},
    {0x1b71b162, 0x8707b2d8, StenoChord(0x00500000), 0, 136, 3},
    {0x1c7f6819, 0x710e300b, StenoChord(0x000000a4), 0, 1, 1},
    {0x1c7f6819, 0x710e300b, StenoChord(0x00001c00), 0, 1, 2},
    {0x1caaa576, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1caaa576, 0x8707b2d8, StenoChord(0x00078000), 0, 8, 3},
    {0x1cc2b052, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1cc2b052, 0x8707b2d8, StenoChord(0x00148000), 0, 8, 3},
    {0x1cc2b052, 0x8707b2d8, StenoChord(0x00548000), 0, 136, 3},
    {0x1d82eb26, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1d82eb26, 0x8707b2d8, StenoChord(0x0005c000), 0, 8, 3},
    {0x1d82eb26, 0x8707b2d8, StenoChord(0x0025c000), 0, 136, 3},
    {0x1d8f9346, 0x0609009d, StenoChord(0x000000e2), 0, 2, 2},
    {0x1d90eeb5, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1d90eeb5, 0x8707b2d8, StenoChord(0x0001c000), 0, 8, 3},
    {0x1d90eeb5, 0x8707b2d8, StenoChord(0x0021c000), 0, 136, 3},
    {0x1dd9069f, 0x8707b2d8, StenoChord(0x000d0000), 0, 8, 3},
    {0x1dd9069f, 0x8707b2d8, StenoChord(0x002d0000), 0, 136, 3},
    {0x1dd969bc, 0x8707b2d8, StenoChord(0x00404000), 0, 8, 3},
    {0x1de6be4e, 0x0609009d, StenoChord(0x00000300), 0, 2, 1},
    {0x1de6be4e, 0x8707b2d8, StenoChord(0x00154000), 0, 8, 3},
    {0x1e151309, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1e151309, 0x8707b2d8, StenoChord(0x00084000), 0, 8, 3},
    {0x1e151309, 0x8707b2d8, StenoChord(0x00284000), 0, 136, 3},
    {0x1f3c70ad, 0x8707b2d8, StenoChord(0x00408000), 0, 8, 3},
    {0x1f3c70ad, 0x8707b2d8, StenoChord(0x00608000), 0, 136, 3},
    {0x1f522e50, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x1f522e50, 0x8707b2d8, StenoChord(0x00030000), 0, 8, 3},
    {0x1f522e50, 0x8707b2d8, StenoChord(0x00230000), 0, 136, 3},
    {0x1fe9e3df, 0x8707b2d8, StenoChord(0x000d4000), 0, 8, 3},
    {0x20663dce, 0x8707b2d8, StenoChord(0x000c0000), 0, 8, 3},
    {0x20663dce, 0x8707b2d8, StenoChord(0x002c0000), 0, 136, 3},
    {0x212afb55, 0x8707b2d8, StenoChord(0x001a4000), 0, 8, 3},
    {0x212afb55, 0x8707b2d8, StenoChord(0x007a4000), 0, 136, 3},
    {0x2171cdfd, 0x0609009d, StenoChord(0x00000700), 0, 2, 1},
    {0x21f3e377, 0x8707b2d8, StenoChord(0x000ac000), 0, 8, 3},
    {0x21f3e377, 0x8707b2d8, StenoChord(0x002ac000), 0, 136, 3},
    {0x2273d1a5, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x2273d1a5, 0x8707b2d8, StenoChord(0x0003c000), 0, 8, 3},
    {0x2273d1a5, 0x8707b2d8, StenoChord(0x0023c000), 0, 136, 3},
    {0x22bfa302, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x22bfa302, 0x8707b2d8, StenoChord(0x0005c000), 0, 8, 3},
    {0x22bfa302, 0x8707b2d8, StenoChord(0x0025c000), 0, 136, 3},
    {0x23e80e1c, 0x0609009d, StenoChord(0x00000062), 0, 2, 2},
    {0x23eb5694, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x23eb5694, 0x8707b2d8, StenoChord(0x00128000), 0, 8, 3},
    {0x249319ef, 0x8707b2d8, StenoChord(0x001f8000), 0, 8, 3},
    {0x262a9340, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x262a9340, 0x8707b2d8, StenoChord(0x00110000), 0, 8, 3},
    {0x266fa23b, 0x8707b2d8, StenoChord(0x00414000), 0, 8, 3},
    {0x266fa23b, 0x8707b2d8, StenoChord(0x00614000), 0, 136, 3},
    {0x26aefbc0, 0x8707b2d8, StenoChord(0x00180000), 0, 8, 3},
    {0x27684180, 0x8707b2d8, StenoChord(0x00190000), 0, 8, 3},
    {0x27684180, 0x8707b2d8, StenoChord(0x00790000), 0, 136, 3},
    {0x27d367ef, 0x8707b2d8, StenoChord(0x002d0000), 0, 136, 3},
    {0x27de2e44, 0x0609009d, StenoChord(0x00000200), 0, 130, 1},
    {0x27de2e44, 0x8707b2d8, StenoChord(0x00234000), 0, 136, 3},
    {0x28218e08, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x28218e08, 0x8707b2d8, StenoChord(0x0025c000), 0, 136, 3},
    {0x28ec173f, 0x8707b2d8, StenoChord(0x000d8000), 0, 8, 3},
    {0x28ec173f, 0x8707b2d8, StenoChord(0x002d8000), 0, 136, 3},
    {0x29ee350f, 0x8707b2d8, StenoChord(0x0012c000), 0, 8, 3},
    {0x2ae44f4f, 0x8707b2d8, StenoChord(0x00440000), 0, 8, 3},
    {0x2b3ad402, 0x8707b2d8, StenoChord(0x00020000), 0, 8, 3},
    {0x2b3ad402, 0x8707b2d8, StenoChord(0x00220000), 0, 136, 3},
    {0x2bca8e0d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x2bca8e0d, 0x2bca8e0d, StenoChord(0x00000800), 0, 4, 1},
    {0x2bca8e0d, 0x2bca8e0d, StenoChord(0x00000800), 0, 132, 1},
    {0x2bca8e0d, 0x2bca8e0d, StenoChord(0x00000c00), 0, 4, 1},
    {0x2bca8e0d, 0x2bca8e0d, StenoChord(0x00000c00), 0, 132, 1},
    {0x2bca8e0d, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x2bca8e0d, 0x8707b2d8, StenoChord(0x00210000), 0, 136, 3},
    {0x2c316b9d, 0x8707b2d8, StenoChord(0x002dc000), 0, 136, 3},
    {0x2d5aebbc, 0x0609009d, StenoChord(0x00000600), 0, 130, 1},
    {0x2d9bcc99, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x2d9bcc99, 0x8707b2d8, StenoChord(0x000a0000), 0, 8, 3},
    {0x2d9bcc99, 0x8707b2d8, StenoChord(0x002a0000), 0, 136, 3},
    {0x2e086b3a, 0x8707b2d8, StenoChord(0x000ac000), 0, 8, 3},
    {0x2e58da19, 0x8707b2d8, StenoChord(0x001a8000), 0, 8, 3},
    {0x2e58da19, 0x8707b2d8, StenoChord(0x007a8000), 0, 136, 3},
    {0x2f21896f, 0x8707b2d8, StenoChord(0x00004000), 0, 8, 3},
    {0x2f21896f, 0x8707b2d8, StenoChord(0x00204000), 0, 136, 3},
    {0x320ed901, 0x8707b2d8, StenoChord(0x00414000), 0, 8, 3},
    {0x326428a8, 0x326428a8, StenoChord(0x00000c00), 0, 6, 1},
    {0x32af1522, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x32af1522, 0x8707b2d8, StenoChord(0x00008000), 0, 8, 3},
    {0x32af1522, 0x8707b2d8, StenoChord(0x00208000), 0, 136, 3},
    {0x32af1522, 0x8707b2d8, StenoChord(0x00614000), 0, 136, 3},
    {0x32e49586, 0x8707b2d8, StenoChord(0x00180000), 0, 8, 3},
    {0x32e49586, 0x8707b2d8, StenoChord(0x00780000), 0, 136, 3},
    {0x3568a8c0, 0x8707b2d8, StenoChord(0x00404000), 0, 8, 3},
    {0x3568a8c0, 0x8707b2d8, StenoChord(0x00604000), 0, 136, 3},
    {0x35a6bdfe, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x35a6bdfe, 0x8707b2d8, StenoChord(0x00240000), 0, 136, 3},
    {0x37dd6e7b, 0x8707b2d8, StenoChord(0x00094000), 0, 8, 3},
    {0x37dd6e7b, 0x8707b2d8, StenoChord(0x00294000), 0, 136, 3},
    {0x386f360b, 0x8707b2d8, StenoChord(0x0008c000), 0, 8, 3},
    {0x386f360b, 0x8707b2d8, StenoChord(0x0028c000), 0, 136, 3},
    {0x3896e72c, 0x8707b2d8, StenoChord(0x0012c000), 0, 8, 3},
    {0x3896e72c, 0x8707b2d8, StenoChord(0x0052c000), 0, 136, 3},
    {0x390a7b63, 0x710e300b, StenoChord(0x000000ea), 0, 1, 1},
    {0x390a7b63, 0x710e300b, StenoChord(0x00000c00), 0, 1, 2},
    {0x3a35c33d, 0x0609009d, StenoChord(0x00000500), 0, 130, 1},
    {0x3a528d82, 0x8707b2d8, StenoChord(0x002a8000), 0, 136, 3},
    {0x3cb73648, 0x8707b2d8, StenoChord(0x000ac000), 0, 8, 3},
    {0x3cb73648, 0x8707b2d8, StenoChord(0x002ac000), 0, 136, 3},
    {0x3cca9f20, 0x8707b2d8, StenoChord(0x001a0000), 0, 8, 3},
    {0x3ceae297, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x3ceae297, 0x3ceae297, StenoChord(0x00000800), 0, 6, 1},
    {0x3ceae297, 0x3ceae297, StenoChord(0x00000800), 0, 4, 1},
    {0x3ceae297, 0x3ceae297, StenoChord(0x00000c00), 0, 4, 1},
    {0x3ceae297, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x3d5b5ba2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x3d5b5ba2, 0x8707b2d8, StenoChord(0x00128000), 0, 8, 3},
    {0x3d5b5ba2, 0x8707b2d8, StenoChord(0x00528000), 0, 136, 3},
    {0x3f30e086, 0x8707b2d8, StenoChord(0x00428000), 0, 8, 3},
    {0x3f30e086, 0x8707b2d8, StenoChord(0x00628000), 0, 136, 3},
    {0x3f31f002, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x3f31f002, 0x8707b2d8, StenoChord(0x00038000), 0, 8, 3},
    {0x3f31f002, 0x8707b2d8, StenoChord(0x00238000), 0, 136, 3},
    {0x3f3807c6, 0x0609009d, StenoChord(0x00000300), 0, 130, 1},
    {0x3f3807c6, 0x8707b2d8, StenoChord(0x00554000), 0, 136, 3},
    {0x410505e2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x410505e2, 0x8707b2d8, StenoChord(0x0001c000), 0, 8, 3},
    {0x426bbd50, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x4286c9ef, 0x8707b2d8, StenoChord(0x0009c000), 0, 8, 3},
    {0x4286c9ef, 0x8707b2d8, StenoChord(0x0029c000), 0, 136, 3},
    {0x429b1a21, 0x00000000, StenoChord(0x00000000), 1, 0, 1},
    {0x429b1a21, 0x0609009d, StenoChord(0x00000200), 0, 2, 1},
    {0x429b1a21, 0x8707b2d8, StenoChord(0x00034000), 0, 8, 3},
    {0x44630acd, 0x8707b2d8, StenoChord(0x0009c000), 0, 8, 3},
    {0x44ac63b8, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x44ac63b8, 0x8707b2d8, StenoChord(0x00290000), 0, 136, 3},
    {0x44be45f9, 0x44be45f9, StenoChord(0x00000c00), 0, 6, 1},
    {0x453f0832, 0x8707b2d8, StenoChord(0x00428000), 0, 8, 3},
    {0x456239f9, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x456239f9, 0x8707b2d8, StenoChord(0x00068000), 0, 8, 3},
    {0x456239f9, 0x8707b2d8, StenoChord(0x00268000), 0, 136, 3},
    {0x4686967c, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4686967c, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x4686967c, 0x8707b2d8, StenoChord(0x00210000), 0, 136, 3},
    {0x47599c71, 0x00000000, StenoChord(0x00000000), 1, 0, 1},
    {0x47599c71, 0x47599c71, StenoChord(0x00002400), 0, 6, 1},
    {0x48746d47, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x48746d47, 0x8707b2d8, StenoChord(0x00030000), 0, 8, 3},
    {0x49033525, 0x8707b2d8, StenoChord(0x000c4000), 0, 8, 3},
    {0x49033525, 0x8707b2d8, StenoChord(0x002c4000), 0, 136, 3},
    {0x49ca4e7d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x49ca4e7d, 0x8707b2d8, StenoChord(0x00070000), 0, 8, 3},
    {0x49e22396, 0x8707b2d8, StenoChord(0x00440000), 0, 8, 3},
    {0x49e22396, 0x8707b2d8, StenoChord(0x00640000), 0, 136, 3},
    {0x4a962ee9, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4a962ee9, 0x8707b2d8, StenoChord(0x0010c000), 0, 8, 3},
    {0x4a962ee9, 0x8707b2d8, StenoChord(0x0050c000), 0, 136, 3},
    {0x4b300576, 0x8707b2d8, StenoChord(0x00024000), 0, 8, 3},
    {0x4b300576, 0x8707b2d8, StenoChord(0x00224000), 0, 136, 3},
    {0x4bf68519, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4bf68519, 0x8707b2d8, StenoChord(0x00140000), 0, 8, 3},
    {0x4cf73ae2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4cf73ae2, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x4cf73ae2, 0x8707b2d8, StenoChord(0x00210000), 0, 136, 3},
    {0x4d8049c1, 0x4d8049c1, StenoChord(0x00002c00), 0, 134, 1},
    {0x4d9a0d84, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4d9a0d84, 0x8707b2d8, StenoChord(0x00054000), 0, 8, 3},
    {0x4d9a0d84, 0x8707b2d8, StenoChord(0x00254000), 0, 136, 3},
    {0x4daeda87, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4daeda87, 0x8707b2d8, StenoChord(0x00244000), 0, 136, 3},
    {0x4de06265, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x4e7b11cb, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4e7b11cb, 0x8707b2d8, StenoChord(0x00038000), 0, 8, 3},
    {0x4e7b11cb, 0x8707b2d8, StenoChord(0x00238000), 0, 136, 3},
    {0x4e7fcf41, 0x0609009d, StenoChord(0x00000000), 0, 130, 1},
    {0x4e7fcf41, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x4e7fcf41, 0x8707b2d8, StenoChord(0x0020c000), 0, 136, 3},
    {0x4f91d53c, 0x8707b2d8, StenoChord(0x00014000), 0, 8, 3},
    {0x4f91d53c, 0x8707b2d8, StenoChord(0x00214000), 0, 136, 3},
    {0x5060b8c4, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x5060b8c4, 0x8707b2d8, StenoChord(0x00148000), 0, 8, 3},
    {0x5060b8c4, 0x8707b2d8, StenoChord(0x00548000), 0, 136, 3},
    {0x5076a4c0, 0x8707b2d8, StenoChord(0x00004000), 0, 8, 3},
    {0x5076a4c0, 0x8707b2d8, StenoChord(0x00204000), 0, 136, 3},
    {0x508edb76, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x508edb76, 0x8707b2d8, StenoChord(0x00078000), 0, 8, 3},
    {0x508edb76, 0x8707b2d8, StenoChord(0x00278000), 0, 136, 3},
    {0x508ffc5a, 0x8707b2d8, StenoChord(0x001c0000), 0, 8, 3},
    {0x5137067c, 0x0609009d, StenoChord(0x00000116), 0, 2, 2},
    {0x51469c33, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x51469c33, 0x8707b2d8, StenoChord(0x00290000), 0, 136, 3},
    {0x5216322e, 0x8707b2d8, StenoChord(0x000d4000), 0, 8, 3},
    {0x5216322e, 0x8707b2d8, StenoChord(0x002d4000), 0, 136, 3},
    {0x52b1c121, 0x8707b2d8, StenoChord(0x00184000), 0, 8, 3},
    {0x52b1c121, 0x8707b2d8, StenoChord(0x00784000), 0, 136, 3},
    {0x52fa60a8, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x52fa60a8, 0x8707b2d8, StenoChord(0x00058000), 0, 8, 3},
    {0x52fa60a8, 0x8707b2d8, StenoChord(0x00258000), 0, 136, 3},
    {0x530f2caf, 0x8707b2d8, StenoChord(0x00420000), 0, 8, 3},
    {0x530f2caf, 0x8707b2d8, StenoChord(0x00620000), 0, 136, 3},
    {0x5313f237, 0x00000000, StenoChord(0x00000000), 1, 0, 1},
    {0x5313f237, 0x5313f237, StenoChord(0x00002400), 0, 6, 1},
    {0x532743cd, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x532743cd, 0x8707b2d8, StenoChord(0x00048000), 0, 8, 3},
    {0x532743cd, 0x8707b2d8, StenoChord(0x00248000), 0, 136, 3},
    {0x534e6880, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x534e6880, 0x8707b2d8, StenoChord(0x00054000), 0, 8, 3},
    {0x534e6880, 0x8707b2d8, StenoChord(0x00254000), 0, 136, 3},
    {0x53da313c, 0x8707b2d8, StenoChord(0x000bc000), 0, 8, 3},
    {0x53da313c, 0x8707b2d8, StenoChord(0x002bc000), 0, 136, 3},
    {0x5468790f, 0x8707b2d8, StenoChord(0x000b0000), 0, 8, 3},
    {0x5468790f, 0x8707b2d8, StenoChord(0x002b0000), 0, 136, 3},
    {0x54ce8b8c, 0x0609009d, StenoChord(0x00000500), 0, 2, 1},
    {0x5559689d, 0x8707b2d8, StenoChord(0x000cc000), 0, 8, 3},
    {0x5559689d, 0x8707b2d8, StenoChord(0x002cc000), 0, 136, 3},
    {0x5647a3be, 0x5647a3be, StenoChord(0x00001800), 0, 6, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x00001800), 0, 4, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x000018a6), 0, 7, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x000018fe), 0, 7, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x00001c00), 0, 4, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x00001ca6), 0, 7, 1},
    {0x5647a3be, 0x5647a3be, StenoChord(0x00001cfe), 0, 7, 1},
    {0x567a7547, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x567a7547, 0x8707b2d8, StenoChord(0x00228000), 0, 136, 3},
    {0x5782501d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x5782501d, 0x8707b2d8, StenoChord(0x00060000), 0, 8, 3},
    {0x5782501d, 0x8707b2d8, StenoChord(0x00260000), 0, 136, 3},
    {0x578bb7bf, 0x8707b2d8, StenoChord(0x000e8000), 0, 8, 3},
    {0x578bb7bf, 0x8707b2d8, StenoChord(0x002e8000), 0, 136, 3},
    {0x5823daa0, 0x8707b2d8, StenoChord(0x000f8000), 0, 8, 3},
    {0x5823daa0, 0x8707b2d8, StenoChord(0x002f8000), 0, 136, 3},
    {0x58a6ad82, 0x8707b2d8, StenoChord(0x000b8000), 0, 8, 3},
    {0x58a6ad82, 0x8707b2d8, StenoChord(0x002b8000), 0, 136, 3},
    {0x59c13249, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x59c13249, 0x8707b2d8, StenoChord(0x00218000), 0, 136, 3},
    {0x5a2fe996, 0x710e300b, StenoChord(0x000000c6), 0, 1, 1},
    {0x5a2fe996, 0x710e300b, StenoChord(0x000000d6), 0, 1, 1},
    {0x5b754b18, 0x8707b2d8, StenoChord(0x0018c000), 0, 8, 3},
    {0x5b754b18, 0x8707b2d8, StenoChord(0x0078c000), 0, 136, 3},
    {0x5ba8d55b, 0x8707b2d8, StenoChord(0x000bc000), 0, 8, 3},
    {0x5d347e5e, 0x8707b2d8, StenoChord(0x00420000), 0, 8, 3},
    {0x5d9a8b18, 0x8707b2d8, StenoChord(0x001a0000), 0, 8, 3},
    {0x5d9a8b18, 0x8707b2d8, StenoChord(0x007a0000), 0, 136, 3},
    {0x5edc26af, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x5edc26af, 0x8707b2d8, StenoChord(0x00114000), 0, 8, 3},
    {0x5edc26af, 0x8707b2d8, StenoChord(0x00514000), 0, 136, 3},
    {0x5ee563df, 0x8707b2d8, StenoChord(0x0019c000), 0, 8, 3},
    {0x605afb1e, 0x8707b2d8, StenoChord(0x00088000), 0, 8, 3},
    {0x605afb1e, 0x8707b2d8, StenoChord(0x00288000), 0, 136, 3},
    {0x6113a845, 0x8707b2d8, StenoChord(0x00404000), 0, 8, 3},
    {0x6113a845, 0x8707b2d8, StenoChord(0x00604000), 0, 136, 3},
    {0x6156e822, 0x8707b2d8, StenoChord(0x000d0000), 0, 8, 3},
    {0x6156e822, 0x8707b2d8, StenoChord(0x002d0000), 0, 136, 3},
    {0x62bfcab3, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x62bfcab3, 0x8707b2d8, StenoChord(0x000a0000), 0, 8, 3},
    {0x62bfcab3, 0x8707b2d8, StenoChord(0x002a0000), 0, 136, 3},
    {0x633cbad2, 0x0609009d, StenoChord(0x00000100), 0, 2, 1},
    {0x633cbad2, 0x8707b2d8, StenoChord(0x00150000), 0, 8, 3},
    {0x64e2a26f, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x64e2a26f, 0x8707b2d8, StenoChord(0x0001c000), 0, 8, 3},
    {0x64e2a26f, 0x8707b2d8, StenoChord(0x0021c000), 0, 136, 3},
    {0x6508f284, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x6508f284, 0x8707b2d8, StenoChord(0x00044000), 0, 8, 3},
    {0x65fe1937, 0x8707b2d8, StenoChord(0x000bc000), 0, 8, 3},
    {0x65fe1937, 0x8707b2d8, StenoChord(0x002bc000), 0, 136, 3},
    {0x66002635, 0x66002635, StenoChord(0x00002800), 0, 132, 1},
    {0x66002635, 0x66002635, StenoChord(0x00002800), 0, 6, 1},
    {0x66002635, 0x66002635, StenoChord(0x00002800), 0, 4, 1},
    {0x66002635, 0x66002635, StenoChord(0x00002c00), 0, 4, 1},
    {0x66002635, 0x66002635, StenoChord(0x00002c00), 0, 132, 1},
    {0x666b7816, 0x710e300b, StenoChord(0x000000b8), 0, 1, 1},
    {0x666b7816, 0x710e300b, StenoChord(0x00001000), 0, 1, 2},
    {0x680e9c5c, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x680e9c5c, 0x8707b2d8, StenoChord(0x0011c000), 0, 8, 3},
    {0x680e9c5c, 0x8707b2d8, StenoChord(0x0051c000), 0, 136, 3},
    {0x6826eae0, 0x8707b2d8, StenoChord(0x00014000), 0, 8, 3},
    {0x6826eae0, 0x8707b2d8, StenoChord(0x00214000), 0, 136, 3},
    {0x6a2996a3, 0x6a2996a3, StenoChord(0x00002800), 0, 6, 1},
    {0x6a2996a3, 0x6a2996a3, StenoChord(0x00002800), 0, 4, 1},
    {0x6a2996a3, 0x6a2996a3, StenoChord(0x00002c00), 0, 4, 1},
    {0x6a59beee, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x6a59beee, 0x8707b2d8, StenoChord(0x0004c000), 0, 8, 3},
    {0x6b660a33, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x6b660a33, 0x8707b2d8, StenoChord(0x00054000), 0, 8, 3},
    {0x6b660a33, 0x8707b2d8, StenoChord(0x00254000), 0, 136, 3},
    {0x6c3bf144, 0x8707b2d8, StenoChord(0x00414000), 0, 8, 3},
    {0x6e357c4a, 0x8707b2d8, StenoChord(0x00088000), 0, 8, 3},
    {0x6e357c4a, 0x8707b2d8, StenoChord(0x00288000), 0, 136, 3},
    {0x6e786c89, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x6e786c89, 0x8707b2d8, StenoChord(0x00148000), 0, 8, 3},
    {0x6e786c89, 0x8707b2d8, StenoChord(0x00548000), 0, 136, 3},
    {0x70718ee4, 0x8707b2d8, StenoChord(0x001a8000), 0, 8, 3},
    {0x70f8efb0, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x70f8efb0, 0x8707b2d8, StenoChord(0x00038000), 0, 8, 3},
    {0x70f8efb0, 0x8707b2d8, StenoChord(0x00238000), 0, 136, 3},
    {0x73044310, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x73044310, 0x8707b2d8, StenoChord(0x00128000), 0, 8, 3},
    {0x73044310, 0x8707b2d8, StenoChord(0x00528000), 0, 136, 3},
    {0x740e219e, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x740e219e, 0x8707b2d8, StenoChord(0x00290000), 0, 136, 3},
    {0x7499292d, 0x8707b2d8, StenoChord(0x000d0000), 0, 8, 3},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x00003000), 0, 4, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x00003000), 0, 6, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x000030a6), 0, 7, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x000030fe), 0, 7, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x00003400), 0, 4, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x000034a6), 0, 7, 1},
    {0x749ffdd0, 0x749ffdd0, StenoChord(0x000034fe), 0, 7, 1},
    {0x74faa480, 0x74faa480, StenoChord(0x00002800), 0, 132, 1},
    {0x74faa480, 0x74faa480, StenoChord(0x00002800), 0, 134, 1},
    {0x74faa480, 0x74faa480, StenoChord(0x00002c00), 0, 132, 1},
    {0x756ac9d5, 0x0609009d, StenoChord(0x00000600), 0, 2, 1},
    {0x75abac61, 0x8707b2d8, StenoChord(0x00194000), 0, 8, 3},
    {0x75abac61, 0x8707b2d8, StenoChord(0x00794000), 0, 136, 3},
    {0x77c63782, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x77c63782, 0x8707b2d8, StenoChord(0x00048000), 0, 8, 3},
    {0x77c63782, 0x8707b2d8, StenoChord(0x00248000), 0, 136, 3},
    {0x785f2182, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x785f2182, 0x8707b2d8, StenoChord(0x00060000), 0, 8, 3},
    {0x789b5ca3, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x789b5ca3, 0x8707b2d8, StenoChord(0x0002c000), 0, 8, 3},
    {0x79476318, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x79476318, 0x8707b2d8, StenoChord(0x00400000), 0, 8, 3},
    {0x79476318, 0x8707b2d8, StenoChord(0x00600000), 0, 136, 3},
    {0x7978904a, 0x8707b2d8, StenoChord(0x0019c000), 0, 8, 3},
    {0x7978904a, 0x8707b2d8, StenoChord(0x0079c000), 0, 136, 3},
    {0x79bbdc48, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x79bbdc48, 0x8707b2d8, StenoChord(0x0011c000), 0, 8, 3},
    {0x7a218af1, 0x8707b2d8, StenoChord(0x00298000), 0, 136, 3},
    {0x7a67e768, 0x8707b2d8, StenoChord(0x00460000), 0, 8, 3},
    {0x7a67e768, 0x8707b2d8, StenoChord(0x00660000), 0, 136, 3},
    {0x7ac61e02, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x7ac61e02, 0x8707b2d8, StenoChord(0x00018000), 0, 8, 3},
    {0x7ac61e02, 0x8707b2d8, StenoChord(0x00218000), 0, 136, 3},
    {0x7b2cc683, 0x8707b2d8, StenoChord(0x00420000), 0, 8, 3},
    {0x7b2cc683, 0x8707b2d8, StenoChord(0x00620000), 0, 136, 3},
    {0x7b6c0fed, 0x8707b2d8, StenoChord(0x001c0000), 0, 8, 3},
    {0x7b6c0fed, 0x8707b2d8, StenoChord(0x007c0000), 0, 136, 3},
    {0x7c796918, 0x8707b2d8, StenoChord(0x00408000), 0, 8, 3},
    {0x7c796918, 0x8707b2d8, StenoChord(0x00608000), 0, 136, 3},
    {0x7d5bd382, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x7d5bd382, 0x8707b2d8, StenoChord(0x00048000), 0, 8, 3},
    {0x7d5bd382, 0x8707b2d8, StenoChord(0x00248000), 0, 136, 3},
    {0x7dbcdfb9, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x7dbcdfb9, 0x8707b2d8, StenoChord(0x00068000), 0, 8, 3},
    {0x7ed87395, 0x8707b2d8, StenoChord(0x00204000), 0, 136, 3},
    {0x7f11e740, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x7f11e740, 0x8707b2d8, StenoChord(0x00058000), 0, 8, 3},
    {0x7f11e740, 0x8707b2d8, StenoChord(0x00258000), 0, 136, 3},
    {0x803a7b1f, 0x8707b2d8, StenoChord(0x00004000), 0, 8, 3},
    {0x80ae03d9, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x80ae03d9, 0x8707b2d8, StenoChord(0x00124000), 0, 8, 3},
    {0x80ae03d9, 0x8707b2d8, StenoChord(0x00524000), 0, 136, 3},
    {0x80aebb80, 0x8707b2d8, StenoChord(0x000e8000), 0, 8, 3},
    {0x80aebb80, 0x8707b2d8, StenoChord(0x002e8000), 0, 136, 3},
    {0x80d6d70e, 0x8707b2d8, StenoChord(0x002c0000), 0, 136, 3},
    {0x81063908, 0x8707b2d8, StenoChord(0x001a4000), 0, 8, 3},
    {0x81063908, 0x8707b2d8, StenoChord(0x007a4000), 0, 136, 3},
    {0x82225168, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x82225168, 0x8707b2d8, StenoChord(0x00050000), 0, 8, 3},
    {0x82225168, 0x8707b2d8, StenoChord(0x00250000), 0, 136, 3},
    {0x840eabae, 0x8707b2d8, StenoChord(0x00440000), 0, 8, 3},
    {0x840eabae, 0x8707b2d8, StenoChord(0x00640000), 0, 136, 3},
    {0x8429f971, 0x8707b2d8, StenoChord(0x00188000), 0, 8, 3},
    {0x843cba45, 0x8707b2d8, StenoChord(0x00064000), 0, 8, 3},
    {0x843cba45, 0x8707b2d8, StenoChord(0x00264000), 0, 136, 3},
    {0x85216679, 0x85216679, StenoChord(0x00002c00), 0, 6, 1},
    {0x8701804a, 0x8707b2d8, StenoChord(0x000a4000), 0, 8, 3},
    {0x87ae0156, 0x8707b2d8, StenoChord(0x0018c000), 0, 8, 3},
    {0x87ae0156, 0x8707b2d8, StenoChord(0x0078c000), 0, 136, 3},
    {0x8952fbf0, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x8952fbf0, 0x8707b2d8, StenoChord(0x0010c000), 0, 8, 3},
    {0x8a1dfa16, 0x8707b2d8, StenoChord(0x00640000), 0, 136, 3},
    {0x8a97f357, 0x00000000, StenoChord(0x00000000), 1, 0, 1},
    {0x8a97f357, 0x8a97f357, StenoChord(0x00002400), 0, 134, 1},
    {0x8b4ba9e6, 0x8707b2d8, StenoChord(0x000c8000), 0, 8, 3},
    {0x8b4ba9e6, 0x8707b2d8, StenoChord(0x002c8000), 0, 136, 3},
    {0x8c17e6cb, 0x8707b2d8, StenoChord(0x00158000), 0, 8, 3},
    {0x8ca4052e, 0x8707b2d8, StenoChord(0x00100000), 0, 8, 3},
    {0x8ca4052e, 0x8707b2d8, StenoChord(0x00500000), 0, 136, 3},
    {0x8eba60cc, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x8eba60cc, 0x8707b2d8, StenoChord(0x0010c000), 0, 8, 3},
    {0x8eba60cc, 0x8707b2d8, StenoChord(0x0050c000), 0, 136, 3},
    {0x8fbcbc2d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoChord(0x00002000), 0, 132, 2},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoChord(0x00002000), 0, 134, 1},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoChord(0x00002000), 0, 132, 1},
    {0x8fbcbc2d, 0x8fbcbc2d, StenoChord(0x00002400), 0, 132, 1},
    {0x8fbcbc2d, 0x8707b2d8, StenoChord(0x00080000), 0, 8, 3},
    {0x8fbcbc2d, 0x8707b2d8, StenoChord(0x00280000), 0, 136, 3},
    {0x8ff6cc98, 0x8707b2d8, StenoChord(0x00190000), 0, 8, 3},
    {0x914d2415, 0x8707b2d8, StenoChord(0x001a8000), 0, 8, 3},
    {0x914d2415, 0x8707b2d8, StenoChord(0x007a8000), 0, 136, 3},
    {0x91bb9f6c, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x91bb9f6c, 0x8707b2d8, StenoChord(0x00058000), 0, 8, 3},
    {0x91bb9f6c, 0x8707b2d8, StenoChord(0x00258000), 0, 136, 3},
    {0x92f07f27, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x92f07f27, 0x8707b2d8, StenoChord(0x000a0000), 0, 8, 3},
    {0x9301d5fc, 0x8707b2d8, StenoChord(0x000dc000), 0, 8, 3},
    {0x9301d5fc, 0x8707b2d8, StenoChord(0x002dc000), 0, 136, 3},
    {0x9350acf9, 0x8707b2d8, StenoChord(0x00180000), 0, 8, 3},
    {0x9350acf9, 0x8707b2d8, StenoChord(0x00780000), 0, 136, 3},
    {0x93ff711d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x93ff711d, 0x8707b2d8, StenoChord(0x0002c000), 0, 8, 3},
    {0x93ff711d, 0x8707b2d8, StenoChord(0x0022c000), 0, 136, 3},
    {0x948d45d1, 0x8707b2d8, StenoChord(0x00130000), 0, 8, 3},
    {0x94b1cc4b, 0x8707b2d8, StenoChord(0x00400000), 0, 8, 3},
    {0x94b1cc4b, 0x8707b2d8, StenoChord(0x00600000), 0, 136, 3},
    {0x94e4ac3b, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x94e4ac3b, 0x8707b2d8, StenoChord(0x0004c000), 0, 8, 3},
    {0x94e4ac3b, 0x8707b2d8, StenoChord(0x0024c000), 0, 136, 3},
    {0x95989bc5, 0x8707b2d8, StenoChord(0x000c8000), 0, 8, 3},
    {0x96445b9d, 0x8707b2d8, StenoChord(0x000d8000), 0, 8, 3},
    {0x96445b9d, 0x8707b2d8, StenoChord(0x002d8000), 0, 136, 3},
    {0x9685f6a6, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9685f6a6, 0x9685f6a6, StenoChord(0x00000800), 0, 4, 1},
    {0x9685f6a6, 0x9685f6a6, StenoChord(0x00000800), 0, 6, 1},
    {0x9685f6a6, 0x9685f6a6, StenoChord(0x00000c00), 0, 4, 1},
    {0x9685f6a6, 0x8707b2d8, StenoChord(0x00010000), 0, 8, 3},
    {0x96bb243e, 0x8707b2d8, StenoChord(0x0018c000), 0, 8, 3},
    {0x98574167, 0x8707b2d8, StenoChord(0x00104000), 0, 8, 3},
    {0x98574167, 0x8707b2d8, StenoChord(0x00504000), 0, 136, 3},
    {0x9869cb72, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9869cb72, 0x8707b2d8, StenoChord(0x00084000), 0, 8, 3},
    {0x9869cb72, 0x8707b2d8, StenoChord(0x00284000), 0, 136, 3},
    {0x9a1e494f, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9a1e494f, 0x8707b2d8, StenoChord(0x00108000), 0, 8, 3},
    {0x9adfc86f, 0x8707b2d8, StenoChord(0x000cc000), 0, 8, 3},
    {0x9b3d4b2f, 0x8707b2d8, StenoChord(0x00024000), 0, 8, 3},
    {0x9b3d4b2f, 0x8707b2d8, StenoChord(0x00224000), 0, 136, 3},
    {0x9b71cf23, 0x8707b2d8, StenoChord(0x001a8000), 0, 8, 3},
    {0x9b71cf23, 0x8707b2d8, StenoChord(0x007a8000), 0, 136, 3},
    {0x9bb080d0, 0x8707b2d8, StenoChord(0x00460000), 0, 8, 3},
    {0x9bb080d0, 0x8707b2d8, StenoChord(0x00660000), 0, 136, 3},
    {0x9c428b46, 0x8707b2d8, StenoChord(0x00194000), 0, 8, 3},
    {0x9c428b46, 0x8707b2d8, StenoChord(0x00794000), 0, 136, 3},
    {0x9cf70218, 0x8707b2d8, StenoChord(0x00120000), 0, 8, 3},
    {0x9d45c095, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9d45c095, 0x0609009d, StenoChord(0x00000000), 0, 2, 1},
    {0x9d45c095, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9d45c095, 0x8707b2d8, StenoChord(0x0000c000), 0, 8, 3},
    {0x9d45c095, 0x8707b2d8, StenoChord(0x0020c000), 0, 136, 3},
    {0x9d46ad5f, 0x8707b2d8, StenoChord(0x00460000), 0, 8, 3},
    {0x9d546aa1, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9d546aa1, 0x8707b2d8, StenoChord(0x0003c000), 0, 8, 3},
    {0x9d546aa1, 0x8707b2d8, StenoChord(0x0023c000), 0, 136, 3},
    {0x9dc696ce, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9dc696ce, 0x8707b2d8, StenoChord(0x00084000), 0, 8, 3},
    {0x9e0aec7b, 0x8707b2d8, StenoChord(0x00404000), 0, 8, 3},
    {0x9e0aec7b, 0x8707b2d8, StenoChord(0x00604000), 0, 136, 3},
    {0x9e3c5b8b, 0x8707b2d8, StenoChord(0x000d8000), 0, 8, 3},
    {0x9e773d63, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9e773d63, 0x8707b2d8, StenoChord(0x00148000), 0, 8, 3},
    {0x9eecc4f0, 0x8707b2d8, StenoChord(0x00090000), 0, 8, 3},
    {0x9f370ce5, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0x9f370ce5, 0x8707b2d8, StenoChord(0x0002c000), 0, 8, 3},
    {0x9f370ce5, 0x8707b2d8, StenoChord(0x0022c000), 0, 136, 3},
    {0x9f3fe7fd, 0x8707b2d8, StenoChord(0x000c4000), 0, 8, 3},
    {0x9f74b898, 0x8707b2d8, StenoChord(0x00130000), 0, 8, 3},
    {0x9f74b898, 0x8707b2d8, StenoChord(0x00530000), 0, 136, 3},
    {0x9f7fae07, 0x8707b2d8, StenoChord(0x00184000), 0, 8, 3},
    {0x9fee2525, 0x8707b2d8, StenoChord(0x000a4000), 0, 8, 3},
    {0x9fee2525, 0x8707b2d8, StenoChord(0x002a4000), 0, 136, 3},
    {0xa0047729, 0x8707b2d8, StenoChord(0x00020000), 0, 8, 3},
    {0xa0047729, 0x8707b2d8, StenoChord(0x00220000), 0, 136, 3},
    {0xa2232b07, 0x8707b2d8, StenoChord(0x00024000), 0, 8, 3},
    {0xa2786530, 0x8707b2d8, StenoChord(0x000b0000), 0, 8, 3},
    {0xa2786530, 0x8707b2d8, StenoChord(0x002b0000), 0, 136, 3},
    {0xa28c83e9, 0x8707b2d8, StenoChord(0x000bc000), 0, 8, 3},
    {0xa28c83e9, 0x8707b2d8, StenoChord(0x002bc000), 0, 136, 3},
    {0xa28e7734, 0x710e300b, StenoChord(0x00000078), 0, 1, 1},
    {0xa28e7734, 0x710e300b, StenoChord(0x00000400), 0, 1, 2},
    {0xa32edac3, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xa32edac3, 0x8707b2d8, StenoChord(0x00018000), 0, 8, 3},
    {0xa32edac3, 0x8707b2d8, StenoChord(0x00218000), 0, 136, 3},
    {0xa407938f, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xa407938f, 0x8707b2d8, StenoChord(0x00018000), 0, 8, 3},
    {0xa407938f, 0x8707b2d8, StenoChord(0x00218000), 0, 136, 3},
    {0xa4520a18, 0x8707b2d8, StenoChord(0x00100000), 0, 8, 3},
    {0xa4520a18, 0x8707b2d8, StenoChord(0x00500000), 0, 136, 3},
    {0xa50957cb, 0x8707b2d8, StenoChord(0x001c8000), 0, 8, 3},
    {0xa50957cb, 0x8707b2d8, StenoChord(0x007c8000), 0, 136, 3},
    {0xa5126bcf, 0x8707b2d8, StenoChord(0x000e8000), 0, 8, 3},
    {0xa5126bcf, 0x8707b2d8, StenoChord(0x002e8000), 0, 136, 3},
    {0xa5f91580, 0x8707b2d8, StenoChord(0x00408000), 0, 8, 3},
    {0xa5f91580, 0x8707b2d8, StenoChord(0x00608000), 0, 136, 3},
    {0xa606c573, 0x8707b2d8, StenoChord(0x00074000), 0, 8, 3},
    {0xa606c573, 0x8707b2d8, StenoChord(0x00274000), 0, 136, 3},
    {0xa67b83b8, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xa67b83b8, 0x8707b2d8, StenoChord(0x0000c000), 0, 8, 3},
    {0xa67b83b8, 0x8707b2d8, StenoChord(0x0020c000), 0, 136, 3},
    {0xa6d78bd1, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xa6d78bd1, 0x8707b2d8, StenoChord(0x00050000), 0, 8, 3},
    {0xa6d78bd1, 0x8707b2d8, StenoChord(0x00250000), 0, 136, 3},
    {0xa7191336, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xa7191336, 0x8707b2d8, StenoChord(0x00078000), 0, 8, 3},
    {0xa7191336, 0x8707b2d8, StenoChord(0x00278000), 0, 136, 3},
    {0xa8539d8c, 0x710e300b, StenoChord(0x00000066), 0, 1, 1},
    {0xa8539d8c, 0x0609009d, StenoChord(0x00000146), 0, 2, 2},
    {0xa9eb7789, 0x8707b2d8, StenoChord(0x0012c000), 0, 8, 3},
    {0xa9eb7789, 0x8707b2d8, StenoChord(0x0052c000), 0, 136, 3},
    {0xaa583684, 0x0609009d, StenoChord(0x00000100), 0, 130, 1},
    {0xaa583684, 0x8707b2d8, StenoChord(0x00550000), 0, 136, 3},
    {0xac49a6db, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xac49a6db, 0x8707b2d8, StenoChord(0x00250000), 0, 136, 3},
    {0xac6340b3, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xac6340b3, 0x8707b2d8, StenoChord(0x00070000), 0, 8, 3},
    {0xac6340b3, 0x8707b2d8, StenoChord(0x00270000), 0, 136, 3},
    {0xac94610e, 0x8707b2d8, StenoChord(0x000c0000), 0, 8, 3},
    {0xac94fac6, 0xac94fac6, StenoChord(0x00000c00), 0, 134, 1},
    {0xad4aa09e, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xad4aa09e, 0x8707b2d8, StenoChord(0x00108000), 0, 8, 3},
    {0xad4aa09e, 0x8707b2d8, StenoChord(0x00508000), 0, 136, 3},
    {0xae36585c, 0x8707b2d8, StenoChord(0x000c0000), 0, 8, 3},
    {0xae36585c, 0x8707b2d8, StenoChord(0x002c0000), 0, 136, 3},
    {0xae9089d4, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xae9089d4, 0x8707b2d8, StenoChord(0x00108000), 0, 8, 3},
    {0xae9089d4, 0x8707b2d8, StenoChord(0x00508000), 0, 136, 3},
    {0xb0339683, 0x8707b2d8, StenoChord(0x00020000), 0, 8, 3},
    {0xb0339683, 0x8707b2d8, StenoChord(0x00220000), 0, 136, 3},
    {0xb04be33a, 0x8707b2d8, StenoChord(0x000f8000), 0, 8, 3},
    {0xb0fa6411, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb0fa6411, 0x8707b2d8, StenoChord(0x00044000), 0, 8, 3},
    {0xb0fa6411, 0x8707b2d8, StenoChord(0x00244000), 0, 136, 3},
    {0xb11e4f22, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb11e4f22, 0x8707b2d8, StenoChord(0x0003c000), 0, 8, 3},
    {0xb28860cb, 0x8707b2d8, StenoChord(0x000f0000), 0, 8, 3},
    {0xb28860cb, 0x8707b2d8, StenoChord(0x002f0000), 0, 136, 3},
    {0xb28e2435, 0x710e300b, StenoChord(0x00000064), 0, 1, 1},
    {0xb28e2435, 0x710e300b, StenoChord(0x00001400), 0, 1, 2},
    {0xb41668c7, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb41668c7, 0x8707b2d8, StenoChord(0x00114000), 0, 8, 3},
    {0xb41668c7, 0x8707b2d8, StenoChord(0x00514000), 0, 136, 3},
    {0xb42b78da, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb42b78da, 0x8707b2d8, StenoChord(0x00068000), 0, 8, 3},
    {0xb42b78da, 0x8707b2d8, StenoChord(0x00268000), 0, 136, 3},
    {0xb495d73d, 0x8707b2d8, StenoChord(0x00188000), 0, 8, 3},
    {0xb495d73d, 0x8707b2d8, StenoChord(0x00788000), 0, 136, 3},
    {0xb521e95a, 0x8707b2d8, StenoChord(0x000c4000), 0, 8, 3},
    {0xb521e95a, 0x8707b2d8, StenoChord(0x002c4000), 0, 136, 3},
    {0xb5d92c3f, 0xb5d92c3f, StenoChord(0x00000c00), 0, 134, 1},
    {0xb6689356, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb6689356, 0x8707b2d8, StenoChord(0x00040000), 0, 8, 3},
    {0xb6689356, 0x8707b2d8, StenoChord(0x00240000), 0, 136, 3},
    {0xb741320d, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb741320d, 0x8707b2d8, StenoChord(0x00044000), 0, 8, 3},
    {0xb741320d, 0x8707b2d8, StenoChord(0x00244000), 0, 136, 3},
    {0xb8cb2493, 0x8707b2d8, StenoChord(0x001c8000), 0, 8, 3},
    {0xb8cb2493, 0x8707b2d8, StenoChord(0x007c8000), 0, 136, 3},
    {0xb961c051, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xb961c051, 0xb961c051, StenoChord(0x00000800), 0, 132, 1},
    {0xb961c051, 0xb961c051, StenoChord(0x00000800), 0, 134, 1},
    {0xb961c051, 0xb961c051, StenoChord(0x00000c00), 0, 132, 1},
    {0xb961c051, 0x8707b2d8, StenoChord(0x00210000), 0, 136, 3},
    {0xba441fc2, 0x8707b2d8, StenoChord(0x000c8000), 0, 8, 3},
    {0xba441fc2, 0x8707b2d8, StenoChord(0x002c8000), 0, 136, 3},
    {0xba745a27, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xba745a27, 0x8707b2d8, StenoChord(0x00140000), 0, 8, 3},
    {0xba745a27, 0x8707b2d8, StenoChord(0x00540000), 0, 136, 3},
    {0xbae63262, 0x0609009d, StenoChord(0x00000162), 0, 2, 2},
    {0xbb255e97, 0x8707b2d8, StenoChord(0x001a0000), 0, 8, 3},
    {0xbb255e97, 0x8707b2d8, StenoChord(0x007a0000), 0, 136, 3},
    {0xbb48838e, 0x8707b2d8, StenoChord(0x00098000), 0, 8, 3},
    {0xbb6c0b99, 0x8707b2d8, StenoChord(0x00014000), 0, 8, 3},
    {0xbbc159ec, 0x8707b2d8, StenoChord(0x000e8000), 0, 8, 3},
    {0xbe97ab72, 0x8707b2d8, StenoChord(0x00294000), 0, 136, 3},
    {0xbfafd76c, 0x8707b2d8, StenoChord(0x001c0000), 0, 8, 3},
    {0xbfafd76c, 0x8707b2d8, StenoChord(0x007c0000), 0, 136, 3},
    {0xbfe8d608, 0x8707b2d8, StenoChord(0x00094000), 0, 8, 3},
    {0xbfe8d608, 0x8707b2d8, StenoChord(0x00294000), 0, 136, 3},
    {0xc11afc41, 0x8707b2d8, StenoChord(0x00104000), 0, 8, 3},
    {0xc11afc41, 0x8707b2d8, StenoChord(0x00504000), 0, 136, 3},
    {0xc1389bb2, 0x8707b2d8, StenoChord(0x001a0000), 0, 8, 3},
    {0xc1389bb2, 0x8707b2d8, StenoChord(0x007a0000), 0, 136, 3},
    {0xc2c4fcec, 0x0609009d, StenoChord(0x00000700), 0, 130, 1},
    {0xc3325b66, 0x8707b2d8, StenoChord(0x0019c000), 0, 8, 3},
    {0xc3325b66, 0x8707b2d8, StenoChord(0x0079c000), 0, 136, 3},
    {0xc3511eb1, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc3511eb1, 0x8707b2d8, StenoChord(0x00058000), 0, 8, 3},
    {0xc3551be2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc3551be2, 0x8707b2d8, StenoChord(0x00044000), 0, 8, 3},
    {0xc3551be2, 0x8707b2d8, StenoChord(0x00244000), 0, 136, 3},
    {0xc39d69da, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc39d69da, 0x8707b2d8, StenoChord(0x0004c000), 0, 8, 3},
    {0xc39d69da, 0x8707b2d8, StenoChord(0x0024c000), 0, 136, 3},
    {0xc3e99418, 0x8707b2d8, StenoChord(0x000c0000), 0, 8, 3},
    {0xc3e99418, 0x8707b2d8, StenoChord(0x002c0000), 0, 136, 3},
    {0xc40e62d7, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc40e62d7, 0x8707b2d8, StenoChord(0x00124000), 0, 8, 3},
    {0xc40e62d7, 0x8707b2d8, StenoChord(0x00524000), 0, 136, 3},
    {0xc4a8c9d9, 0x8707b2d8, StenoChord(0x000e0000), 0, 8, 3},
    {0xc55b700d, 0x8707b2d8, StenoChord(0x000d8000), 0, 8, 3},
    {0xc55b700d, 0x8707b2d8, StenoChord(0x002d8000), 0, 136, 3},
    {0xc571953b, 0x8707b2d8, StenoChord(0x000b8000), 0, 8, 3},
    {0xc633348f, 0x8707b2d8, StenoChord(0x000b0000), 0, 8, 3},
    {0xc6ec8df5, 0x8707b2d8, StenoChord(0x00428000), 0, 8, 3},
    {0xc6ec8df5, 0x8707b2d8, StenoChord(0x00628000), 0, 136, 3},
    {0xc75466c1, 0x8707b2d8, StenoChord(0x00100000), 0, 8, 3},
    {0xc8406cb1, 0x8707b2d8, StenoChord(0x00104000), 0, 8, 3},
    {0xc9317cf2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc9317cf2, 0x8707b2d8, StenoChord(0x00030000), 0, 8, 3},
    {0xc9317cf2, 0x8707b2d8, StenoChord(0x00230000), 0, 136, 3},
    {0xc9ae6404, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xc9ae6404, 0x8707b2d8, StenoChord(0x00078000), 0, 8, 3},
    {0xc9ae6404, 0x8707b2d8, StenoChord(0x00278000), 0, 136, 3},
    {0xca19cbba, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xca19cbba, 0x8707b2d8, StenoChord(0x00070000), 0, 8, 3},
    {0xca19cbba, 0x8707b2d8, StenoChord(0x00270000), 0, 136, 3},
    {0xcba7e880, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xcba7e880, 0x8707b2d8, StenoChord(0x00030000), 0, 8, 3},
    {0xcba7e880, 0x8707b2d8, StenoChord(0x00230000), 0, 136, 3},
    {0xcbe29615, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xcbe29615, 0x8707b2d8, StenoChord(0x0002c000), 0, 8, 3},
    {0xcbe29615, 0x8707b2d8, StenoChord(0x0022c000), 0, 136, 3},
    {0xcbf0480b, 0x8707b2d8, StenoChord(0x00158000), 0, 8, 3},
    {0xcbf0480b, 0x8707b2d8, StenoChord(0x00558000), 0, 136, 3},
    {0xcc2b13c3, 0x8707b2d8, StenoChord(0x000d4000), 0, 8, 3},
    {0xcc2b13c3, 0x8707b2d8, StenoChord(0x002d4000), 0, 136, 3},
    {0xcc8e2f3e, 0x8707b2d8, StenoChord(0x00074000), 0, 8, 3},
    {0xcc8e2f3e, 0x8707b2d8, StenoChord(0x00274000), 0, 136, 3},
    {0xcd2578ed, 0x0609009d, StenoChord(0x00000400), 0, 130, 1},
    {0xce5c21fa, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xce5c21fa, 0x8707b2d8, StenoChord(0x00008000), 0, 8, 3},
    {0xcf05a4c2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xcf05a4c2, 0x8707b2d8, StenoChord(0x0011c000), 0, 8, 3},
    {0xcf05a4c2, 0x8707b2d8, StenoChord(0x0051c000), 0, 136, 3},
    {0xcf401f72, 0xcf401f72, StenoChord(0x00003c00), 0, 4, 1},
    {0xcf401f72, 0xcf401f72, StenoChord(0x00003ca6), 0, 7, 1},
    {0xcf401f72, 0xcf401f72, StenoChord(0x00003cfe), 0, 7, 1},
    {0xcfcd5811, 0x8707b2d8, StenoChord(0x00194000), 0, 8, 3},
    {0xcfcd5811, 0x8707b2d8, StenoChord(0x00794000), 0, 136, 3},
    {0xd01f8468, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd01f8468, 0x8707b2d8, StenoChord(0x00038000), 0, 8, 3},
    {0xd0cdcf54, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd0cdcf54, 0x8707b2d8, StenoChord(0x00080000), 0, 8, 3},
    {0xd0cdcf54, 0x8707b2d8, StenoChord(0x00280000), 0, 136, 3},
    {0xd1256687, 0x710e300b, StenoChord(0x000000e8), 0, 1, 1},
    {0xd1256687, 0x710e300b, StenoChord(0x00000800), 0, 1, 2},
    {0xd2109e05, 0xd2109e05, StenoChord(0x000014a6), 0, 7, 1},
    {0xd2109e05, 0xd2109e05, StenoChord(0x000014fe), 0, 7, 1},
    {0xd27eea40, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002000), 0, 6, 1},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002000), 0, 4, 2},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002000), 0, 132, 1},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002000), 0, 4, 1},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002400), 0, 132, 1},
    {0xd27eea40, 0xd27eea40, StenoChord(0x00002400), 0, 4, 1},
    {0xd27eea40, 0x8707b2d8, StenoChord(0x00080000), 0, 8, 3},
    {0xd27eea40, 0x8707b2d8, StenoChord(0x00280000), 0, 136, 3},
    {0xd5fe8280, 0x8707b2d8, StenoChord(0x000f0000), 0, 8, 3},
    {0xd5fe8280, 0x8707b2d8, StenoChord(0x002f0000), 0, 136, 3},
    {0xd62c7057, 0x8707b2d8, StenoChord(0x00290000), 0, 136, 3},
    {0xd787d2c4, 0xd787d2c4, StenoChord(0x000010a6), 0, 7, 1},
    {0xd787d2c4, 0xd787d2c4, StenoChord(0x000010fe), 0, 7, 1},
    {0xd7d174c9, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd7d174c9, 0x8707b2d8, StenoChord(0x00114000), 0, 8, 3},
    {0xd7d174c9, 0x8707b2d8, StenoChord(0x00514000), 0, 136, 3},
    {0xd8e006ac, 0x8707b2d8, StenoChord(0x000b0000), 0, 8, 3},
    {0xd8e006ac, 0x8707b2d8, StenoChord(0x002b0000), 0, 136, 3},
    {0xd95f49c1, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd95f49c1, 0x8707b2d8, StenoChord(0x00070000), 0, 8, 3},
    {0xd95f49c1, 0x8707b2d8, StenoChord(0x00270000), 0, 136, 3},
    {0xd9dbbaac, 0x8707b2d8, StenoChord(0x0028c000), 0, 136, 3},
    {0xd9e3b838, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xd9e3b838, 0x8707b2d8, StenoChord(0x00008000), 0, 8, 3},
    {0xd9e3b838, 0x8707b2d8, StenoChord(0x00208000), 0, 136, 3},
    {0xd9e56117, 0x8707b2d8, StenoChord(0x0009c000), 0, 8, 3},
    {0xd9e56117, 0x8707b2d8, StenoChord(0x0029c000), 0, 136, 3},
    {0xdaa35c8f, 0x8707b2d8, StenoChord(0x00074000), 0, 8, 3},
    {0xdac894e3, 0xdac894e3, StenoChord(0x00002c00), 0, 6, 1},
    {0xdafa51be, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xdafa51be, 0x8707b2d8, StenoChord(0x00008000), 0, 8, 3},
    {0xdafa51be, 0x8707b2d8, StenoChord(0x00208000), 0, 136, 3},
    {0xdb36efc7, 0x8707b2d8, StenoChord(0x000e0000), 0, 8, 3},
    {0xdb36efc7, 0x8707b2d8, StenoChord(0x002e0000), 0, 136, 3},
    {0xdbb7ef0c, 0x8707b2d8, StenoChord(0x000dc000), 0, 8, 3},
    {0xdbfaf0b0, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xdbfaf0b0, 0x8707b2d8, StenoChord(0x00084000), 0, 8, 3},
    {0xdbfaf0b0, 0x8707b2d8, StenoChord(0x00284000), 0, 136, 3},
    {0xdcb980a8, 0x8707b2d8, StenoChord(0x000cc000), 0, 8, 3},
    {0xdcb980a8, 0x8707b2d8, StenoChord(0x002cc000), 0, 136, 3},
    {0xdceebad1, 0x8707b2d8, StenoChord(0x00094000), 0, 8, 3},
    {0xdd0216b9, 0x710e300b, StenoChord(0x000000a2), 0, 1, 1},
    {0xdd0216b9, 0x710e300b, StenoChord(0x00001800), 0, 1, 2},
    {0xdd0fa368, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xdd0fa368, 0x8707b2d8, StenoChord(0x00114000), 0, 8, 3},
    {0xdd5c9ea2, 0x8707b2d8, StenoChord(0x0009c000), 0, 8, 3},
    {0xdd5c9ea2, 0x8707b2d8, StenoChord(0x0029c000), 0, 136, 3},
    {0xdd72bca0, 0x8707b2d8, StenoChord(0x000c8000), 0, 8, 3},
    {0xdd72bca0, 0x8707b2d8, StenoChord(0x002c8000), 0, 136, 3},
    {0xdd939ce8, 0x8707b2d8, StenoChord(0x00098000), 0, 8, 3},
    {0xdd939ce8, 0x8707b2d8, StenoChord(0x00298000), 0, 136, 3},
    {0xddd85bbd, 0x8707b2d8, StenoChord(0x00184000), 0, 8, 3},
    {0xddd85bbd, 0x8707b2d8, StenoChord(0x00784000), 0, 136, 3},
    {0xde51d5bf, 0x8707b2d8, StenoChord(0x00098000), 0, 8, 3},
    {0xde51d5bf, 0x8707b2d8, StenoChord(0x00298000), 0, 136, 3},
    {0xdee7fb99, 0x8707b2d8, StenoChord(0x00420000), 0, 8, 3},
    {0xdee7fb99, 0x8707b2d8, StenoChord(0x00620000), 0, 136, 3},
    {0xe134d5ad, 0x8707b2d8, StenoChord(0x00400000), 0, 8, 3},
    {0xe134d5ad, 0x8707b2d8, StenoChord(0x00600000), 0, 136, 3},
    {0xe1bf2d5a, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xe1bf2d5a, 0x8707b2d8, StenoChord(0x00110000), 0, 8, 3},
    {0xe1bf2d5a, 0x8707b2d8, StenoChord(0x00510000), 0, 136, 3},
    {0xe1dbf968, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xe1dbf968, 0x8707b2d8, StenoChord(0x00040000), 0, 8, 3},
    {0xe1dbf968, 0x8707b2d8, StenoChord(0x00240000), 0, 136, 3},
    {0xe26e7f10, 0x8707b2d8, StenoChord(0x001c8000), 0, 8, 3},
    {0xe26e7f10, 0x8707b2d8, StenoChord(0x007c8000), 0, 136, 3},
    {0xe46e0c39, 0xe46e0c39, StenoChord(0x00000c00), 0, 6, 1},
    {0xe50a8bf1, 0x8707b2d8, StenoChord(0x0018c000), 0, 8, 3},
    {0xe50a8bf1, 0x8707b2d8, StenoChord(0x0078c000), 0, 136, 3},
    {0xe50f412f, 0x8707b2d8, StenoChord(0x00480000), 0, 8, 3},
    {0xe50f412f, 0x8707b2d8, StenoChord(0x00680000), 0, 136, 3},
    {0xe593d6de, 0x8707b2d8, StenoChord(0x000a8000), 0, 8, 3},
    {0xe61425dc, 0x8707b2d8, StenoChord(0x00130000), 0, 8, 3},
    {0xe61425dc, 0x8707b2d8, StenoChord(0x00530000), 0, 136, 3},
    {0xe6f46c44, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xe6f46c44, 0x8707b2d8, StenoChord(0x0004c000), 0, 8, 3},
    {0xe6f46c44, 0x8707b2d8, StenoChord(0x0024c000), 0, 136, 3},
    {0xe7df49eb, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xe7df49eb, 0x8707b2d8, StenoChord(0x0011c000), 0, 8, 3},
    {0xe7df49eb, 0x8707b2d8, StenoChord(0x0051c000), 0, 136, 3},
    {0xe875de63, 0x8707b2d8, StenoChord(0x0008c000), 0, 8, 3},
    {0xe948c569, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xe948c569, 0x8707b2d8, StenoChord(0x00178000), 0, 8, 3},
    {0xea71121f, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xea71121f, 0x8707b2d8, StenoChord(0x00028000), 0, 8, 3},
    {0xea7cfac2, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xea7cfac2, 0x8707b2d8, StenoChord(0x00050000), 0, 8, 3},
    {0xead7e731, 0x8707b2d8, StenoChord(0x0008c000), 0, 8, 3},
    {0xead7e731, 0x8707b2d8, StenoChord(0x0028c000), 0, 136, 3},
    {0xeb6094e6, 0x8707b2d8, StenoChord(0x00180000), 0, 8, 3},
    {0xeb6094e6, 0x8707b2d8, StenoChord(0x00780000), 0, 136, 3},
    {0xebcdd0fb, 0x8707b2d8, StenoChord(0x000cc000), 0, 8, 3},
    {0xebcdd0fb, 0x8707b2d8, StenoChord(0x002cc000), 0, 136, 3},
    {0xebd87b09, 0x8707b2d8, StenoChord(0x00188000), 0, 8, 3},
    {0xebd87b09, 0x8707b2d8, StenoChord(0x00788000), 0, 136, 3},
    {0xec5b6e29, 0x8707b2d8, StenoChord(0x001c8000), 0, 8, 3},
    {0xee12f24d, 0x8707b2d8, StenoChord(0x001a4000), 0, 8, 3},
    {0xef35ef86, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xef35ef86, 0x8707b2d8, StenoChord(0x000a0000), 0, 8, 3},
    {0xef35ef86, 0x8707b2d8, StenoChord(0x002a0000), 0, 136, 3},
    {0xef3b04d6, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xef3b04d6, 0x8707b2d8, StenoChord(0x00110000), 0, 8, 3},
    {0xef3b04d6, 0x8707b2d8, StenoChord(0x00510000), 0, 136, 3},
    {0xef3e3778, 0x8707b2d8, StenoChord(0x00428000), 0, 8, 3},
    {0xef3e3778, 0x8707b2d8, StenoChord(0x00628000), 0, 136, 3},
    {0xefe51ebf, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xefe51ebf, 0x8707b2d8, StenoChord(0x00128000), 0, 8, 3},
    {0xefe51ebf, 0x8707b2d8, StenoChord(0x00528000), 0, 136, 3},
    {0xf0c1c06e, 0x8707b2d8, StenoChord(0x001a4000), 0, 8, 3},
    {0xf0c1c06e, 0x8707b2d8, StenoChord(0x007a4000), 0, 136, 3},
    {0xf16ec191, 0x8707b2d8, StenoChord(0x00120000), 0, 8, 3},
    {0xf16ec191, 0x8707b2d8, StenoChord(0x00520000), 0, 136, 3},
    {0xf3eb085e, 0x8707b2d8, StenoChord(0x000ac000), 0, 8, 3},
    {0xf3eb085e, 0x8707b2d8, StenoChord(0x002ac000), 0, 136, 3},
    {0xf46c3323, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xf46c3323, 0x8707b2d8, StenoChord(0x00060000), 0, 8, 3},
    {0xf46c3323, 0x8707b2d8, StenoChord(0x00260000), 0, 136, 3},
    {0xf51fcfcd, 0x8707b2d8, StenoChord(0x007c0000), 0, 136, 3},
    {0xf5221184, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xf5221184, 0x8707b2d8, StenoChord(0x0005c000), 0, 8, 3},
    {0xf52b94f3, 0x8707b2d8, StenoChord(0x00020000), 0, 8, 3},
    {0xf5984163, 0x8707b2d8, StenoChord(0x00614000), 0, 136, 3},
    {0xf6e50243, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xf6e50243, 0x8707b2d8, StenoChord(0x00054000), 0, 8, 3},
    {0xf6fb6275, 0x8707b2d8, StenoChord(0x00190000), 0, 8, 3},
    {0xf6fb6275, 0x8707b2d8, StenoChord(0x00790000), 0, 136, 3},
    {0xf7630619, 0x8707b2d8, StenoChord(0x001c0000), 0, 8, 3},
    {0xf7630619, 0x8707b2d8, StenoChord(0x007c0000), 0, 136, 3},
    {0xf770d380, 0x8707b2d8, StenoChord(0x000d4000), 0, 8, 3},
    {0xf770d380, 0x8707b2d8, StenoChord(0x002d4000), 0, 136, 3},
    {0xf7ccf8c1, 0x8707b2d8, StenoChord(0x00290000), 0, 136, 3},
    {0xf998c2f6, 0x8707b2d8, StenoChord(0x00074000), 0, 8, 3},
    {0xf998c2f6, 0x8707b2d8, StenoChord(0x00274000), 0, 136, 3},
    {0xfa94e6df, 0x8707b2d8, StenoChord(0x00400000), 0, 8, 3},
    {0xfb286a06, 0x8707b2d8, StenoChord(0x00158000), 0, 8, 3},
    {0xfb286a06, 0x8707b2d8, StenoChord(0x00558000), 0, 136, 3},
    {0xfb8ca445, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xfb8ca445, 0x8707b2d8, StenoChord(0x00060000), 0, 8, 3},
    {0xfb8ca445, 0x8707b2d8, StenoChord(0x00260000), 0, 136, 3},
    {0xfc8b1c22, 0x0609009d, StenoChord(0x00000400), 0, 2, 1},
    {0xfca1d0e5, 0x8707b2d8, StenoChord(0x000e0000), 0, 8, 3},
    {0xfca1d0e5, 0x8707b2d8, StenoChord(0x002e0000), 0, 136, 3},
    {0xfcc99c65, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xfcc99c65, 0x8707b2d8, StenoChord(0x00110000), 0, 8, 3},
    {0xfcc99c65, 0x8707b2d8, StenoChord(0x00510000), 0, 136, 3},
    {0xfd3b2e70, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xfd3b2e70, 0x8707b2d8, StenoChord(0x00140000), 0, 8, 3},
    {0xfd3b2e70, 0x8707b2d8, StenoChord(0x00540000), 0, 136, 3},
    {0xfe6f5a7e, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xfe6f5a7e, 0x8707b2d8, StenoChord(0x00068000), 0, 8, 3},
    {0xfe6f5a7e, 0x8707b2d8, StenoChord(0x00268000), 0, 136, 3},
    {0xfe885645, 0x00000000, StenoChord(0x00000000), 1, 0, 3},
    {0xfe885645, 0x8707b2d8, StenoChord(0x00048000), 0, 8, 3},
    {0xfe8c34a4, 0x8707b2d8, StenoChord(0x000b8000), 0, 8, 3},
    {0xfe8c34a4, 0x8707b2d8, StenoChord(0x002b8000), 0, 136, 3},
    {0xfeee8227, 0x710e300b, StenoChord(0x0000004e), 0, 1, 1},
    {0xff91f1fb, 0x8707b2d8, StenoChord(0x002c4000), 0, 136, 3},
    {0x00000000, 0x00000000, StenoChord(0x00000000), 0, 0, 0},
};

constexpr const JeffPhrasingReverseHashMapEntry *REVERSE_HASH_MAP[] = {
    REVERSE_ENTRIES + 0,
    REVERSE_ENTRIES + 1,
    REVERSE_ENTRIES + 229,
    REVERSE_ENTRIES + 908,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 723,
    nullptr,
    REVERSE_ENTRIES + 700,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 738,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 379,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 114,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 333,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 360,
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
    REVERSE_ENTRIES + 246,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 772,
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
    REVERSE_ENTRIES + 41,
    REVERSE_ENTRIES + 358,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 433,
    REVERSE_ENTRIES + 476,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 658,
    REVERSE_ENTRIES + 118,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 760,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 622,
    nullptr,
    REVERSE_ENTRIES + 863,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 59,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 859,
    REVERSE_ENTRIES + 536,
    REVERSE_ENTRIES + 19,
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
    REVERSE_ENTRIES + 314,
    REVERSE_ENTRIES + 717,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 227,
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
    REVERSE_ENTRIES + 546,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 567,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 322,
    REVERSE_ENTRIES + 619,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 195,
    REVERSE_ENTRIES + 304,
    REVERSE_ENTRIES + 788,
    REVERSE_ENTRIES + 69,
    nullptr,
    REVERSE_ENTRIES + 136,
    nullptr,
    REVERSE_ENTRIES + 14,
    REVERSE_ENTRIES + 785,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 614,
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
    REVERSE_ENTRIES + 202,
    REVERSE_ENTRIES + 881,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 15,
    REVERSE_ENTRIES + 288,
    nullptr,
    REVERSE_ENTRIES + 639,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 635,
    REVERSE_ENTRIES + 489,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 541,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
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
    REVERSE_ENTRIES + 891,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 729,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 46,
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
    REVERSE_ENTRIES + 842,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 194,
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
    nullptr,
    REVERSE_ENTRIES + 617,
    REVERSE_ENTRIES + 319,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 775,
    REVERSE_ENTRIES + 447,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 511,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 302,
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
    REVERSE_ENTRIES + 409,
    nullptr,
    REVERSE_ENTRIES + 820,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 372,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 317,
    REVERSE_ENTRIES + 24,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 394,
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
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 477,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 647,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 113,
    REVERSE_ENTRIES + 872,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 529,
    REVERSE_ENTRIES + 467,
    REVERSE_ENTRIES + 812,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 192,
    nullptr,
    REVERSE_ENTRIES + 472,
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
    REVERSE_ENTRIES + 164,
    nullptr,
    REVERSE_ENTRIES + 426,
    nullptr,
    REVERSE_ENTRIES + 869,
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
    REVERSE_ENTRIES + 861,
    nullptr,
    REVERSE_ENTRIES + 97,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 344,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 407,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 149,
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
    REVERSE_ENTRIES + 130,
    nullptr,
    REVERSE_ENTRIES + 779,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 273,
    REVERSE_ENTRIES + 768,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 280,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 624,
    REVERSE_ENTRIES + 420,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 699,
    REVERSE_ENTRIES + 691,
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
    REVERSE_ENTRIES + 484,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 35,
    REVERSE_ENTRIES + 828,
    REVERSE_ENTRIES + 673,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 158,
    REVERSE_ENTRIES + 237,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 247,
    REVERSE_ENTRIES + 730,
    REVERSE_ENTRIES + 907,
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
    REVERSE_ENTRIES + 885,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 653,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 91,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 481,
    nullptr,
    REVERSE_ENTRIES + 545,
    REVERSE_ENTRIES + 190,
    REVERSE_ENTRIES + 587,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 84,
    REVERSE_ENTRIES + 834,
    REVERSE_ENTRIES + 239,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 665,
    REVERSE_ENTRIES + 906,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 300,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 391,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 309,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 161,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 749,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 873,
    nullptr,
    REVERSE_ENTRIES + 473,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 342,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 847,
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
    REVERSE_ENTRIES + 668,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 279,
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
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 875,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 899,
    nullptr,
    REVERSE_ENTRIES + 758,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 370,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 277,
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
    REVERSE_ENTRIES + 219,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 338,
    REVERSE_ENTRIES + 552,
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
    REVERSE_ENTRIES + 771,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 362,
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
    REVERSE_ENTRIES + 836,
    REVERSE_ENTRIES + 581,
    REVERSE_ENTRIES + 761,
    REVERSE_ENTRIES + 102,
    REVERSE_ENTRIES + 618,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 10,
    nullptr,
    REVERSE_ENTRIES + 242,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 790,
    REVERSE_ENTRIES + 365,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 696,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 89,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 384,
    nullptr,
    REVERSE_ENTRIES + 270,
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
    REVERSE_ENTRIES + 78,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 437,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 17,
    REVERSE_ENTRIES + 882,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 51,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 152,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 574,
    nullptr,
    REVERSE_ENTRIES + 133,
    REVERSE_ENTRIES + 844,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 404,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 346,
    REVERSE_ENTRIES + 350,
    REVERSE_ENTRIES + 430,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 356,
    REVERSE_ENTRIES + 458,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 865,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 121,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 537,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 603,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 189,
    REVERSE_ENTRIES + 629,
    nullptr,
    REVERSE_ENTRIES + 214,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 159,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 124,
    REVERSE_ENTRIES + 543,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 111,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 87,
    REVERSE_ENTRIES + 144,
    REVERSE_ENTRIES + 650,
    REVERSE_ENTRIES + 482,
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
    REVERSE_ENTRIES + 212,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 684,
    nullptr,
    REVERSE_ENTRIES + 793,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 27,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 531,
    REVERSE_ENTRIES + 674,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 293,
    REVERSE_ENTRIES + 147,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 462,
    REVERSE_ENTRIES + 879,
    REVERSE_ENTRIES + 449,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 559,
    REVERSE_ENTRIES + 321,
    nullptr,
    REVERSE_ENTRIES + 671,
    REVERSE_ENTRIES + 584,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 454,
    REVERSE_ENTRIES + 261,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 672,
    REVERSE_ENTRIES + 805,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 184,
    REVERSE_ENTRIES + 521,
    REVERSE_ENTRIES + 72,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 224,
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
    REVERSE_ENTRIES + 470,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 681,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 243,
    REVERSE_ENTRIES + 597,
    REVERSE_ENTRIES + 714,
    nullptr,
    REVERSE_ENTRIES + 185,
    REVERSE_ENTRIES + 65,
    REVERSE_ENTRIES + 324,
    REVERSE_ENTRIES + 801,
    REVERSE_ENTRIES + 163,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 725,
    nullptr,
    REVERSE_ENTRIES + 520,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 311,
    nullptr,
    REVERSE_ENTRIES + 591,
    nullptr,
    REVERSE_ENTRIES + 600,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 459,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 82,
    REVERSE_ENTRIES + 141,
    REVERSE_ENTRIES + 355,
    REVERSE_ENTRIES + 688,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 577,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 479,
    REVERSE_ENTRIES + 818,
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
    REVERSE_ENTRIES + 175,
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
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 630,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 500,
    REVERSE_ENTRIES + 95,
    nullptr,
    REVERSE_ENTRIES + 694,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 890,
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
    REVERSE_ENTRIES + 492,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 21,
    nullptr,
    REVERSE_ENTRIES + 298,
    nullptr,
    REVERSE_ENTRIES + 637,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 817,
    nullptr,
    REVERSE_ENTRIES + 517,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 528,
    REVERSE_ENTRIES + 649,
    nullptr,
    REVERSE_ENTRIES + 679,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 825,
    REVERSE_ENTRIES + 887,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 435,
    nullptr,
    REVERSE_ENTRIES + 395,
    REVERSE_ENTRIES + 515,
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
    REVERSE_ENTRIES + 296,
    nullptr,
    REVERSE_ENTRIES + 381,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 421,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 893,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 740,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 253,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 557,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 417,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 397,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 703,
    REVERSE_ENTRIES + 777,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 656,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 499,
    REVERSE_ENTRIES + 186,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 797,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 428,
    REVERSE_ENTRIES + 904,
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
    REVERSE_ENTRIES + 307,
    nullptr,
    REVERSE_ENTRIES + 707,
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
    REVERSE_ENTRIES + 63,
    REVERSE_ENTRIES + 80,
    REVERSE_ENTRIES + 291,
    REVERSE_ENTRIES + 732,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 44,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 763,
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
    REVERSE_ENTRIES + 851,
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
    REVERSE_ENTRIES + 778,
    nullptr,
    REVERSE_ENTRIES + 563,
    REVERSE_ENTRIES + 840,
    nullptr,
    REVERSE_ENTRIES + 799,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 683,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 562,
    nullptr,
    REVERSE_ENTRIES + 708,
    REVERSE_ENTRIES + 871,
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
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 74,
    REVERSE_ENTRIES + 173,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 109,
    REVERSE_ENTRIES + 268,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 12,
    nullptr,
    REVERSE_ENTRIES + 196,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 257,
    REVERSE_ENTRIES + 570,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 487,
    nullptr,
    REVERSE_ENTRIES + 575,
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
    REVERSE_ENTRIES + 286,
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
    REVERSE_ENTRIES + 255,
    REVERSE_ENTRIES + 331,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 236,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 809,
    REVERSE_ENTRIES + 348,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 551,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 560,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 832,
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
    REVERSE_ENTRIES + 28,
    REVERSE_ENTRIES + 116,
    REVERSE_ENTRIES + 266,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 593,
    nullptr,
    REVERSE_ENTRIES + 215,
    REVERSE_ENTRIES + 340,
    REVERSE_ENTRIES + 274,
    nullptr,
    REVERSE_ENTRIES + 200,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 606,
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
    REVERSE_ENTRIES + 67,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 86,
    REVERSE_ENTRIES + 104,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 807,
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
    REVERSE_ENTRIES + 803,
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
    REVERSE_ENTRIES + 142,
    nullptr,
    REVERSE_ENTRIES + 410,
    REVERSE_ENTRIES + 514,
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
    REVERSE_ENTRIES + 823,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 234,
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
    REVERSE_ENTRIES + 704,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 245,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 54,
    REVERSE_ENTRIES + 146,
    REVERSE_ENTRIES + 204,
    REVERSE_ENTRIES + 507,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 440,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 747,
    nullptr,
    REVERSE_ENTRIES + 569,
    REVERSE_ENTRIES + 169,
    REVERSE_ENTRIES + 677,
    nullptr,
    REVERSE_ENTRIES + 208,
    nullptr,
    REVERSE_ENTRIES + 177,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 720,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 877,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 155,
    nullptr,
    REVERSE_ENTRIES + 8,
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
    REVERSE_ENTRIES + 846,
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
    REVERSE_ENTRIES + 29,
    REVERSE_ENTRIES + 374,
    nullptr,
    REVERSE_ENTRIES + 32,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 76,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 167,
    REVERSE_ENTRIES + 902,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 216,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 131,
    nullptr,
    REVERSE_ENTRIES + 138,
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
    REVERSE_ENTRIES + 349,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 831,
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
    REVERSE_ENTRIES + 99,
    nullptr,
    REVERSE_ENTRIES + 896,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 475,
    nullptr,
    REVERSE_ENTRIES + 206,
    REVERSE_ENTRIES + 250,
    REVERSE_ENTRIES + 259,
    REVERSE_ENTRIES + 297,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 443,
    REVERSE_ENTRIES + 610,
    REVERSE_ENTRIES + 627,
    nullptr,
    REVERSE_ENTRIES + 745,
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
    REVERSE_ENTRIES + 156,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 669,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 128,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 795,
    REVERSE_ENTRIES + 386,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 523,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 766,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 352,
    nullptr,
    REVERSE_ENTRIES + 686,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 125,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 791,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 856,
    nullptr,
    REVERSE_ENTRIES + 706,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 486,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 555,
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
    REVERSE_ENTRIES + 612,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 822,
    REVERSE_ENTRIES + 884,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 400,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 263,
    nullptr,
    REVERSE_ENTRIES + 48,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 389,
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
    REVERSE_ENTRIES + 25,
    nullptr,
    nullptr,
    nullptr,
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
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 784,
    nullptr,
    REVERSE_ENTRIES + 464,
    nullptr,
    REVERSE_ENTRIES + 815,
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
    REVERSE_ENTRIES + 218,
    nullptr,
    REVERSE_ENTRIES + 633,
    REVERSE_ENTRIES + 539,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 505,
    nullptr,
    REVERSE_ENTRIES + 572,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 210,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 838,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 579,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 4,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 645,
    REVERSE_ENTRIES + 727,
    REVERSE_ENTRIES + 171,
    REVERSE_ENTRIES + 455,
    REVERSE_ENTRIES + 283,
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
    REVERSE_ENTRIES + 174,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 742,
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
    REVERSE_ENTRIES + 438,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 502,
    REVERSE_ENTRIES + 675,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 735,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 854,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 423,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 848,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 608,
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
    REVERSE_ENTRIES + 401,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 452,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 336,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 663,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 232,
    REVERSE_ENTRIES + 782,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 589,
    nullptr,
    REVERSE_ENTRIES + 868,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_ENTRIES + 61,
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
    REVERSE_ENTRIES + 445,
    nullptr,
    REVERSE_ENTRIES + 166,
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
    REVERSE_ENTRIES + 566,
    nullptr,
    nullptr,
};

constexpr JeffPhrasingReverseStructureEntry REVERSE_STRUCTURES[] = {
    {0x01b0b145, StenoChord(0x000034a6), 1},
    {0x01b0b145, StenoChord(0x000034fe), 1},
    {0x04851ccd, StenoChord(0x00002000), 1},
    {0x065238df, StenoChord(0x00000800), 1},
    {0x0a8e1d6a, StenoChord(0x00002000), 3},
    {0x0c5b397c, StenoChord(0x00000800), 1},
    {0x0e583654, StenoChord(0x00000c00), 1},
    {0x123b84f3, StenoChord(0x00003800), 1},
    {0x132bc08a, StenoChord(0x00000800), 1},
    {0x178c0805, StenoChord(0x00002800), 1},
    {0x18448590, StenoChord(0x00003800), 1},
    {0x1934c127, StenoChord(0x00000800), 1},
    {0x1d9508a2, StenoChord(0x00002800), 1},
    {0x29e9718d, StenoChord(0x00000800), 1},
    {0x2a7a0b8b, StenoChord(0x00000c00), 1},
    {0x3500c57a, StenoChord(0x00000800), 1},
    {0x3b09c617, StenoChord(0x00000800), 1},
    {0x3f6f7f54, StenoChord(0x00002400), 1},
    {0x4b29d51a, StenoChord(0x00002400), 1},
    {0x4e5d86a1, StenoChord(0x00001800), 1},
    {0x517c57af, StenoChord(0x00002800), 1},
    {0x5466873e, StenoChord(0x00001800), 1},
    {0x5647d24a, StenoChord(0x00003ca6), 1},
    {0x5647d24a, StenoChord(0x00003cfe), 1},
    {0x5785584c, StenoChord(0x00002800), 1},
    {0x5e8f859c, StenoChord(0x000010a6), 1},
    {0x5e8f859c, StenoChord(0x000010fe), 1},
    {0x66958311, StenoChord(0x000014a6), 1},
    {0x66958311, StenoChord(0x000014fe), 1},
    {0x6cb5e0b3, StenoChord(0x00003000), 1},
    {0x72bee150, StenoChord(0x00003000), 1},
    {0x82add63a, StenoChord(0x00002400), 1},
    {0x87d29f10, StenoChord(0x00002000), 1},
    {0x8c66ba36, StenoChord(0x00002c00), 1},
    {0x8ddb9fad, StenoChord(0x00002000), 3},
    {0x8e9bd989, StenoChord(0x00000800), 1},
    {0x94a4da26, StenoChord(0x00000800), 1},
    {0x98210ffc, StenoChord(0x00002c00), 1},
    {0xa12d54e8, StenoChord(0x000038a6), 1},
    {0xa12d54e8, StenoChord(0x000038fe), 1},
    {0xa4aadda9, StenoChord(0x00000c00), 1},
    {0xadef0f22, StenoChord(0x00000c00), 1},
    {0xb177a334, StenoChord(0x00000800), 1},
    {0xb780a3d1, StenoChord(0x00000800), 1},
    {0xca94cd23, StenoChord(0x00002000), 1},
    {0xcd5f02f2, StenoChord(0x00003800), 1},
    {0xcfa5111c, StenoChord(0x00002c00), 1},
    {0xd09dcdc0, StenoChord(0x00002000), 3},
    {0xd4c9d9f2, StenoChord(0x00002800), 1},
    {0xdad2da8f, StenoChord(0x00002800), 1},
    {0xdc83ef1c, StenoChord(0x00000c00), 1},
    {0xdd4f5696, StenoChord(0x000018a6), 1},
    {0xdd4f5696, StenoChord(0x000018fe), 1},
    {0xe3585733, StenoChord(0x00001ca6), 1},
    {0xe3585733, StenoChord(0x00001cfe), 1},
    {0xf815e2e3, StenoChord(0x00000000), 1},
    {0xfba7b0a8, StenoChord(0x000030a6), 1},
    {0xfba7b0a8, StenoChord(0x000030fe), 1},
    {0xfe1ee380, StenoChord(0x00000000), 3},
    {0xfe1ee380, StenoChord(0x00001000), 1},
    {0x00000000, StenoChord(0x00000000), 0},
};

constexpr const JeffPhrasingReverseStructureEntry *REVERSE_STRUCTURES_MAP[] = {
    REVERSE_STRUCTURES + 58,
    REVERSE_STRUCTURES + 60,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 9,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 35,
    REVERSE_STRUCTURES + 8,
    REVERSE_STRUCTURES + 14,
    nullptr,
    REVERSE_STRUCTURES + 13,
    nullptr,
    REVERSE_STRUCTURES + 49,
    REVERSE_STRUCTURES + 10,
    REVERSE_STRUCTURES + 27,
    REVERSE_STRUCTURES + 32,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 51,
    REVERSE_STRUCTURES + 16,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 18,
    nullptr,
    REVERSE_STRUCTURES + 25,
    REVERSE_STRUCTURES + 46,
    REVERSE_STRUCTURES + 50,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 19,
    REVERSE_STRUCTURES + 12,
    REVERSE_STRUCTURES + 41,
    REVERSE_STRUCTURES + 44,
    nullptr,
    REVERSE_STRUCTURES + 36,
    REVERSE_STRUCTURES + 11,
    REVERSE_STRUCTURES + 56,
    REVERSE_STRUCTURES + 40,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 34,
    nullptr,
    REVERSE_STRUCTURES + 20,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 29,
    REVERSE_STRUCTURES + 42,
    REVERSE_STRUCTURES + 53,
    REVERSE_STRUCTURES + 33,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 31,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 21,
    nullptr,
    REVERSE_STRUCTURES + 47,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 0,
    REVERSE_STRUCTURES + 1,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 22,
    nullptr,
    REVERSE_STRUCTURES + 24,
    REVERSE_STRUCTURES + 2,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 30,
    REVERSE_STRUCTURES + 43,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 6,
    REVERSE_STRUCTURES + 17,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 3,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 55,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 38,
    nullptr,
    REVERSE_STRUCTURES + 4,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 45,
    REVERSE_STRUCTURES + 7,
    REVERSE_STRUCTURES + 48,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    REVERSE_STRUCTURES + 15,
    nullptr,
    REVERSE_STRUCTURES + 5,
    REVERSE_STRUCTURES + 37,
    nullptr,
    nullptr,
};

//---------------------------------------------------------------------------

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

void GenerateLookupTable() {
  printf("endersCount: %zu\n", phrasingData.endersCount);

  static const size_t HASH_SIZE = 512;

  const JeffPhrasingEnder **enderMap = new const JeffPhrasingEnder *[512];
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    enderMap[i] = nullptr;
  }

  for (size_t i = 0; i < phrasingData.endersCount; ++i) {
    const JeffPhrasingEnder *ender = &phrasingData.enders[i];
    size_t index = ender->chord.Hash() & (HASH_SIZE - 1);

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
    StenoChord chord;
    bool checkNext : 1;
    bool optionalReplace : 1;
    uint8_t componentMask;
    uint8_t modeMask;
  };

  std::vector<Entry> entries;

  void RecurseVariant(const JeffPhrasingVariant &variant, StenoChord chord,
                      uint32_t replacement, uint8_t componentMask,
                      uint8_t modeMask, const char *suffix);

  void AddText(const char *text, StenoChord chord, uint32_t replacement,
               uint8_t componentMask, uint8_t modeMask);
};

void ReverseBuilder::Generate() {
  uint32_t starterReplacement = Crc32("\\0", 2);
  for (size_t i = 0; i < phrasingData.fullStarterCount; ++i) {
    AddText(phrasingData.fullStarters[i].pronoun.word,
            phrasingData.fullStarters[i].chord, starterReplacement, 1, 1);
  }

  for (size_t i = 1; i < 8; ++i) {
    const JeffPhrasingPronoun &pronoun = phrasingData.simplePronouns[i];
    AddText(pronoun.word, StenoChord(uint32_t(i) << ChordBitIndex::STAR),
            starterReplacement, 1, 2);
  }

  uint32_t middleReplacement = Crc32("\\1", 2);
  for (size_t i = 0; i < 8; ++i) {
    RecurseVariant(phrasingData.fullMiddles[i].word,
                   StenoChord(uint32_t(i) << ChordBitIndex::A),
                   middleReplacement, 2, 1, nullptr);
  }

  for (size_t i = 0; i < phrasingData.simpleStarterCount; ++i) {
    RecurseVariant(phrasingData.simpleStarters[i].middle.word,
                   phrasingData.simpleStarters[i].chord, middleReplacement, 2,
                   2, nullptr);
  }

  for (size_t i = 0; i < 16; ++i) {
    const JeffPhrasingStructure &structure = phrasingData.fullStructures[i];
    RecurseVariant(structure.format,
                   StenoChord(uint32_t(i) << ChordBitIndex::STAR), 0, 4, 1,
                   nullptr);
  }

  for (size_t i = 0; i < 2; ++i) {
    const JeffPhrasingStructure &structure = phrasingData.simpleStructures[i];
    RecurseVariant(structure.format,
                   StenoChord(uint32_t(i) << ChordBitIndex::FR), 0, 4, 2,
                   nullptr);
  }

  for (size_t i = 0; i < phrasingData.structureExceptionCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.structureExceptions[i];
    RecurseVariant(structure.structure.format, structure.chord, 0, 6, 1,
                   nullptr);
  }

  for (size_t i = 0; i < phrasingData.uniqueStarterCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.uniqueStarters[i];
    RecurseVariant(structure.structure.format, structure.chord, 0, 7, 1,
                   nullptr);
  }

  uint32_t verbReplacement = Crc32("\\2", 2);
  uint32_t suffixReplacement = Crc32("\\3", 2);

  for (size_t i = 0; i < phrasingData.endersCount; ++i) {
    const JeffPhrasingEnder &ender = phrasingData.enders[i];
    uint8_t componentMask = 8;
    if (ender.tense == Tense::PAST) {
      componentMask |= 0x80;
    }
    RecurseVariant(ender.ender, ender.chord,
                   verbReplacement + suffixReplacement, componentMask, 3,
                   ender.suffix);
  }

  std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
    return a.hash < b.hash ||
           (a.hash == b.hash && a.chord.GetKeyState() < b.chord.GetKeyState());
  });
  for (size_t i = 0; i < entries.size() - 1; ++i) {
    Entry &entry = entries[i];
    Entry &next = entries[i + 1];
    if (entry.hash == next.hash && entry.chord == next.chord &&
        entry.replacementHash == next.replacementHash &&
        entry.componentMask == next.componentMask) {
      entry.modeMask |= next.modeMask;
      entries.erase(entries.begin() + i + 1);
      --i;
    }
  }
  Entry sentinel = {};
  entries.push_back(sentinel);

  printf("constexpr JeffPhrasingReverseHashMapEntry REVERSE_ENTRIES[] = {\n");
  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry &entry = entries[i];
    printf("  { 0x%08x, 0x%08x, StenoChord(0x%08x), %d, %d, "
           "%d},\n",
           entry.hash, entry.replacementHash, entry.chord.GetKeyState(),
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
                                    StenoChord chord, uint32_t replacement,
                                    uint8_t componentMask, uint8_t modeMask,
                                    const char *suffix) {
  switch (variant.type) {
  case JeffPhrasingVariant::Type::MAP: {
    const JeffPhrasingMap *map = variant.map;
    for (size_t i = 0; i < map->entryCount; ++i) {
      uint8_t localComponentMask = componentMask;
      if (map->entries[i].key == (uint32_t)Tense::PAST) {
        localComponentMask |= 0x80;
      }
      RecurseVariant(map->entries[i].value, chord, replacement,
                     localComponentMask, modeMask, suffix);
    }
  } break;

  case JeffPhrasingVariant::Type::UNKNOWN:
    break;

  case JeffPhrasingVariant::Type::TEXT:
    if (suffix != nullptr) {
      char buffer[64];
      snprintf(buffer, 63, "%s%s", variant.text, suffix);
      AddText(buffer, chord, replacement, componentMask, modeMask);
    } else {
      AddText(variant.text, chord, replacement, componentMask, modeMask);
    }
    break;
  }
}

void ReverseBuilder::AddText(const char *text, StenoChord chord,
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
  entry.replacementHash = replacement == 0 ? entry.hash : replacement;
  entry.chord = chord;
  entry.componentMask = componentMask;
  entry.modeMask = modeMask;
  entries.push_back(entry);

  const char *space = (const char *)memchr(p, ' ', pEnd - p);

  if (space) {
    Entry entry = {};
    entry.modeMask = modeMask;
    entry.hash = Crc32(p, space - p);
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
    StenoChord chord;
    uint8_t modeMask;
  };

  std::vector<Entry> entries;

  void RecurseVariant(const JeffPhrasingVariant &variant, StenoChord chord,
                      uint8_t modeMask);
};

void ReverseStructureBuilder::Generate() {
  for (size_t i = 0; i < 16; ++i) {
    RecurseVariant(phrasingData.fullStructures[i].format,
                   StenoChord(uint32_t(i & ~1) << ChordBitIndex::STAR), 1);
  }

  for (size_t i = 0; i < 2; ++i) {
    RecurseVariant(phrasingData.simpleStructures[i].format,
                   StenoChord(uint32_t(i) << ChordBitIndex::FR), 2);
  }

  for (size_t i = 0; i < phrasingData.structureExceptionCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.structureExceptions[i];
    RecurseVariant(structure.structure.format, structure.chord, 1);
  }

  for (size_t i = 0; i < phrasingData.uniqueStarterCount; ++i) {
    const JeffPhrasingStructureException &structure =
        phrasingData.uniqueStarters[i];
    RecurseVariant(structure.structure.format, structure.chord, 1);
  }

  std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
    return a.hash < b.hash ||
           (a.hash == b.hash && a.chord.GetKeyState() < b.chord.GetKeyState());
  });
  for (size_t i = 0; i < entries.size() - 1; ++i) {
    Entry &entry = entries[i];
    Entry &next = entries[i + 1];
    if (entry.hash == next.hash && entry.chord == next.chord) {
      entry.modeMask |= next.modeMask;
      entries.erase(entries.begin() + i + 1);
      --i;
    }
  }
  Entry sentinel;
  sentinel.hash = 0;
  sentinel.chord = StenoChord();
  sentinel.modeMask = 0;
  entries.push_back(sentinel);

  printf(
      "constexpr JeffPhrasingReverseStructureEntry REVERSE_STRUCTURES[] = {\n");
  for (size_t i = 0; i < entries.size(); ++i) {
    printf("  { 0x%08x, StenoChord(0x%08x), %d},\n", entries[i].hash,
           entries[i].chord.GetKeyState(), entries[i].modeMask);
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
                                             StenoChord chord,
                                             uint8_t modeMask) {
  switch (variant.type) {
  case JeffPhrasingVariant::Type::MAP: {
    const JeffPhrasingMap *map = variant.map;
    for (size_t i = 0; i < map->entryCount; ++i) {
      RecurseVariant(map->entries[i].value, chord, modeMask);
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
      while (*pEnd != '\\' && *pEnd != ' ' && *pEnd != '\0') {
        ++pEnd;
      }
      hash += Crc32(p, pEnd - p);
      p = pEnd;
    }

    Entry entry;
    entry.chord = chord;
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
  return 0;
}

#endif
