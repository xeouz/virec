#include "keyword_hash.hpp"

namespace vire
{
  inline unsigned int
Perfect_Hash::hash (const char *str, size_t len)
{
  static const unsigned char asso_values[] =
    {
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42,  5, 30,  0,
      25,  0,  5, 42, 42,  0, 42, 42, 10, 42,
       0, 10, 10, 42,  5, 15, 10, 10, 30, 25,
      42,  5, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
      42, 42, 42, 42, 42, 42
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[2])];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

static const struct KeywordCode wordlist[] =
  {
#line 25 "keywords.gperf"
    {"if", KeywordTokenCode::kw_if},
#line 29 "keywords.gperf"
    {"const", KeywordTokenCode::kw_const},
#line 48 "keywords.gperf"
    {"except", KeywordTokenCode::kw_except},
#line 32 "keywords.gperf"
    {"as", KeywordTokenCode::kw_as},
#line 43 "keywords.gperf"
    {"continue", KeywordTokenCode::kw_continue},
#line 22 "keywords.gperf"
    {"func", KeywordTokenCode::kw_func},
#line 35 "keywords.gperf"
    {"class", KeywordTokenCode::kw_class},
#line 50 "keywords.gperf"
    {"constructor", KeywordTokenCode::kw_constructor},
#line 24 "keywords.gperf"
    {"or", KeywordTokenCode::kw_or},
#line 39 "keywords.gperf"
    {"for", KeywordTokenCode::kw_for},
#line 36 "keywords.gperf"
    {"union", KeywordTokenCode::kw_union},
#line 38 "keywords.gperf"
    {"extern", KeywordTokenCode::kw_extern},
#line 46 "keywords.gperf"
    {"extends", KeywordTokenCode::kw_extends},
#line 47 "keywords.gperf"
    {"try", KeywordTokenCode::kw_try},
#line 26 "keywords.gperf"
    {"else", KeywordTokenCode::kw_else},
#line 31 "keywords.gperf"
    {"false", KeywordTokenCode::kw_false},
#line 41 "keywords.gperf"
    {"return", KeywordTokenCode::kw_return},
#line 44 "keywords.gperf"
    {"returns", KeywordTokenCode::kw_returns},
#line 28 "keywords.gperf"
    {"let", KeywordTokenCode::kw_let},
#line 30 "keywords.gperf"
    {"true", KeywordTokenCode::kw_true},
#line 45 "keywords.gperf"
    {"proto", KeywordTokenCode::kw_proto},
#line 37 "keywords.gperf"
    {"struct", KeywordTokenCode::kw_struct},
#line 33 "keywords.gperf"
    {"new", KeywordTokenCode::kw_new},
#line 40 "keywords.gperf"
    {"while", KeywordTokenCode::kw_while},
#line 49 "keywords.gperf"
    {"unsafe", KeywordTokenCode::kw_unsafe},
#line 23 "keywords.gperf"
    {"and", KeywordTokenCode::kw_and},
#line 42 "keywords.gperf"
    {"break", KeywordTokenCode::kw_break},
#line 27 "keywords.gperf"
    {"var", KeywordTokenCode::kw_var},
#line 34 "keywords.gperf"
    {"delete", KeywordTokenCode::kw_delete}
  };

static const signed char lookup[] =
  {
    -1, -1,  0, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, -1,
    22, -1, 23, 24, -1, 25, -1, 26, -1, -1, 27, -1, -1, 28
  };

int fast_compare( const char *ptr0, const char *ptr1){
  
}

const struct KeywordCode *
Perfect_Hash::hash_keyword_to_token (const char *str, size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          int index = lookup[key];

          if (index >= 0)
            {
              const char *s = wordlist[index].Keyword;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
}