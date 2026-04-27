#ifndef AWQAT_CONFIG_H_
#define AWQAT_CONFIG_H_

#define ALADHAN_TIMINGS_URL "https://api.aladhan.com/v1/timings/"
#define IP_API_URL  "http://ip-api.com/json/"

// Methods

typedef struct {
  int index;
  const char* alias;
  const char* full_name;
} Method;

static Method methods[] = {
  {  0, "jafari",   "Jafari / Shia Ithna-Ashari"},
  {  1, "karachi",  "University of Islamic Sciences, Karachi"},
  {  2, "isna",     "Islamic Society of North America"},
  {  3, "mwl",      "Muslim World League"},
  {  4, "uqu",      "Umm Al-Qura University, Makkah"},
  {  5, "mwri",     "Egyptian General Authority of Survey"},
  {  7, "tehran",   "Institute of Geophysics, University of Tehran"},
  {  8, "gulf",     "Gulf Region"},
  {  9, "kuwait",   "Kuwait"},
  { 10, "qatar",    "Qatar"},
  { 11, "muis",     "Majlis Ugama Islam Singapura, Singapore"},
  { 12, "france",   "Union Organization islamic de France"},
  { 13, "turkey",   "Diyanet İşleri Başkanlığı, Turkey"},
  { 14, "russia",   "Spiritual Administration of Muslims of Russia"},
  { 15, "mcw",      "Moonsighting Committee Worldwide (also requires shafaq parameter)"},
  { 16, "dubai",    "Dubai (experimental)"},
  { 17, "jakim",    "Jabatan Kemajuan Islam Malaysia (JAKIM)"},
  { 18, "tunisia",  "Tunisia"},
  { 19, "algeria",  "Algeria"},
  { 20, "kemenag",  "KEMENAG - Kementerian Agama Republik Indonesia"},
  { 21, "morocco",  "Morocco"},
  { 22, "cil",      "Comunidade Islamica de Lisboa"},
  { 23, "jordan",   "Ministry of Awqaf, Islamic Affairs and Holy Places, Jordan"},
};

#endif // AWQAT_CONFIG_H_
