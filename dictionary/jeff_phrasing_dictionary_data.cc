//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary_data.h"
#include "dictionary.h"

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
};

//---------------------------------------------------------------------------
