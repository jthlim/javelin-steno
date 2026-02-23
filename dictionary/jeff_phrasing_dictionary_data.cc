//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary_data.h"
#include <string.h>

//---------------------------------------------------------------------------

// spellchecker: disable

// clang-format off
constexpr JeffPhrasingSimpleStarter SIMPLE_STARTERS[] = {
    {StenoStroke(0x146 /*STHA*/), "that"},
    {StenoStroke(0x116 /*STPA*/), "if"},
    {StenoStroke(0x62 /*SWH*/), "when"},
    {StenoStroke(0x162 /*SWHA*/), "what"},
    {StenoStroke(0xe2 /*SWHR*/), "where"},
    {StenoStroke(0x262 /*SWHO*/), "who"},
    {StenoStroke(0x362 /*SWHAO*/), "why"},
    {StenoStroke(0x72 /*SPWH*/), "but"},
    {StenoStroke(0x96 /*STPR*/), "for"},

    // # Remove the entry below if you don't want "and" phrases.
    {StenoStroke(0x1a /*SKP*/), "and"},
};

constexpr JeffPhrasingFullStarter FULL_STARTERS[] = {
  {StenoStroke(0xa2 /*SWR*/), {"I", WordForm::FIRST_PERSON_SINGULAR, true}},
  {StenoStroke(0xb8 /*KPWR*/), {"you", WordForm::SECOND_PERSON, true}},
  {StenoStroke(0xe8 /*KWHR*/), {"he", WordForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xea /*SKWHR*/), {"she", WordForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0x78 /*KPWH*/), {"it", WordForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xa4 /*TWR*/), {"we", WordForm::FIRST_PERSON_PLURAL, true}},
  {StenoStroke(0x64 /*TWH*/), {"they", WordForm::THIRD_PERSON_PLURAL, true}},
  {StenoStroke(0x4e /*STKH*/), {"this", WordForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0x66 /*STWH*/), {"that", WordForm::THIRD_PERSON_SINGULAR, true}},
  {StenoStroke(0xc6 /*STHR*/), {"there", WordForm::THIRD_PERSON_SINGULAR, false}},
  {StenoStroke(0xd6 /*STPHR*/), {"there", WordForm::THIRD_PERSON_PLURAL, false}},
  {StenoStroke(0xfe /*STKPWHR*/), {"", WordForm::THIRD_PERSON_SINGULAR_BLANK_PRONOUN, true}},
  {StenoStroke(0xa6 /*STWR*/), {"", WordForm::THIRD_PERSON_PLURAL_BLANK_PRONOUN, true}},
};
// clang-format on

constexpr JeffPhrasingPronoun SIMPLE_PRONOUNS[] = {
    {}, // Unused
    {"it", WordForm::THIRD_PERSON_SINGULAR, true},
    {"he", WordForm::THIRD_PERSON_SINGULAR, true},
    {"she", WordForm::THIRD_PERSON_SINGULAR, true},
    {"you", WordForm::SECOND_PERSON, true},
    {"they", WordForm::THIRD_PERSON_PLURAL, true},
    {"I", WordForm::FIRST_PERSON_SINGULAR, true},
    {"we", WordForm::FIRST_PERSON_PLURAL, true},
};

constexpr JeffPhrasingMapData<2> SIMPLE_STRUCTURE_HAVE_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\1 \\0 has\\2\\3"},
            {WordForm::UNSPECIFIED, "\\1 \\0 have\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> SIMPLE_STRUCTURE_HAVE = {
    .entries =
        {
            {WordForm::PAST, "\\1 \\0 had\\2\\3"},
            {WordForm::PRESENT, &SIMPLE_STRUCTURE_HAVE_PRESENT},
        },
};

constexpr JeffPhrasingStructure SIMPLE_STRUCTURES[] = {
    {"\\1 \\0\\2\\3", true, WordForm::UNSPECIFIED},
    {&SIMPLE_STRUCTURE_HAVE, true, WordForm::PAST_PARTICIPLE},
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, {" does", WordForm::ROOT}},
            {WordForm::UNSPECIFIED, {" do", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO = {
    .entries =
        {
            {WordForm::PAST, {" did", WordForm::ROOT}},
            {WordForm::PRESENT, &MIDDLES_DO_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_SHALL = {
    .entries =
        {
            {WordForm::PAST, {" should", WordForm::ROOT}},
            {WordForm::PRESENT, {" shall", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_CAN = {
    .entries =
        {
            {WordForm::PAST, {" could", WordForm::ROOT}},
            {WordForm::PRESENT, {" can", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_WILL = {
    .entries =
        {
            {WordForm::PAST, {" would", WordForm::ROOT}},
            {WordForm::PRESENT, {" will", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_NOT_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, {" doesn't", WordForm::ROOT}},
            {WordForm::UNSPECIFIED, {" don't", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_DO_NOT = {
    .entries =
        {
            {WordForm::PAST, {" didn't", WordForm::ROOT}},
            {WordForm::PRESENT, &MIDDLES_DO_NOT_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_SHALL_NOT = {
    .entries =
        {
            {WordForm::PAST, {" shouldn't", WordForm::ROOT}},
            {WordForm::PRESENT, {" shall not", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_CAN_NOT = {
    .entries =
        {
            {WordForm::PAST, {" couldn't", WordForm::ROOT}},
            {WordForm::PRESENT, {" can't", WordForm::ROOT}},
        },
};

constexpr JeffPhrasingMapData<2> MIDDLES_WILL_NOT = {
    .entries =
        {
            {WordForm::PAST, {" wouldn't", WordForm::ROOT}},
            {WordForm::PRESENT, {" won't", WordForm::ROOT}},
        },
};

// Indexes are into `AO*`
constexpr JeffPhrasingMiddle MIDDLES[8] = {
    {.word = &MIDDLES_DO},        //
    {.word = &MIDDLES_CAN},       //
    {.word = &MIDDLES_SHALL},     //
    {.word = &MIDDLES_WILL},      //
    {.word = &MIDDLES_DO_NOT},    //
    {.word = &MIDDLES_CAN_NOT},   //
    {.word = &MIDDLES_SHALL_NOT}, //
    {.word = &MIDDLES_WILL_NOT},  //
};

constexpr JeffPhrasingMapData<4> FULL_STRUCTURE_BE_PRESENT = {
    .entries =
        {
            {WordForm::ROOT, "\\0\\1 be\\2\\3"},
            {WordForm::FIRST_PERSON_SINGULAR, "\\0\\1 am\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0\\1 is\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 are\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<4> FULL_STRUCTURE_BE_PAST = {
    .entries =
        {
            {WordForm::ROOT, "\\0\\1 be\\2\\3"},
            {WordForm::FIRST_PERSON_SINGULAR, "\\0\\1 was\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0\\1 was\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 were\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_BE = {
    .entries =
        {
            {WordForm::PAST, &FULL_STRUCTURE_BE_PAST},
            {WordForm::PRESENT, &FULL_STRUCTURE_BE_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0\\1 has\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 have\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_PAST = {
    .entries =
        {
            {WordForm::PAST, "\\0\\1 had\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 have\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE = {
    .entries =
        {
            {WordForm::PAST, &FULL_STRUCTURE_HAVE_PAST},
            {WordForm::PRESENT, &FULL_STRUCTURE_HAVE_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0\\1 has been\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 have been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN_PAST = {
    .entries =
        {
            {WordForm::PAST, "\\0\\1 had been\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0\\1 have been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_HAVE_BEEN = {
    .entries =
        {
            {WordForm::PAST, &FULL_STRUCTURE_HAVE_BEEN_PAST},
            {WordForm::PRESENT, &FULL_STRUCTURE_HAVE_BEEN_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> FULL_STRUCTURE_INVERT_PRONOUN = {
    .entries =
        {
            {WordForm::BLANK_PRONOUN, "\\1 always\\0\\2\\3"},
            {WordForm::UNSPECIFIED, "\\1 \\0\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<1> FULL_STRUCTURE_INVERT = {
    .entries =
        {
            {WordForm::UNSPECIFIED, &FULL_STRUCTURE_INVERT_PRONOUN},
        },
};

// Indexed on `*EUF`
// \\0 is starter.
// \\1 is middle.
// \\2 is verb
// \\3 is suffix word
constexpr JeffPhrasingStructure STRUCTURES[16] = {
    {"\\0\\1\\2\\3", true, WordForm::UNSPECIFIED},
    {"\\0\\1\\2\\3", true, WordForm::UNSPECIFIED},
    {&FULL_STRUCTURE_BE, true, WordForm::PRESENT_PARTICIPLE},
    {&FULL_STRUCTURE_BE, true, WordForm::PRESENT_PARTICIPLE},
    {&FULL_STRUCTURE_INVERT, true, WordForm::UNSPECIFIED},
    {&FULL_STRUCTURE_INVERT, true, WordForm::UNSPECIFIED},
    {"\\0\\1 still\\2\\3", true, WordForm::UNSPECIFIED},
    {"\\0 still\\1\\2\\3", true, WordForm::UNSPECIFIED},
    {&FULL_STRUCTURE_HAVE, true, WordForm::PAST_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE, true, WordForm::PAST_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE_BEEN, true, WordForm::PRESENT_PARTICIPLE},
    {&FULL_STRUCTURE_HAVE_BEEN, true, WordForm::PRESENT_PARTICIPLE},
    {"\\0\\1 just\\2\\3", true, WordForm::UNSPECIFIED},
    {"\\0 just\\1\\2\\3", true, WordForm::UNSPECIFIED},
    {"\\0\\1 never\\2\\3", true, WordForm::UNSPECIFIED},
    {"\\0\\1 even\\2\\3", true, WordForm::UNSPECIFIED},
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_PRESENT = {
    .entries =
        {
            {WordForm::FIRST_PERSON_SINGULAR, "\\0 am\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 is\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 are\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_PAST = {
    .entries =
        {
            {WordForm::FIRST_PERSON_SINGULAR, "\\0 was\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 was\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 were\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_BE = {
    .entries =
        {
            {WordForm::PAST, &STRUCTURE_EXCEPTION_BE_PAST},
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_BE_PRESENT},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_NOT_PRESENT = {
    .entries =
        {
            {WordForm::FIRST_PERSON_SINGULAR, "\\0 am not\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 isn't\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 aren't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<3> STRUCTURE_EXCEPTION_BE_NOT_PAST = {
    .entries =
        {
            {WordForm::FIRST_PERSON_SINGULAR, "\\0 wasn't\\2\\3"},
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 wasn't\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 weren't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_BE_NOT = {
    .entries =
        {
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_BE_NOT_PRESENT},
            {WordForm::PAST, &STRUCTURE_EXCEPTION_BE_NOT_PAST},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 has\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 have\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE = {
    .entries =
        {
            {WordForm::PAST, "\\0 had\\2\\3"},
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_HAVE_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 hasn't\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 haven't\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT = {
    .entries =
        {
            {WordForm::PAST, "\\0 hadn't\\2\\3"},
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_HAVE_NOT_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_BEEN_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 has been\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 have been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_BEEN = {
    .entries =
        {
            {WordForm::PAST, "\\0 had been\\2\\3"},
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_HAVE_BEEN_PRESENT},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_BEEN_PRESENT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, "\\0 hasn't been\\2\\3"},
            {WordForm::UNSPECIFIED, "\\0 haven't been\\2\\3"},
        },
};

constexpr JeffPhrasingMapData<2> STRUCTURE_EXCEPTION_HAVE_NOT_BEEN = {
    .entries =
        {
            {WordForm::PAST, "\\0 hadn't been\\2\\3"},
            {WordForm::PRESENT, &STRUCTURE_EXCEPTION_HAVE_NOT_BEEN_PRESENT},
        },
};

// clang-format off
constexpr JeffPhrasingStructureException STRUCTURE_EXCEPTIONS[] = {
    {StenoStroke(), {"\\0\\2\\3", false, WordForm::UNSPECIFIED}},

    {StenoStroke(0x800 /*E*/), {&STRUCTURE_EXCEPTION_BE, false, WordForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0xC00 /**E*/), {&STRUCTURE_EXCEPTION_BE_NOT, false, WordForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0x2000 /*F*/), {&STRUCTURE_EXCEPTION_HAVE, false, WordForm::PAST_PARTICIPLE}},
    {StenoStroke(0x2400 /**F*/), {&STRUCTURE_EXCEPTION_HAVE_NOT, false, WordForm::PAST_PARTICIPLE}},
    {StenoStroke(0x2800 /*EF*/), {&STRUCTURE_EXCEPTION_HAVE_BEEN, false, WordForm::PRESENT_PARTICIPLE}},
    {StenoStroke(0x2c00 /**EF*/), {&STRUCTURE_EXCEPTION_HAVE_NOT_BEEN, false, WordForm::PRESENT_PARTICIPLE}},

    {StenoStroke(0x1800 /*EU*/), {"\\0 still\\2\\3", false, WordForm::UNSPECIFIED}},
    {StenoStroke(0x3800 /*EUF*/), {"\\0 never\\2\\3", false, WordForm::UNSPECIFIED}},
    {StenoStroke(0x3000 /*UF*/), {"\\0 just\\2\\3", false, WordForm::UNSPECIFIED}},
};

constexpr JeffPhrasingStructureException UNIQUE_STARTERS[] = {
    {StenoStroke(0x10a6 /*STWRU*/), {"to\\2\\3", false, WordForm::ROOT}},
    {StenoStroke(0x14a6 /*STWR*U*/), {"not to\\2\\3", false, WordForm::ROOT}},
    {StenoStroke(0x10fe /*STKPWHRU*/), {"to\\2\\3", false, WordForm::ROOT}},
    {StenoStroke(0x14fe /*STKPWHR*U*/), {"not to\\2\\3", false, WordForm::ROOT}},
};

// clang-format on

constexpr JeffPhrasingMapData<4> ENDER_RB = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " asks"},
            {WordForm::PAST_PARTICIPLE, " asked"},
            {WordForm::PRESENT_PARTICIPLE, " asking"},
            {WordForm::UNSPECIFIED, " ask"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBD = {
    .entries =
        {
            {WordForm::ROOT, " ask"},
            {WordForm::PAST_PARTICIPLE, " asked"},
            {WordForm::PRESENT_PARTICIPLE, " asking"},
            {WordForm::UNSPECIFIED, " asked"},
        },
};

constexpr JeffPhrasingMapData<6> ENDER_B = {
    .entries =
        {
            {WordForm::ROOT, " be"},
            {WordForm::FIRST_PERSON_SINGULAR, " am"},
            {WordForm::THIRD_PERSON_SINGULAR, " is"},
            {WordForm::PAST_PARTICIPLE, " been"},
            {WordForm::PRESENT_PARTICIPLE, " being"},
            {WordForm::UNSPECIFIED, " are"},
        },
};

constexpr JeffPhrasingMapData<6> ENDER_BD = {
    .entries =
        {
            {WordForm::ROOT, " be"},
            {WordForm::FIRST_PERSON_SINGULAR, " was"},
            {WordForm::THIRD_PERSON_SINGULAR, " was"},
            {WordForm::PAST_PARTICIPLE, " been"},
            {WordForm::PRESENT_PARTICIPLE, " being"},
            {WordForm::UNSPECIFIED, " were"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " becomes"},
            {WordForm::PAST_PARTICIPLE, " become"},
            {WordForm::PRESENT_PARTICIPLE, " becoming"},
            {WordForm::UNSPECIFIED, " become"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBGD = {
    .entries =
        {
            {WordForm::ROOT, " become"},
            {WordForm::PAST_PARTICIPLE, " become"},
            {WordForm::PRESENT_PARTICIPLE, " becoming"},
            {WordForm::UNSPECIFIED, " became"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BL = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " believes"},
            {WordForm::PAST_PARTICIPLE, " believed"},
            {WordForm::PRESENT_PARTICIPLE, " believing"},
            {WordForm::UNSPECIFIED, " believe"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLD = {
    .entries =
        {
            {WordForm::ROOT, " believe"},
            {WordForm::PAST_PARTICIPLE, " believed"},
            {WordForm::PRESENT_PARTICIPLE, " believing"},
            {WordForm::UNSPECIFIED, " believed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBLG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " calls"},
            {WordForm::PAST_PARTICIPLE, " called"},
            {WordForm::PRESENT_PARTICIPLE, " calling"},
            {WordForm::UNSPECIFIED, " call"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBLGD = {
    .entries =
        {
            {WordForm::ROOT, " call"},
            {WordForm::PAST_PARTICIPLE, " called"},
            {WordForm::PRESENT_PARTICIPLE, " calling"},
            {WordForm::UNSPECIFIED, " called"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " cares"},
            {WordForm::PAST_PARTICIPLE, " cared"},
            {WordForm::PRESENT_PARTICIPLE, " caring"},
            {WordForm::UNSPECIFIED, " care"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RDZ = {
    .entries =
        {
            {WordForm::ROOT, " care"},
            {WordForm::PAST_PARTICIPLE, " cared"},
            {WordForm::PRESENT_PARTICIPLE, " caring"},
            {WordForm::UNSPECIFIED, " cared"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " changes"},
            {WordForm::PAST_PARTICIPLE, " changed"},
            {WordForm::PRESENT_PARTICIPLE, " changing"},
            {WordForm::UNSPECIFIED, " change"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGDZ = {
    .entries =
        {
            {WordForm::ROOT, " change"},
            {WordForm::PAST_PARTICIPLE, " changed"},
            {WordForm::PRESENT_PARTICIPLE, " changing"},
            {WordForm::UNSPECIFIED, " changed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " comes"},
            {WordForm::PAST_PARTICIPLE, " come"},
            {WordForm::PRESENT_PARTICIPLE, " coming"},
            {WordForm::UNSPECIFIED, " come"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BGD = {
    .entries =
        {
            {WordForm::ROOT, " come"},
            {WordForm::PAST_PARTICIPLE, " come"},
            {WordForm::PRESENT_PARTICIPLE, " coming"},
            {WordForm::UNSPECIFIED, " came"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " considers"},
            {WordForm::PAST_PARTICIPLE, " considered"},
            {WordForm::PRESENT_PARTICIPLE, " considering"},
            {WordForm::UNSPECIFIED, " consider"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGDZ = {
    .entries =
        {
            {WordForm::ROOT, " consider"},
            {WordForm::PAST_PARTICIPLE, " considered"},
            {WordForm::PRESENT_PARTICIPLE, " considering"},
            {WordForm::UNSPECIFIED, " considered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RP = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " does"},
            {WordForm::PAST_PARTICIPLE, " done"},
            {WordForm::PRESENT_PARTICIPLE, " doing"},
            {WordForm::UNSPECIFIED, " do"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPD = {
    .entries =
        {
            {WordForm::ROOT, " do"},
            {WordForm::PAST_PARTICIPLE, " done"},
            {WordForm::PRESENT_PARTICIPLE, " doing"},
            {WordForm::UNSPECIFIED, " did"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " expects"},
            {WordForm::PAST_PARTICIPLE, " expected"},
            {WordForm::PRESENT_PARTICIPLE, " expecting"},
            {WordForm::UNSPECIFIED, " expect"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGSZ = {
    .entries =
        {
            {WordForm::ROOT, " expect"},
            {WordForm::PAST_PARTICIPLE, " expected"},
            {WordForm::PRESENT_PARTICIPLE, " expecting"},
            {WordForm::UNSPECIFIED, " expected"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " feels"},
            {WordForm::PAST_PARTICIPLE, " felt"},
            {WordForm::PRESENT_PARTICIPLE, " feeling"},
            {WordForm::UNSPECIFIED, " feel"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LTD = {
    .entries =
        {
            {WordForm::ROOT, " feel"},
            {WordForm::PAST_PARTICIPLE, " felt"},
            {WordForm::PRESENT_PARTICIPLE, " feeling"},
            {WordForm::UNSPECIFIED, " felt"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " finds"},
            {WordForm::PAST_PARTICIPLE, " found"},
            {WordForm::PRESENT_PARTICIPLE, " finding"},
            {WordForm::UNSPECIFIED, " find"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLGD = {
    .entries =
        {
            {WordForm::ROOT, " find"},
            {WordForm::PAST_PARTICIPLE, " found"},
            {WordForm::PRESENT_PARTICIPLE, " finding"},
            {WordForm::UNSPECIFIED, " found"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " forgets"},
            {WordForm::PAST_PARTICIPLE, " forgotten"},
            {WordForm::PRESENT_PARTICIPLE, " forgetting"},
            {WordForm::UNSPECIFIED, " forget"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RGD = {
    .entries =
        {
            {WordForm::ROOT, " forget"},
            {WordForm::PAST_PARTICIPLE, " forgotten"},
            {WordForm::PRESENT_PARTICIPLE, " forgetting"},
            {WordForm::UNSPECIFIED, " forgot"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " gets"},
            {WordForm::PAST_PARTICIPLE, " gotten"},
            {WordForm::PRESENT_PARTICIPLE, " getting"},
            {WordForm::UNSPECIFIED, " get"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GSZ = {
    .entries =
        {
            {WordForm::ROOT, " get"},
            {WordForm::PAST_PARTICIPLE, " gotten"},
            {WordForm::PRESENT_PARTICIPLE, " getting"},
            {WordForm::UNSPECIFIED, " got"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " gives"},
            {WordForm::PAST_PARTICIPLE, " given"},
            {WordForm::PRESENT_PARTICIPLE, " giving"},
            {WordForm::UNSPECIFIED, " give"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GDZ = {
    .entries =
        {
            {WordForm::ROOT, " give"},
            {WordForm::PAST_PARTICIPLE, " given"},
            {WordForm::PRESENT_PARTICIPLE, " giving"},
            {WordForm::UNSPECIFIED, " gave"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_G = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " goes"},
            {WordForm::PAST_PARTICIPLE, " gone"},
            {WordForm::PRESENT_PARTICIPLE, " going"},
            {WordForm::UNSPECIFIED, " go"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_GD = {
    .entries =
        {
            {WordForm::ROOT, " go"},
            {WordForm::PAST_PARTICIPLE, " gone"},
            {WordForm::PRESENT_PARTICIPLE, " going"},
            {WordForm::UNSPECIFIED, " went"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_T = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " has"},
            {WordForm::PAST_PARTICIPLE, " had"},
            {WordForm::PRESENT_PARTICIPLE, " having"},
            {WordForm::UNSPECIFIED, " have"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_TD = {
    .entries =
        {
            {WordForm::ROOT, " have"},
            {WordForm::PAST_PARTICIPLE, " had"},
            {WordForm::PRESENT_PARTICIPLE, " having"},
            {WordForm::UNSPECIFIED, " had"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " happens"},
            {WordForm::PAST_PARTICIPLE, " happened"},
            {WordForm::PRESENT_PARTICIPLE, " happening"},
            {WordForm::UNSPECIFIED, " happen"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PDZ = {
    .entries =
        {
            {WordForm::ROOT, " happen"},
            {WordForm::PAST_PARTICIPLE, " happened"},
            {WordForm::PRESENT_PARTICIPLE, " happening"},
            {WordForm::UNSPECIFIED, " happened"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " hears"},
            {WordForm::PAST_PARTICIPLE, " heard"},
            {WordForm::PRESENT_PARTICIPLE, " hearing"},
            {WordForm::UNSPECIFIED, " hear"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PGD = {
    .entries =
        {
            {WordForm::ROOT, " hear"},
            {WordForm::PAST_PARTICIPLE, " heard"},
            {WordForm::PRESENT_PARTICIPLE, " hearing"},
            {WordForm::UNSPECIFIED, " heard"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " hopes"},
            {WordForm::PAST_PARTICIPLE, " hoped"},
            {WordForm::PRESENT_PARTICIPLE, " hoping"},
            {WordForm::UNSPECIFIED, " hope"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPSZ = {
    .entries =
        {
            {WordForm::ROOT, " hope"},
            {WordForm::PAST_PARTICIPLE, " hoped"},
            {WordForm::PRESENT_PARTICIPLE, " hoping"},
            {WordForm::UNSPECIFIED, " hoped"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " imagines"},
            {WordForm::PAST_PARTICIPLE, " imagined"},
            {WordForm::PRESENT_PARTICIPLE, " imagining"},
            {WordForm::UNSPECIFIED, " imagine"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLGD = {
    .entries =
        {
            {WordForm::ROOT, " imagine"},
            {WordForm::PAST_PARTICIPLE, " imagined"},
            {WordForm::PRESENT_PARTICIPLE, " imagining"},
            {WordForm::UNSPECIFIED, " imagined"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " keeps"},
            {WordForm::PAST_PARTICIPLE, " kept"},
            {WordForm::PRESENT_PARTICIPLE, " keeping"},
            {WordForm::UNSPECIFIED, " keep"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGSZ = {
    .entries =
        {
            {WordForm::ROOT, " keep"},
            {WordForm::PAST_PARTICIPLE, " kept"},
            {WordForm::PRESENT_PARTICIPLE, " keeping"},
            {WordForm::UNSPECIFIED, " kept"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PB = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " knows"},
            {WordForm::PAST_PARTICIPLE, " known"},
            {WordForm::PRESENT_PARTICIPLE, " knowing"},
            {WordForm::UNSPECIFIED, " know"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBD = {
    .entries =
        {
            {WordForm::ROOT, " know"},
            {WordForm::PAST_PARTICIPLE, " known"},
            {WordForm::PRESENT_PARTICIPLE, " knowing"},
            {WordForm::UNSPECIFIED, " knew"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " learns"},
            {WordForm::PAST_PARTICIPLE, " learned"},
            {WordForm::PRESENT_PARTICIPLE, " learning"},
            {WordForm::UNSPECIFIED, " learn"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBSZ = {
    .entries =
        {
            {WordForm::ROOT, " learn"},
            {WordForm::PAST_PARTICIPLE, " learned"},
            {WordForm::PRESENT_PARTICIPLE, " learning"},
            {WordForm::UNSPECIFIED, " learned"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " leaves"},
            {WordForm::PAST_PARTICIPLE, " left"},
            {WordForm::PRESENT_PARTICIPLE, " leaving"},
            {WordForm::UNSPECIFIED, " leave"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGDZ = {
    .entries =
        {
            {WordForm::ROOT, " leave"},
            {WordForm::PAST_PARTICIPLE, " left"},
            {WordForm::PRESENT_PARTICIPLE, " leaving"},
            {WordForm::UNSPECIFIED, " left"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " lets"},
            {WordForm::PAST_PARTICIPLE, " let"},
            {WordForm::PRESENT_PARTICIPLE, " letting"},
            {WordForm::UNSPECIFIED, " let"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LSZ = {
    .entries =
        {
            {WordForm::ROOT, " let"},
            {WordForm::PAST_PARTICIPLE, " let"},
            {WordForm::PRESENT_PARTICIPLE, " letting"},
            {WordForm::UNSPECIFIED, " let"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " likes"},
            {WordForm::PAST_PARTICIPLE, " liked"},
            {WordForm::PRESENT_PARTICIPLE, " liking"},
            {WordForm::UNSPECIFIED, " like"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLGD = {
    .entries =
        {
            {WordForm::ROOT, " like"},
            {WordForm::PAST_PARTICIPLE, " liked"},
            {WordForm::PRESENT_PARTICIPLE, " liking"},
            {WordForm::UNSPECIFIED, " liked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " lives"},
            {WordForm::PAST_PARTICIPLE, " lived"},
            {WordForm::PRESENT_PARTICIPLE, " living"},
            {WordForm::UNSPECIFIED, " live"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LDZ = {
    .entries =
        {
            {WordForm::ROOT, " live"},
            {WordForm::PAST_PARTICIPLE, " lived"},
            {WordForm::PRESENT_PARTICIPLE, " living"},
            {WordForm::UNSPECIFIED, " lived"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_L = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " looks"},
            {WordForm::PAST_PARTICIPLE, " looked"},
            {WordForm::PRESENT_PARTICIPLE, " looking"},
            {WordForm::UNSPECIFIED, " look"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LD = {
    .entries =
        {
            {WordForm::ROOT, " look"},
            {WordForm::PAST_PARTICIPLE, " looked"},
            {WordForm::PRESENT_PARTICIPLE, " looking"},
            {WordForm::UNSPECIFIED, " looked"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " loves"},
            {WordForm::PAST_PARTICIPLE, " loved"},
            {WordForm::PRESENT_PARTICIPLE, " loving"},
            {WordForm::UNSPECIFIED, " love"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_LGD = {
    .entries =
        {
            {WordForm::ROOT, " love"},
            {WordForm::PAST_PARTICIPLE, " loved"},
            {WordForm::PRESENT_PARTICIPLE, " loving"},
            {WordForm::UNSPECIFIED, " loved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBL = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " makes"},
            {WordForm::PAST_PARTICIPLE, " made"},
            {WordForm::PRESENT_PARTICIPLE, " making"},
            {WordForm::UNSPECIFIED, " make"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBLD = {
    .entries =
        {
            {WordForm::ROOT, " make"},
            {WordForm::PAST_PARTICIPLE, " made"},
            {WordForm::PRESENT_PARTICIPLE, " making"},
            {WordForm::UNSPECIFIED, " made"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBL = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " means"},
            {WordForm::PAST_PARTICIPLE, " meant"},
            {WordForm::PRESENT_PARTICIPLE, " meaning"},
            {WordForm::UNSPECIFIED, " mean"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLD = {
    .entries =
        {
            {WordForm::ROOT, " mean"},
            {WordForm::PAST_PARTICIPLE, " meant"},
            {WordForm::PRESENT_PARTICIPLE, " meaning"},
            {WordForm::UNSPECIFIED, " meant"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " minds"},
            {WordForm::PAST_PARTICIPLE, " minded"},
            {WordForm::PRESENT_PARTICIPLE, " minding"},
            {WordForm::UNSPECIFIED, " mind"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBLSZ = {
    .entries =
        {
            {WordForm::ROOT, " mind"},
            {WordForm::PAST_PARTICIPLE, " minded"},
            {WordForm::PRESENT_PARTICIPLE, " minding"},
            {WordForm::UNSPECIFIED, " minded"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " moves"},
            {WordForm::PAST_PARTICIPLE, " moved"},
            {WordForm::PRESENT_PARTICIPLE, " moving"},
            {WordForm::UNSPECIFIED, " move"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLDZ = {
    .entries =
        {
            {WordForm::ROOT, " move"},
            {WordForm::PAST_PARTICIPLE, " moved"},
            {WordForm::PRESENT_PARTICIPLE, " moving"},
            {WordForm::UNSPECIFIED, " moved"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " needs"},
            {WordForm::PAST_PARTICIPLE, " needed"},
            {WordForm::PRESENT_PARTICIPLE, " needing"},
            {WordForm::UNSPECIFIED, " need"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPGD = {
    .entries =
        {
            {WordForm::ROOT, " need"},
            {WordForm::PAST_PARTICIPLE, " needed"},
            {WordForm::PRESENT_PARTICIPLE, " needing"},
            {WordForm::UNSPECIFIED, " needed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " puts"},
            {WordForm::PAST_PARTICIPLE, " put"},
            {WordForm::PRESENT_PARTICIPLE, " putting"},
            {WordForm::UNSPECIFIED, " put"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PSZ = {
    .entries =
        {
            {WordForm::ROOT, " put"},
            {WordForm::PAST_PARTICIPLE, " put"},
            {WordForm::PRESENT_PARTICIPLE, " putting"},
            {WordForm::UNSPECIFIED, " put"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " reads"},
            {WordForm::PAST_PARTICIPLE, " read"},
            {WordForm::PRESENT_PARTICIPLE, " reading"},
            {WordForm::UNSPECIFIED, " read"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RSZ = {
    .entries =
        {
            {WordForm::ROOT, " read"},
            {WordForm::PAST_PARTICIPLE, " read"},
            {WordForm::PRESENT_PARTICIPLE, " reading"},
            {WordForm::UNSPECIFIED, " read"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RL = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " recalls"},
            {WordForm::PAST_PARTICIPLE, " recalled"},
            {WordForm::PRESENT_PARTICIPLE, " recalling"},
            {WordForm::UNSPECIFIED, " recall"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLD = {
    .entries =
        {
            {WordForm::ROOT, " recall"},
            {WordForm::PAST_PARTICIPLE, " recalled"},
            {WordForm::PRESENT_PARTICIPLE, " recalling"},
            {WordForm::UNSPECIFIED, " recalled"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " realizes"},
            {WordForm::PAST_PARTICIPLE, " realized"},
            {WordForm::PRESENT_PARTICIPLE, " realizing"},
            {WordForm::UNSPECIFIED, " realize"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLSZ = {
    .entries =
        {
            {WordForm::ROOT, " realize"},
            {WordForm::PAST_PARTICIPLE, " realized"},
            {WordForm::PRESENT_PARTICIPLE, " realizing"},
            {WordForm::UNSPECIFIED, " realized"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPL = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " remembers"},
            {WordForm::PAST_PARTICIPLE, " remembered"},
            {WordForm::PRESENT_PARTICIPLE, " remembering"},
            {WordForm::UNSPECIFIED, " remember"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLD = {
    .entries =
        {
            {WordForm::ROOT, " remember"},
            {WordForm::PAST_PARTICIPLE, " remembered"},
            {WordForm::PRESENT_PARTICIPLE, " remembering"},
            {WordForm::UNSPECIFIED, " remembered"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " remains"},
            {WordForm::PAST_PARTICIPLE, " remained"},
            {WordForm::PRESENT_PARTICIPLE, " remaining"},
            {WordForm::UNSPECIFIED, " remain"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPLSZ = {
    .entries =
        {
            {WordForm::ROOT, " remain"},
            {WordForm::PAST_PARTICIPLE, " remained"},
            {WordForm::PRESENT_PARTICIPLE, " remaining"},
            {WordForm::UNSPECIFIED, " remained"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_R = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " runs"},
            {WordForm::PAST_PARTICIPLE, " run"},
            {WordForm::PRESENT_PARTICIPLE, " running"},
            {WordForm::UNSPECIFIED, " run"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RD = {
    .entries =
        {
            {WordForm::ROOT, " run"},
            {WordForm::PAST_PARTICIPLE, " run"},
            {WordForm::PRESENT_PARTICIPLE, " running"},
            {WordForm::UNSPECIFIED, " ran"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " says"},
            {WordForm::PAST_PARTICIPLE, " said"},
            {WordForm::PRESENT_PARTICIPLE, " saying"},
            {WordForm::UNSPECIFIED, " say"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BSZ = {
    .entries =
        {
            {WordForm::ROOT, " say"},
            {WordForm::PAST_PARTICIPLE, " said"},
            {WordForm::PRESENT_PARTICIPLE, " saying"},
            {WordForm::UNSPECIFIED, " said"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_S = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " sees"},
            {WordForm::PAST_PARTICIPLE, " seen"},
            {WordForm::PRESENT_PARTICIPLE, " seeing"},
            {WordForm::UNSPECIFIED, " see"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_SZ = {
    .entries =
        {
            {WordForm::ROOT, " see"},
            {WordForm::PAST_PARTICIPLE, " seen"},
            {WordForm::PRESENT_PARTICIPLE, " seeing"},
            {WordForm::UNSPECIFIED, " saw"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " sets"},
            {WordForm::PAST_PARTICIPLE, " set"},
            {WordForm::PRESENT_PARTICIPLE, " setting"},
            {WordForm::UNSPECIFIED, " set"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_BLSZ = {
    .entries =
        {
            {WordForm::ROOT, " set"},
            {WordForm::PAST_PARTICIPLE, " set"},
            {WordForm::PRESENT_PARTICIPLE, " setting"},
            {WordForm::UNSPECIFIED, " set"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " seems"},
            {WordForm::PAST_PARTICIPLE, " seemed"},
            {WordForm::PRESENT_PARTICIPLE, " seeming"},
            {WordForm::UNSPECIFIED, " seem"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PLSZ = {
    .entries =
        {
            {WordForm::ROOT, " seem"},
            {WordForm::PAST_PARTICIPLE, " seemed"},
            {WordForm::PRESENT_PARTICIPLE, " seeming"},
            {WordForm::UNSPECIFIED, " seemed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBZ = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " shows"},
            {WordForm::PAST_PARTICIPLE, " shown"},
            {WordForm::PRESENT_PARTICIPLE, " showing"},
            {WordForm::UNSPECIFIED, " show"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBDZ = {
    .entries =
        {
            {WordForm::ROOT, " want"},
            {WordForm::PAST_PARTICIPLE, " shown"},
            {WordForm::PRESENT_PARTICIPLE, " showing"},
            {WordForm::UNSPECIFIED, " showed"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " takes"},
            {WordForm::PAST_PARTICIPLE, " taken"},
            {WordForm::PRESENT_PARTICIPLE, " taking"},
            {WordForm::UNSPECIFIED, " take"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBTD = {
    .entries =
        {
            {WordForm::ROOT, " take"},
            {WordForm::PAST_PARTICIPLE, " taken"},
            {WordForm::PRESENT_PARTICIPLE, " taking"},
            {WordForm::UNSPECIFIED, " took"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " tells"},
            {WordForm::PAST_PARTICIPLE, " told"},
            {WordForm::PRESENT_PARTICIPLE, " telling"},
            {WordForm::UNSPECIFIED, " tell"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RLTD = {
    .entries =
        {
            {WordForm::ROOT, " tell"},
            {WordForm::PAST_PARTICIPLE, " told"},
            {WordForm::PRESENT_PARTICIPLE, " telling"},
            {WordForm::UNSPECIFIED, " told"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " thinks"},
            {WordForm::PAST_PARTICIPLE, " thought"},
            {WordForm::PRESENT_PARTICIPLE, " thinking"},
            {WordForm::UNSPECIFIED, " think"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PBGD = {
    .entries =
        {
            {WordForm::ROOT, " think"},
            {WordForm::PAST_PARTICIPLE, " thought"},
            {WordForm::PRESENT_PARTICIPLE, " thinking"},
            {WordForm::UNSPECIFIED, " thought"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RT = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " tries"},
            {WordForm::PAST_PARTICIPLE, " tried"},
            {WordForm::PRESENT_PARTICIPLE, " trying"},
            {WordForm::UNSPECIFIED, " try"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RTD = {
    .entries =
        {
            {WordForm::ROOT, " try"},
            {WordForm::PAST_PARTICIPLE, " tried"},
            {WordForm::PRESENT_PARTICIPLE, " trying"},
            {WordForm::UNSPECIFIED, " tried"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPB = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " understands"},
            {WordForm::PAST_PARTICIPLE, " understood"},
            {WordForm::PRESENT_PARTICIPLE, " understanding"},
            {WordForm::UNSPECIFIED, " understand"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RPBD = {
    .entries =
        {
            {WordForm::ROOT, " understand"},
            {WordForm::PAST_PARTICIPLE, " understood"},
            {WordForm::PRESENT_PARTICIPLE, " understanding"},
            {WordForm::UNSPECIFIED, " understood"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_Z = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " uses"},
            {WordForm::PAST_PARTICIPLE, " used"},
            {WordForm::PRESENT_PARTICIPLE, " using"},
            {WordForm::UNSPECIFIED, " use"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_DZ = {
    .entries =
        {
            {WordForm::ROOT, " use"},
            {WordForm::PAST_PARTICIPLE, " used"},
            {WordForm::PRESENT_PARTICIPLE, " using"},
            {WordForm::UNSPECIFIED, " used"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_P = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " wants"},
            {WordForm::PAST_PARTICIPLE, " wanted"},
            {WordForm::PRESENT_PARTICIPLE, " wanting"},
            {WordForm::UNSPECIFIED, " want"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_PD = {
    .entries =
        {
            {WordForm::ROOT, " want"},
            {WordForm::PAST_PARTICIPLE, " wanted"},
            {WordForm::PRESENT_PARTICIPLE, " wanting"},
            {WordForm::UNSPECIFIED, " wanted"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBS = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " wishes"},
            {WordForm::PAST_PARTICIPLE, " wished"},
            {WordForm::PRESENT_PARTICIPLE, " wishing"},
            {WordForm::UNSPECIFIED, " wish"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBSZ = {
    .entries =
        {
            {WordForm::ROOT, " wish"},
            {WordForm::PAST_PARTICIPLE, " wished"},
            {WordForm::PRESENT_PARTICIPLE, " wishing"},
            {WordForm::UNSPECIFIED, " wished"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBG = {
    .entries =
        {
            {WordForm::THIRD_PERSON_SINGULAR, " works"},
            {WordForm::PAST_PARTICIPLE, " worked"},
            {WordForm::PRESENT_PARTICIPLE, " working"},
            {WordForm::UNSPECIFIED, " work"},
        },
};

constexpr JeffPhrasingMapData<4> ENDER_RBGD = {
    .entries =
        {
            {WordForm::ROOT, " work"},
            {WordForm::PAST_PARTICIPLE, " worked"},
            {WordForm::PRESENT_PARTICIPLE, " working"},
            {WordForm::UNSPECIFIED, " worked"},
        },
};

// clang-format off
constexpr JeffPhrasingEnder ENDERS[] = {
    {StenoStroke(), WordForm::PRESENT, true, "", ""},
    {StenoStroke(0x200000 /*D*/), WordForm::PAST, true, "", ""},

    // RB - To ask
    {StenoStroke(0x14000 /*RB*/), WordForm::PRESENT, false, &ENDER_RB, ""},
    {StenoStroke(0x214000 /*-RBD*/), WordForm::PAST, false, &ENDER_RBD, ""},

    // B - To be (a)
    {StenoStroke(0x10000 /*B*/), WordForm::PRESENT, true, &ENDER_B, ""},
    {StenoStroke(0x210000 /*BD*/), WordForm::PAST, true, &ENDER_BD, ""},
    {StenoStroke(0x90000 /*BT*/), WordForm::PRESENT, true, &ENDER_B, " a"},
    {StenoStroke(0x290000 /*BTD*/), WordForm::PAST, true, &ENDER_BD, " a"},

    // RPBG - To become (a)
    {StenoStroke(0x5c000 /*-RPBG*/), WordForm::PRESENT, false, &ENDER_RPBG, ""},
    {StenoStroke(0x25c000 /*-RPBGD*/), WordForm::PAST, false, &ENDER_RPBGD, ""},
    {StenoStroke(0xdc000 /*-RPBGT*/), WordForm::PRESENT, false, &ENDER_RPBG, " a"},
    {StenoStroke(0x2dc000 /*-RPBGTD*/), WordForm::PAST, false, &ENDER_RPBGD, " a"},

    // BL - To believe (that)
    {StenoStroke(0x30000 /*-BL*/), WordForm::PRESENT, false, &ENDER_BL, ""},
    {StenoStroke(0x230000 /*-BLD*/), WordForm::PAST, false, &ENDER_BLD, ""},
    {StenoStroke(0xb0000 /*-BLT*/), WordForm::PRESENT, false, &ENDER_BL, " that"},
    {StenoStroke(0x2b0000 /*-BLTD*/), WordForm::PAST, false, &ENDER_BLD, " that"},

    // RBLG - To call
    {StenoStroke(0x74000 /*-RBLG*/), WordForm::PRESENT, false, &ENDER_RBLG, ""},
    {StenoStroke(0x274000 /*-RBLGD*/), WordForm::PAST, false, &ENDER_RBLGD, ""},

    // BGS - Can -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x150000 /*-BGS*/), WordForm::PRESENT, false, " can", ""},
    {StenoStroke(0x550000 /*-BGSZ*/), WordForm::PAST, false, " could", ""},

    // RZ - To care
    {StenoStroke(0x404000 /*-RZ*/), WordForm::PRESENT, false, &ENDER_RZ, ""},
    {StenoStroke(0x604000 /*-RDZ*/), WordForm::PAST, false, &ENDER_RDZ, ""},

    // PBGZ - To change
    {StenoStroke(0x458000 /*-PBGZ*/), WordForm::PRESENT, false, &ENDER_PBGZ, ""},
    {StenoStroke(0x658000 /*-PBGDZ*/), WordForm::PAST, false, &ENDER_PBGDZ, ""},

    // BG - To come (to)
    {StenoStroke(0x50000 /*-BG*/), WordForm::PRESENT, true, &ENDER_BG, ""},
    {StenoStroke(0x250000 /*-BGD*/), WordForm::PAST, true, &ENDER_BGD, ""},
    {StenoStroke(0xd0000 /*-BGT*/), WordForm::PRESENT, true, &ENDER_BG, " to"},
    {StenoStroke(0x2d0000 /*BGTD*/), WordForm::PAST, true, &ENDER_BGD, " to"},

    // RBGZ - To consider
    {StenoStroke(0x454000 /*-RBGZ*/), WordForm::PRESENT, false, &ENDER_RBGZ, ""},
    {StenoStroke(0x654000 /*-RBGDZ*/), WordForm::PAST, false, &ENDER_RBGDZ, ""},

    // RP - To do (it)
    {StenoStroke(0xc000 /*-RP*/), WordForm::PRESENT, false, &ENDER_RP, ""},
    {StenoStroke(0x20c000 /*-RPD*/), WordForm::PAST, false, &ENDER_RPD, ""},
    {StenoStroke(0x8c000 /*-RPT*/), WordForm::PRESENT, false, &ENDER_RP, " it"},
    {StenoStroke(0x28c000 /*-RPTD*/), WordForm::PAST, false, &ENDER_RPD, " it"},

    // RP - To expect (that)
    {StenoStroke(0x148000 /*-PGS*/), WordForm::PRESENT, false, &ENDER_PGS, ""},
    {StenoStroke(0x548000 /*-PGSZ*/), WordForm::PAST, false, &ENDER_PGSZ, ""},
    {StenoStroke(0x1c8000 /*-PGTS*/), WordForm::PRESENT, false, &ENDER_PGS, " that"},
    {StenoStroke(0x7c8000 /*-PGTSDZ*/), WordForm::PAST, false, &ENDER_PGSZ, " that"},

    // LT - To feel (like)
    {StenoStroke(0xa0000 /*-LT*/), WordForm::PRESENT, false, &ENDER_LT, ""},
    {StenoStroke(0x2a0000 /*-LTD*/), WordForm::PAST, false, &ENDER_LTD, ""},
    {StenoStroke(0x1a0000 /*-LTS*/), WordForm::PRESENT, false, &ENDER_LT, " like"},
    {StenoStroke(0x7a0000 /*-LTSDZ*/), WordForm::PAST, false, &ENDER_LTD, " like"},

    // PBLG - To find (that)
    {StenoStroke(0x78000 /*-PBLG*/), WordForm::PRESENT, false, &ENDER_PBLG, ""},
    {StenoStroke(0x278000 /*-PBLGD*/), WordForm::PAST, false, &ENDER_PBLGD, ""},
    {StenoStroke(0xf8000 /*-PBLGT*/), WordForm::PRESENT, false, &ENDER_PBLG, " that"},
    {StenoStroke(0x2f8000 /*-PBLGTD*/), WordForm::PAST, false, &ENDER_PBLGD, " that"},

    // RG - To forget (to)
    {StenoStroke(0x44000 /*-RG*/), WordForm::PRESENT, false, &ENDER_RG, ""},
    {StenoStroke(0x244000 /*-RGD*/), WordForm::PAST, false, &ENDER_RGD, ""},
    {StenoStroke(0xc4000 /*-RGT*/), WordForm::PRESENT, false, &ENDER_RG, " to"},
    {StenoStroke(0x2c4000 /*-RGTD*/), WordForm::PAST, false, &ENDER_RGD, " to"},

    // GS - To get (to)
    {StenoStroke(0x140000 /*-GS*/), WordForm::PRESENT, false, &ENDER_GS, ""},
    {StenoStroke(0x540000 /*-GSZ*/), WordForm::PAST, false, &ENDER_GSZ, ""},
    {StenoStroke(0x1c0000 /*-GTS*/), WordForm::PRESENT, false, &ENDER_GS, " to"},
    {StenoStroke(0x7c0000 /*-GTSDZ*/), WordForm::PAST, false, &ENDER_GSZ, " to"},

    // GZ - To give
    {StenoStroke(0x440000 /*-GZ*/), WordForm::PRESENT, false, &ENDER_GZ, ""},
    {StenoStroke(0x640000 /*-GDZ*/), WordForm::PAST, false, &ENDER_GDZ, ""},

    // G - To go (to)
    {StenoStroke(0x40000 /*G*/), WordForm::PRESENT, true, &ENDER_G, ""},
    {StenoStroke(0x240000 /*GD*/), WordForm::PAST, true, &ENDER_GD, ""},
    {StenoStroke(0xC0000 /*GT*/), WordForm::PRESENT, true, &ENDER_G, " to"},
    {StenoStroke(0x2C0000 /*GTD*/), WordForm::PAST, true, &ENDER_GD, " to"},

    // PZ - To happen
    {StenoStroke(0x408000 /*-PZ*/), WordForm::PRESENT, true, &ENDER_PZ, ""},
    {StenoStroke(0x608000 /*-PDZ*/), WordForm::PAST, true, &ENDER_PDZ, ""},

    // H - To have (to)
    {StenoStroke(0x80000 /*T*/), WordForm::PRESENT, true, &ENDER_T, ""},
    {StenoStroke(0x280000 /*TD*/), WordForm::PAST, true, &ENDER_TD, ""},
    {StenoStroke(0x180000 /*TS*/), WordForm::PRESENT, true, &ENDER_T, " to"},
    {StenoStroke(0x780000 /*TSDZ*/), WordForm::PAST, true, &ENDER_TD, " to"},

    // PG - To hear (that)
    {StenoStroke(0x48000 /*-PG*/), WordForm::PRESENT, false, &ENDER_PG, ""},
    {StenoStroke(0x248000 /*-PGD*/), WordForm::PAST, false, &ENDER_PGD, ""},
    {StenoStroke(0xc8000 /*-PGT*/), WordForm::PRESENT, false, &ENDER_PG, " that"},
    {StenoStroke(0x2c8000 /*-PGTD*/), WordForm::PAST, false, &ENDER_PGD, " that"},

    // RPS - To hope (to)
    {StenoStroke(0x10c000 /*-RPS*/), WordForm::PRESENT, false, &ENDER_RPS, ""},
    {StenoStroke(0x50c000 /*-RPSZ*/), WordForm::PAST, false, &ENDER_RPSZ, ""},
    {StenoStroke(0x18c000 /*-RPTS*/), WordForm::PRESENT, false, &ENDER_RPS, " to"},
    {StenoStroke(0x78c000 /*-RPTSDZ*/), WordForm::PAST, false, &ENDER_RPSZ, " to"},

    // PLG - To imagine (that)
    {StenoStroke(0x68000 /*-PLG*/), WordForm::PRESENT, false, &ENDER_PLG, ""},
    {StenoStroke(0x268000 /*-PLGD*/), WordForm::PAST, false, &ENDER_PLGD, ""},
    {StenoStroke(0xe8000 /*-PLGT*/), WordForm::PRESENT, false, &ENDER_PLG, " that"},
    {StenoStroke(0x2e8000 /*-PLGTD*/), WordForm::PAST, false, &ENDER_PLGD, " that"},

    // PBLGSZ - just
    {StenoStroke(0x578000 /*-PBLGSZ*/), WordForm::PRESENT, true, " just", ""},
    {StenoStroke(0x7f8000 /*-PBLGTSDZ*/), WordForm::PAST, true, " just", ""},

    // PBGS - To keep
    {StenoStroke(0x158000 /*-PBGS*/), WordForm::PRESENT, false, &ENDER_PBGS, ""},
    {StenoStroke(0x558000 /*-PBGSZ*/), WordForm::PAST, false, &ENDER_PBGSZ, ""},

    // PB - To know (that)
    {StenoStroke(0x18000 /*-PB*/), WordForm::PRESENT, false, &ENDER_PB, ""},
    {StenoStroke(0x218000 /*-PBD*/), WordForm::PAST, false, &ENDER_PBD, ""},
    {StenoStroke(0x98000 /*-PBT*/), WordForm::PRESENT, false, &ENDER_PB, " that"},
    {StenoStroke(0x298000 /*-PBTD*/), WordForm::PAST, false, &ENDER_PBD, " that"},

    // RPBS - To learn (to)
    {StenoStroke(0x11c000 /*-RPBS*/), WordForm::PRESENT, false, &ENDER_RPBS, ""},
    {StenoStroke(0x51c000 /*-RPBSZ*/), WordForm::PAST, false, &ENDER_RPBSZ, ""},
    {StenoStroke(0x19c000 /*-RPBTS*/), WordForm::PRESENT, false, &ENDER_RPBS, " to"},
    {StenoStroke(0x79c000 /*-RPBTSDZ*/), WordForm::PAST, false, &ENDER_RPBSZ, " to"},

    // LGZ - To leave
    {StenoStroke(0x460000 /*-LGZ*/), WordForm::PRESENT, false, &ENDER_LGZ, ""},
    {StenoStroke(0x660000 /*-LGDZ*/), WordForm::PAST, false, &ENDER_LGDZ, ""},

    // LS - To let
    {StenoStroke(0x120000 /*-LS*/), WordForm::PRESENT, false, &ENDER_LS, ""},
    {StenoStroke(0x520000 /*-LSZ*/), WordForm::PAST, false, &ENDER_LSZ, ""},

    // BLG - To like (to)
    {StenoStroke(0x70000 /*-BLG*/), WordForm::PRESENT, false, &ENDER_BLG, ""},
    {StenoStroke(0x270000 /*-BLGD*/), WordForm::PAST, false, &ENDER_BLGD, ""},
    {StenoStroke(0xf0000 /*-BLGT*/), WordForm::PRESENT, false, &ENDER_BLG, " to"},
    {StenoStroke(0x2f0000 /*-BLGTD*/), WordForm::PAST, false, &ENDER_BLGD, " to"},

    // LZ - To live
    {StenoStroke(0x420000 /*-LZ*/), WordForm::PRESENT, true, &ENDER_LZ, ""},
    {StenoStroke(0x620000 /*-LDZ*/), WordForm::PAST, true, &ENDER_LDZ, ""},

    // L - To look
    {StenoStroke(0x20000 /*-L*/), WordForm::PRESENT, false, &ENDER_L, ""},
    {StenoStroke(0x220000 /*-LD*/), WordForm::PAST, false, &ENDER_LD, ""},

    // LG - To love (to)
    {StenoStroke(0x60000 /*-LG*/), WordForm::PRESENT, false, &ENDER_LG, ""},
    {StenoStroke(0x260000 /*-LGD*/), WordForm::PAST, false, &ENDER_LGD, ""},
    {StenoStroke(0xe0000 /*-LGT*/), WordForm::PRESENT, false, &ENDER_LG, " to"},
    {StenoStroke(0x2e0000 /*-LGTD*/), WordForm::PAST, false, &ENDER_LGD, " to"},

    // RPBL - To make (a)
    {StenoStroke(0x3c000 /*-RPBL*/), WordForm::PRESENT, false, &ENDER_RPBL, ""},
    {StenoStroke(0x23c000 /*-RPBLD*/), WordForm::PAST, false, &ENDER_RPBLD, ""},
    {StenoStroke(0xbc000 /*-RPBLT*/), WordForm::PRESENT, false, &ENDER_RPBL, " a"},
    {StenoStroke(0x2bc000 /*-RPBLTD*/), WordForm::PAST, false, &ENDER_RPBLD, " a"},

    // PL - may (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x28000 /*-PL*/), WordForm::PRESENT, true, " may", ""},
    {StenoStroke(0x228000 /*-PLD*/), WordForm::PAST, true, " might", ""},
    {StenoStroke(0xa8000 /*-PLT*/), WordForm::PRESENT, true, " may", " be"},
    {StenoStroke(0x2a8000 /*-PLTD*/), WordForm::PAST, true, " might", " be"},

    // PBL - To mean (to)
    {StenoStroke(0x38000 /*-PBL*/), WordForm::PRESENT, false, &ENDER_PBL, ""},
    {StenoStroke(0x238000 /*-PBLD*/), WordForm::PAST, false, &ENDER_PBLD, ""},
    {StenoStroke(0xb8000 /*-PBLT*/), WordForm::PRESENT, false, &ENDER_PBL, " to"},
    {StenoStroke(0x2b8000 /*-PBLTD*/), WordForm::PAST, false, &ENDER_PBLD, " to"},

    // PBLS - To mind
    {StenoStroke(0x138000 /*-PBLS*/), WordForm::PRESENT, false, &ENDER_PBLS, ""},
    {StenoStroke(0x538000 /*-PBLSZ*/), WordForm::PAST, false, &ENDER_PBLSZ, ""},

    // PLZ - To move
    {StenoStroke(0x428000 /*-PLZ*/), WordForm::PRESENT, false, &ENDER_PLZ, ""},
    {StenoStroke(0x628000 /*-PLDZ*/), WordForm::PAST, false, &ENDER_PLDZ, ""},

    // PBLGS - must (be) - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x178000 /*-PBLGS*/), WordForm::PRESENT, true, " must", ""},
    {StenoStroke(0x1f8000 /*-PBLGTS*/), WordForm::PRESENT, true, " must", " be"},

    // RPG - To need (to)
    {StenoStroke(0x4c000 /*-RPG*/), WordForm::PRESENT, true, &ENDER_RPG, ""},
    {StenoStroke(0x24c000 /*-RPGD*/), WordForm::PAST, true, &ENDER_RPGD, ""},
    {StenoStroke(0xcc000 /*-RPGT*/), WordForm::PRESENT, true, &ENDER_RPG, " to"},
    {StenoStroke(0x2cc000 /*-RPGTD*/), WordForm::PAST, true, &ENDER_RPGD, " to"},

    // PS - To put (it)
    {StenoStroke(0x108000 /*-PS*/), WordForm::PRESENT, false, &ENDER_PS, ""},
    {StenoStroke(0x508000 /*-PSZ*/), WordForm::PAST, false, &ENDER_PSZ, ""},
    {StenoStroke(0x188000 /*-PTS*/), WordForm::PRESENT, false, &ENDER_PS, " it"},
    {StenoStroke(0x788000 /*-PTSDZ*/), WordForm::PAST, false, &ENDER_PSZ, " it"},

    // RS - To read
    {StenoStroke(0x104000 /*-RS*/), WordForm::PRESENT, false, &ENDER_RS, ""},
    {StenoStroke(0x504000 /*-RSZ*/), WordForm::PAST, false, &ENDER_RSZ, ""},

    // RLG - really
    {StenoStroke(0x64000 /*-RLG*/), WordForm::PRESENT, true, " really", ""},
    {StenoStroke(0x264000 /*-RLG*/), WordForm::PAST, true, " really", ""},

    // RL - To recall
    {StenoStroke(0x24000 /*-RL*/), WordForm::PRESENT, false, &ENDER_RL, ""},
    {StenoStroke(0x224000 /*-RLD*/), WordForm::PAST, false, &ENDER_RLD, ""},

    // RLS - To realize (that)
    {StenoStroke(0x124000 /*-RLS*/), WordForm::PRESENT, false, &ENDER_RLS, ""},
    {StenoStroke(0x524000 /*-RLSZ*/), WordForm::PAST, false, &ENDER_RLSZ, ""},
    {StenoStroke(0x1a4000 /*-RLTS*/), WordForm::PRESENT, false, &ENDER_RLS, " that"},
    {StenoStroke(0x7a4000 /*-RLTSDZ*/), WordForm::PAST, false, &ENDER_RLSZ, " that"},

    // RPL - To remember (that)
    {StenoStroke(0x2c000 /*-RPL*/), WordForm::PRESENT, false, &ENDER_RPL, ""},
    {StenoStroke(0x22c000 /*-RPLD*/), WordForm::PAST, false, &ENDER_RPLD, ""},
    {StenoStroke(0xac000 /*-RPLT*/), WordForm::PRESENT, false, &ENDER_RPL, " that"},
    {StenoStroke(0x2ac000 /*-RPLTD*/), WordForm::PAST, false, &ENDER_RPLD, " that"},

    // RPLS - To remain
    {StenoStroke(0x12c000 /*-RPLS*/), WordForm::PRESENT, false, &ENDER_RPLS, ""},
    {StenoStroke(0x52c000 /*-RPLSZ*/), WordForm::PAST, false, &ENDER_RPLSZ, ""},

    // R - To run
    {StenoStroke(0x4000 /*-R*/), WordForm::PRESENT, false, &ENDER_R, ""},
    {StenoStroke(0x204000 /*-RD*/), WordForm::PAST, false, &ENDER_RD, ""},

    // BS - To say (that)
    {StenoStroke(0x110000 /*-BS*/), WordForm::PRESENT, false, &ENDER_BS, ""},
    {StenoStroke(0x510000 /*-BSZ*/), WordForm::PAST, false, &ENDER_BSZ, ""},
    {StenoStroke(0x190000 /*-BTS*/), WordForm::PRESENT, false, &ENDER_BS, " that"},
    {StenoStroke(0x790000 /*-BTSDZ*/), WordForm::PAST, false, &ENDER_BSZ, " that"},

    // S - To see
    {StenoStroke(0x100000 /*-S*/), WordForm::PRESENT, false, &ENDER_S, ""},
    {StenoStroke(0x500000 /*-SZ*/), WordForm::PAST, false, &ENDER_SZ, ""},

    // BLS - To set
    {StenoStroke(0x130000 /*-BLS*/), WordForm::PRESENT, false, &ENDER_BLS, ""},
    {StenoStroke(0x530000 /*-BLSZ*/), WordForm::PAST, false, &ENDER_BLSZ, ""},

    // PLS - To seem (to)
    {StenoStroke(0x128000 /*-PLS*/), WordForm::PRESENT, true, &ENDER_PLS, ""},
    {StenoStroke(0x528000 /*-PLSZ*/), WordForm::PAST, true, &ENDER_PLSZ, ""},
    {StenoStroke(0x1a8000 /*-PLTS*/), WordForm::PRESENT, true, &ENDER_PLS, " to"},
    {StenoStroke(0x7a8000 /*-PLTSDZ*/), WordForm::PAST, true, &ENDER_PLSZ, " to"},

    // RBL - shall - Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x34000 /*RBL*/), WordForm::PRESENT, false, " shall", ""},
    {StenoStroke(0x234000 /*RBLD*/), WordForm::PAST, false, " should", ""},

    // RBZ - To show
    {StenoStroke(0x414000 /*-RBZ*/), WordForm::PRESENT, false, &ENDER_RBZ, ""},
    {StenoStroke(0x614000 /*-RBDZ*/), WordForm::PAST, false, &ENDER_RBDZ, ""},

    // RBT - To take
    {StenoStroke(0x94000 /*-RBT*/), WordForm::PRESENT, false, &ENDER_RBT, ""},
    {StenoStroke(0x294000 /*-RBTD*/), WordForm::PAST, false, &ENDER_RBTD, ""},

    // RLT - To tell
    {StenoStroke(0xa4000 /*-RLT*/), WordForm::PRESENT, false, &ENDER_RLT, ""},
    {StenoStroke(0x2a4000 /*-RLTD*/), WordForm::PAST, false, &ENDER_RLTD, ""},

    // PBG - To think (that)
    {StenoStroke(0x58000 /*-PBG*/), WordForm::PRESENT, false, &ENDER_PBG, ""},
    {StenoStroke(0x258000 /*-PBGD*/), WordForm::PAST, false, &ENDER_PBGD, ""},
    {StenoStroke(0xd8000 /*-PBGT*/), WordForm::PRESENT, false, &ENDER_PBG, " that"},
    {StenoStroke(0x2d8000 /*-PBGTD*/), WordForm::PAST, false, &ENDER_PBGD, " that"},

    // RT - To try (to)
    {StenoStroke(0x84000 /*-RT*/), WordForm::PRESENT, false, &ENDER_RT, ""},
    {StenoStroke(0x284000 /*-RTD*/), WordForm::PAST, false, &ENDER_RTD, ""},
    {StenoStroke(0x184000 /*-RTS*/), WordForm::PRESENT, false, &ENDER_RT, " to"},
    {StenoStroke(0x784000 /*-RTSDZ*/), WordForm::PAST, false, &ENDER_RTD, " to"},

    // RPB - To understand (the)
    {StenoStroke(0x1c000 /*-RPB*/), WordForm::PRESENT, false, &ENDER_RPB, ""},
    {StenoStroke(0x21c000 /*-RPBD*/), WordForm::PAST, false, &ENDER_RPBD, ""},
    {StenoStroke(0x9c000 /*-RPBT*/), WordForm::PRESENT, false, &ENDER_RPB, " the"},
    {StenoStroke(0x29c000 /*-RPBTD*/), WordForm::PAST, false, &ENDER_RPBD, " the"},

    // Z - To use
    {StenoStroke(0x400000 /*-Z*/), WordForm::PRESENT, true, &ENDER_Z, ""},
    {StenoStroke(0x600000 /*-DZ*/), WordForm::PAST, true, &ENDER_DZ, ""},
    // TZ - Special case
    {StenoStroke(0x480000 /*-TZ*/), WordForm::PRESENT, true, " used", " to"},
    {StenoStroke(0x680000 /*-TDZ*/), WordForm::PAST, true, " used", " to"},

    // P - To want (to)
    {StenoStroke(0x8000 /*P*/), WordForm::PRESENT, false, &ENDER_P, ""},
    {StenoStroke(0x208000 /*PD*/), WordForm::PAST, false, &ENDER_PD, ""},
    {StenoStroke(0x88000 /*PT*/), WordForm::PRESENT, false, &ENDER_P, " to"},
    {StenoStroke(0x288000 /*PTD*/), WordForm::PAST, false, &ENDER_PD, " to"},

    // RBGS - will -- Auxiliary verb
    // These do not combine naturally with middle/structures.
    {StenoStroke(0x154000 /*RBGS*/), WordForm::PRESENT, false, " will", ""},
    {StenoStroke(0x554000 /*RBGSZ*/), WordForm::PAST, false, " would", ""},

    // RBS - To wish (to)
    {StenoStroke(0x114000 /*-RBS*/), WordForm::PRESENT, false, &ENDER_RBS, ""},
    {StenoStroke(0x514000 /*-RBSZ*/), WordForm::PAST, false, &ENDER_RBSZ, ""},
    {StenoStroke(0x194000 /*-RBTS*/), WordForm::PRESENT, false, &ENDER_RBS, " to"},
    {StenoStroke(0x794000 /*-RBTSDZ*/), WordForm::PAST, false, &ENDER_RBSZ, " to"},

    // RBG - To work (on)
    {StenoStroke(0x54000 /*-RBG*/), WordForm::PRESENT, false, &ENDER_RBG, ""},
    {StenoStroke(0x254000 /*-RBGD*/), WordForm::PAST, false, &ENDER_RBGD, ""},
    {StenoStroke(0xd4000 /*-RBGT*/), WordForm::PRESENT, false, &ENDER_RBG, " on"},
    {StenoStroke(0x2d4000 /*-RBGTD*/), WordForm::PAST, false, &ENDER_RBGD, " on"},
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

constexpr JeffPhrasingDictionaryData JeffPhrasingDictionaryData::instance = {
    .simpleStarters =
        {
            sizeof(SIMPLE_STARTERS) / sizeof(*SIMPLE_STARTERS),
            SIMPLE_STARTERS,
        },

    .simplePronouns = SIMPLE_PRONOUNS,
    .simpleStructures = SIMPLE_STRUCTURES,

    .fullStarters =
        {
            sizeof(FULL_STARTERS) / sizeof(*FULL_STARTERS),
            FULL_STARTERS,
        },

    .fullMiddles = MIDDLES,
    .fullStructures = STRUCTURES,

    .structureExceptions =
        {
            sizeof(STRUCTURE_EXCEPTIONS) / sizeof(*STRUCTURE_EXCEPTIONS),
            STRUCTURE_EXCEPTIONS,
        },

    .enders = {sizeof(ENDERS) / sizeof(*ENDERS), ENDERS},

    .nonPhraseStrokes =
        {
            sizeof(NON_PHRASE_STROKES) / sizeof(*NON_PHRASE_STROKES),
            NON_PHRASE_STROKES,
        },

    .uniqueStarters =
        {
            sizeof(UNIQUE_STARTERS) / sizeof(*UNIQUE_STARTERS),
            UNIQUE_STARTERS,
        },

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
#include "../crc32.h"
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

  static constexpr size_t HASH_SIZE = 512;

  const JeffPhrasingEnder **enderMap = new const JeffPhrasingEnder *[512];
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    enderMap[i] = nullptr;
  }

  for (const JeffPhrasingEnder &ender : phrasingData.enders) {
    size_t index = ender.stroke.Hash() & (HASH_SIZE - 1);

    while (enderMap[index] != nullptr) {
      index = (index + 1) & (HASH_SIZE - 1);
    }
    enderMap[index] = &ender;
  }

  printf("const size_t ENDER_HASH_MAP_SIZE = %zu;\n", HASH_SIZE);
  printf("const JeffPhrasingEnder *const ENDER_HASH_MAP[] = {\n");
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    if (enderMap[i] == nullptr) {
      printf("  nullptr,\n");
    } else {
      size_t offset = enderMap[i] - phrasingData.enders.data;
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
  uint32_t starterReplacement = Crc32::Hash("\\0", 2);
  for (const JeffPhrasingFullStarter &fullStarter : phrasingData.fullStarters) {
    AddText(fullStarter.pronoun.word, fullStarter.stroke, starterReplacement,
            ComponentMask::STARTER,
            ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (size_t i = 1; i < 8; ++i) {
    const JeffPhrasingPronoun &pronoun = phrasingData.simplePronouns[i];
    AddText(pronoun.word, StenoStroke(uint32_t(i) << StrokeBitIndex::STAR),
            starterReplacement, ComponentMask::STARTER,
            ModeMask::SIMPLE | ModeMask::PRESENT | ModeMask::PAST);
  }

  uint32_t middleReplacement = Crc32::Hash("\\1", 2);
  for (size_t i = 0; i < 8; ++i) {
    RecurseVariant(phrasingData.fullMiddles[i].word,
                   StenoStroke(uint32_t(i) << StrokeBitIndex::A),
                   middleReplacement, ComponentMask::MIDDLE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  for (const JeffPhrasingSimpleStarter &simpleStarter :
       phrasingData.simpleStarters) {
    RecurseVariant(simpleStarter.middle.word, simpleStarter.stroke,
                   middleReplacement, ComponentMask::MIDDLE,
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

  for (const JeffPhrasingStructureException &structure :
       phrasingData.structureExceptions) {
    RecurseVariant(structure.structure.format, structure.stroke, 0,
                   ComponentMask::MIDDLE | ComponentMask::STRUCTURE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  for (const JeffPhrasingStructureException &uniqueStarter :
       phrasingData.uniqueStarters) {
    RecurseVariant(uniqueStarter.structure.format, uniqueStarter.stroke, 0,
                   ComponentMask::STARTER | ComponentMask::MIDDLE |
                       ComponentMask::STRUCTURE,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST,
                   nullptr);
  }

  uint32_t verbReplacement = Crc32::Hash("\\2", 2);
  uint32_t suffixReplacement = Crc32::Hash("\\3", 2);

  for (const JeffPhrasingEnder &ender : phrasingData.enders) {
    uint8_t modeMask = ModeMask::FULL | ModeMask::SIMPLE;
    if (uint32_t(ender.tense) & uint32_t(WordForm::PAST)) {
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
    for (const JeffPhrasingMapEntry &entry : map->entries) {
      uint8_t localModeMask = modeMask;
      if (entry.key & (uint32_t)WordForm::PRESENT) {
        localModeMask &= ~ModeMask::PAST;
      }
      if (entry.key & (uint32_t)WordForm::PAST) {
        localModeMask &= ~ModeMask::PRESENT;
      }
      RecurseVariant(entry.value, stroke, replacement, componentMask,
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
  entry.hash = Crc32::Hash(p, pEnd - p);
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
    entry.hash = Crc32::Hash(p, space - p);
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

  for (const JeffPhrasingStructureException &structureException :
       phrasingData.structureExceptions) {
    RecurseVariant(structureException.structure.format,
                   structureException.stroke,
                   ModeMask::FULL | ModeMask::PRESENT | ModeMask::PAST);
  }

  for (const JeffPhrasingStructureException &uniqueStarter :
       phrasingData.uniqueStarters) {
    RecurseVariant(uniqueStarter.structure.format, uniqueStarter.stroke,
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
    for (const JeffPhrasingMapEntry &entry : map->entries) {
      uint8_t localModeMask = modeMask;
      if (entry.key == (uint32_t)WordForm::PRESENT) {
        localModeMask &= ~ModeMask::PAST;
      }
      if (entry.key == (uint32_t)WordForm::PAST) {
        localModeMask &= ~ModeMask::PRESENT;
      }
      RecurseVariant(entry.value, stroke, localModeMask);
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
        hash += Crc32::Hash(p, 2);
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
      hash += Crc32::Hash(p, pEnd - p);
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
      Crc32::Hash("\\0", 2));
  printf("const uint32_t MIDDLE_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\1\"\n",
         Crc32::Hash("\\1", 2));
  printf("const uint32_t VERB_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\2\"\n",
         Crc32::Hash("\\2", 2));
  printf("const uint32_t SUFFIX_REPLACEMENT_CRC = 0x%08x; // CRC for \"\\3\"\n",
         Crc32::Hash("\\3", 2));

  GenerateLookupTable();

  ReverseBuilder reverseBuilder;
  reverseBuilder.Generate();

  ReverseStructureBuilder reverseStructureBuilder;
  reverseStructureBuilder.Generate();

  PrintCharacters();
  return 0;
}

#endif
