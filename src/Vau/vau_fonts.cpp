
/******************************************************************************
* MODULE     : vau_fonts.cpp
* DESCRIPTION: Latin Modern in the place of the TeX fonts
* COPYRIGHT  : (C) 2026  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
*******************************************************************************
* Vau has neither the Metafont plugin of TeXmacs nor the TeX fonts (TFM, PK
* and Type 1 files). The functions of that plugin which the rest of the code
* calls are defined here, on the OpenType fonts of the Latin Modern family:
* the fonts asked by their TeX names ("ecrm", "cmr", "ecss"...), which the
* typesetter uses for its own marks (errors, flags, missing references) and
* the interface for its default font. The family "roman" of the documents is
* turned into Latin Modern by the smart font (roman_fix in smart_font.cpp).
******************************************************************************/

#include "font.hpp"
#include "analyze.hpp"
#include "Freetype/tt_file.hpp"
#include "Metafont/tex_files.hpp"

/******************************************************************************
* Fonts of letters of another alphabet
*******************************************************************************
* TeX has fonts whose letters are calligraphic (cmsy, rsfs), fraktur (eufm)
* or blackboard bold (msbm, bbm): "A" in such a font is what Unicode calls a
* mathematical script, fraktur or double-struck capital A. An alphabet font
* draws the letters of its strings as the symbols <cal-A>, <frak-A>, <bbb-A>
* of a base font, which is Latin Modern Math.
******************************************************************************/

struct alphabet_font_rep: font_rep {
  font   base;
  string kind;   // "cal", "frak" or "bbb"

  alphabet_font_rep (string name, font base, string kind);
  string rewrite (string s);
  string rewrite (string s, array<int>& where);

  bool   supports (string c);
  void   get_extents (string s, metric& ex);
  void   get_xpositions (string s, SI* xpos);
  void   get_xpositions (string s, SI* xpos, bool lig);
  void   get_xpositions (string s, SI* xpos, SI xk);
  void   draw_fixed (renderer ren, string s, SI x, SI y);
  void   draw_fixed (renderer ren, string s, SI x, SI y, bool ligf);
  void   draw_fixed (renderer ren, string s, SI x, SI y, SI xk);
  font   magnify (double zoomx, double zoomy);
  void   advance_glyph (string s, int& pos, bool ligf);
  glyph  get_glyph (string s);
  int    index_glyph (string s, font_metric& fnm, font_glyphs& fng);
  double get_left_slope  (string s);
  double get_right_slope (string s);
  SI     get_left_correction  (string s);
  SI     get_right_correction (string s);
  SI     get_lsub_correction  (string s);
  SI     get_lsup_correction  (string s);
  SI     get_rsub_correction  (string s);
  SI     get_rsup_correction  (string s);
  SI     get_wide_correction  (string s, int mode);
};

static font alphabet_font (font base, string kind);

alphabet_font_rep::alphabet_font_rep (string name, font base2, string kind2):
  font_rep (name, base2), base (base2), kind (kind2)
{
  // not an OpenType math font for the smart font: it would take the single
  // letters of the formulas from the math italic alphabet of the base font
  math_type= MATH_TYPE_NORMAL;
  ot_math= false;
}

string
alphabet_font_rep::rewrite (string s, array<int>& where) {
  // the string with its letters as symbols; where[i] is the position in it
  // of the character at position i of s (and where[N(s)] its length)
  string r;
  where= array<int> (N(s) + 1);
  int i= 0;
  while (i < N(s)) {
    int start= i;
    tm_char_forwards (s, i);
    for (int j= start; j < i; j++) where[j]= N(r);
    string c= s (start, i);
    bool letter= N(c) == 1 && (is_upcase (c[0]) ||
                               (kind != "cal" && is_locase (c[0])));
    if (kind == "cal" && N(c) == 1 && is_locase (c[0])) {
      // cmsy has delimiters in the place of the small letters, and the
      // typesetter asks for them there (concater_rep::ghost)
      switch (c[0]) {
      case 'f': c= "{"; break;
      case 'g': c= "}"; break;
      case 'h': c= "<langle>"; break;
      case 'i': c= "<rangle>"; break;
      case 'j': c= "|"; break;
      case 'n': c= "\\"; break;
      }
    }
    else if (letter) {
      string sym= "<" * kind * "-" * c * ">";
      if (base->supports (sym)) c= sym;
    }
    r << c;
  }
  where[N(s)]= N(r);
  return r;
}

string
alphabet_font_rep::rewrite (string s) {
  array<int> where;
  return rewrite (s, where);
}

bool
alphabet_font_rep::supports (string c) {
  return base->supports (rewrite (c));
}

void
alphabet_font_rep::get_extents (string s, metric& ex) {
  base->get_extents (rewrite (s), ex);
}

#define ALPHABET_XPOSITIONS(CALL) \
  array<int> where; \
  string r= rewrite (s, where); \
  STACK_NEW_ARRAY (rpos, SI, N(r) + 1); \
  for (int i= 0; i <= N(r); i++) rpos[i]= 0; \
  CALL; \
  for (int i= 0; i <= N(s); i++) xpos[i]= rpos[where[i]]; \
  STACK_DELETE_ARRAY (rpos);

void
alphabet_font_rep::get_xpositions (string s, SI* xpos) {
  ALPHABET_XPOSITIONS (base->get_xpositions (r, rpos));
}

void
alphabet_font_rep::get_xpositions (string s, SI* xpos, bool lig) {
  ALPHABET_XPOSITIONS (base->get_xpositions (r, rpos, lig));
}

void
alphabet_font_rep::get_xpositions (string s, SI* xpos, SI xk) {
  ALPHABET_XPOSITIONS (base->get_xpositions (r, rpos, xk));
}

void
alphabet_font_rep::draw_fixed (renderer ren, string s, SI x, SI y) {
  base->draw_fixed (ren, rewrite (s), x, y);
}

void
alphabet_font_rep::draw_fixed (renderer ren, string s, SI x, SI y, bool lf) {
  base->draw_fixed (ren, rewrite (s), x, y, lf);
}

void
alphabet_font_rep::draw_fixed (renderer ren, string s, SI x, SI y, SI xk) {
  base->draw_fixed (ren, rewrite (s), x, y, xk);
}

font
alphabet_font_rep::magnify (double zoomx, double zoomy) {
  return alphabet_font (base->magnify (zoomx, zoomy), kind);
}

void
alphabet_font_rep::advance_glyph (string s, int& pos, bool ligf) {
  (void) ligf;
  tm_char_forwards (s, pos);
}

glyph
alphabet_font_rep::get_glyph (string s) {
  return base->get_glyph (rewrite (s));
}

int
alphabet_font_rep::index_glyph (string s, font_metric& fnm,
                                font_glyphs& fng) {
  return base->index_glyph (rewrite (s), fnm, fng);
}

double
alphabet_font_rep::get_left_slope (string s) {
  return base->get_left_slope (rewrite (s));
}

double
alphabet_font_rep::get_right_slope (string s) {
  return base->get_right_slope (rewrite (s));
}

SI
alphabet_font_rep::get_left_correction (string s) {
  return base->get_left_correction (rewrite (s));
}

SI
alphabet_font_rep::get_right_correction (string s) {
  return base->get_right_correction (rewrite (s));
}

SI
alphabet_font_rep::get_lsub_correction (string s) {
  return base->get_lsub_correction (rewrite (s));
}

SI
alphabet_font_rep::get_lsup_correction (string s) {
  return base->get_lsup_correction (rewrite (s));
}

SI
alphabet_font_rep::get_rsub_correction (string s) {
  return base->get_rsub_correction (rewrite (s));
}

SI
alphabet_font_rep::get_rsup_correction (string s) {
  return base->get_rsup_correction (rewrite (s));
}

SI
alphabet_font_rep::get_wide_correction (string s, int mode) {
  return base->get_wide_correction (rewrite (s), mode);
}

static font
alphabet_font (font base, string kind) {
  string name= "alphabet[" * base->res_name * "," * kind * "]";
  return make (font, name, tm_new<alphabet_font_rep> (name, base, kind));
}

static string
alphabet_of_tex_font (string fam) {
  // the alphabet which the letters of a TeX font are in, or ""
  if (starts (fam, "cmsy") || starts (fam, "cmbsy") || starts (fam, "rsfs") ||
      starts (fam, "eusm") || starts (fam, "eusb") || starts (fam, "euxm"))
    return "cal";
  if (starts (fam, "eufm") || starts (fam, "eufb")) return "frak";
  if (starts (fam, "msbm") || starts (fam, "bbm") || starts (fam, "bbold") ||
      starts (fam, "dsrom") || starts (fam, "dsss") || starts (fam, "ocmr"))
    return "bbb";
  return "";
}

/******************************************************************************
* From the name of a TeX font to a Latin Modern font
******************************************************************************/

static string
with_design_size (string base, string face, int dsize) {
  // Latin Modern comes in design sizes, as Computer Modern does: take the
  // one which is asked for when it exists, the 10 points one otherwise
  string name= base * as_string (dsize) * "-" * face;
  if (dsize != 10 && tt_font_exists (name)) return name;
  return base * "10-" * face;
}

static string
latin_modern_name (string fam, int dsize) {
  // the name without the design size and the prefix of the encoding
  // ("ec" for Cork, "cm" for the original fonts, "la" for Cyrillic...)
  while (N(fam) > 0 && is_digit (fam[N(fam)-1])) fam= fam (0, N(fam)-1);
  string kind= fam;
  if (N(fam) > 2 &&
      (starts (fam, "ec") || starts (fam, "cm") || starts (fam, "la") ||
       starts (fam, "gr") || starts (fam, "tc")))
    kind= fam (2, N(fam));

  // roman
  if (kind == "r" || kind == "rm")
    return with_design_size ("lmroman", "regular", dsize);
  if (kind == "bx" || kind == "b" || kind == "rb")
    return with_design_size ("lmroman", "bold", dsize);
  if (kind == "ti" || kind == "u")
    return with_design_size ("lmroman", "italic", dsize);
  if (kind == "bi" || kind == "bxti")
    return "lmroman10-bolditalic";
  if (kind == "sl")
    return with_design_size ("lmromanslant", "regular", dsize);
  if (kind == "bl" || kind == "bxsl")
    return "lmromanslant10-bold";
  if (kind == "cc" || kind == "csc" || kind == "xc")
    return "lmromancaps10-regular";
  if (kind == "sc" || kind == "oc")
    return "lmromancaps10-oblique";
  if (kind == "dh" || kind == "dunh")
    return "lmromandunh10-regular";

  // sans serif
  if (kind == "ss")
    return with_design_size ("lmsans", "regular", dsize);
  if (kind == "sx" || kind == "ssbx")
    return "lmsans10-bold";
  if (kind == "si" || kind == "ssi")
    return with_design_size ("lmsans", "oblique", dsize);
  if (kind == "so" || kind == "ssxi")
    return "lmsans10-boldoblique";
  if (kind == "ssdc")
    return "lmsansdemicond10-regular";

  // typewriter
  if (kind == "tt")
    return with_design_size ("lmmono", "regular", dsize);
  if (kind == "it" || kind == "itt")
    return "lmmono10-italic";
  if (kind == "st" || kind == "sltt")
    return "lmmonoslant10-regular";
  if (kind == "tc" || kind == "tcsc")
    return "lmmonocaps10-regular";
  if (kind == "vt" || kind == "vtt")
    return "lmmonoprop10-regular";

  // the fonts of the formulas (symbols, extensible characters, AMS...)
  if (kind == "mi" || kind == "sy" || kind == "ex" || kind == "mib" ||
      kind == "bsy" || starts (fam, "ms") || starts (fam, "eu") ||
      starts (fam, "stmary") || starts (fam, "wasy") || starts (fam, "bbm"))
    return "latinmodern-math";

  return with_design_size ("lmroman", "regular", dsize);
}

static font
latin_modern_font (string fam, int size, int dpi, int dsize) {
  string kind= alphabet_of_tex_font (fam);
  if (kind != "")
    return alphabet_font (unicode_font ("latinmodern-math", size, dpi), kind);
  return unicode_font (latin_modern_name (fam, dsize), size, dpi);
}

/******************************************************************************
* The interface of the Metafont plugin (font.hpp)
******************************************************************************/

font
tex_font (string fam, int size, int dpi, int dsize) {
  return latin_modern_font (fam, size, dpi, dsize);
}

font
tex_cm_font (string fam, int size, int dpi, int dsize) {
  return latin_modern_font (fam, size, dpi, dsize);
}

font
tex_ec_font (string fam, int size, int dpi, int dsize) {
  return latin_modern_font (fam, size, dpi, dsize);
}

font
tex_la_font (string fam, int size, int dpi, int dsize) {
  // sizes are in hundredths of a point for these two
  return latin_modern_font (fam, (size + 50) / 100, dpi, (dsize + 50) / 100);
}

font
tex_gr_font (string fam, int size, int dpi, int dsize) {
  return latin_modern_font (fam, (size + 50) / 100, dpi, (dsize + 50) / 100);
}

font
tex_adobe_font (string fam, int size, int dpi, int dsize) {
  return latin_modern_font (fam, size, dpi, dsize);
}

font
tex_rubber_font (string trl_name, string fam, int size, int dpi, int dsize) {
  (void) trl_name; (void) fam; (void) dsize;
  return rubber_font (unicode_font ("latinmodern-math", size, dpi));
}

font
tex_dummy_rubber_font (font base_fn) {
  return rubber_font (base_fn);
}

/******************************************************************************
* The default font of the interface (gui.hpp)
******************************************************************************/

static string the_default_font= "ecrm11@300";

void
set_default_font (string name) {
  the_default_font= name;
}

font
get_default_font (bool tt, bool mini, bool bold) {
  // as the interfaces of TeXmacs: a TeX name, its size and resolution, with
  // tt for a monospaced font, mini for a smaller one and bold for a bold one
  string s= the_default_font;
  int i, j, n= N(s);
  for (j=0; j<n; j++) if (is_digit (s[j])) break;
  string fam= s (0, j);
  if (tt && fam == "ecrm") fam= "ectt";
  if (mini && fam == "ecrm") fam= "ecss";
  if (bold && fam == "ecrm") fam= "ecbx";
  if (bold && fam == "ecss") fam= "ecsx";
  for (i=j; j<n; j++) if (s[j] == '@') break;
  int sz= (j<n? as_int (s (i, j)): 10);
  if (j<n) j++;
  int dpi= (j<n? as_int (s (j, n)): 300);
  if (mini) { sz= (int) (0.6 * sz); dpi= (int) (1.3333333 * dpi); }
  return tex_ec_font (fam, sz, dpi);
}

/******************************************************************************
* The files of TeX (tex_files.hpp): there are none
******************************************************************************/

void make_tex_tfm (string fn_name) { (void) fn_name; }
void make_tex_pk (string fn_name, int dpi, int design_dpi) {
  (void) fn_name; (void) dpi; (void) design_dpi; }
url  tfm_font_path () { return url_none (); }
void reset_tfm_path (bool rehash) { (void) rehash; }
void reset_pk_path (bool rehash) { (void) rehash; }
void reset_pfb_path () {}
url  resolve_tex (url name) { (void) name; return url_none (); }
bool exists_in_tex (url font_name) { (void) font_name; return false; }
