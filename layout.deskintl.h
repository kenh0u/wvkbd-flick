/* constants */
/* how tall the keyboard should be by default (can be overriden) */
#define KBD_PIXEL_HEIGHT 400

/* how tall the keyboard should be by default (can be overriden) */
#define KBD_PIXEL_LANDSCAPE_HEIGHT 400

/* spacing around each key */
#define KBD_KEY_BORDER 1

/* layout declarations */
enum layout_id {
	Full = 0,
	Special,
	KanaFlick,
	KanaDakuten,
	KanaHandakuten,
	KanaSmall,
	Cyrillic,
	ComposeA,
	ComposeE,
	ComposeY,
	ComposeU,
	ComposeI,
	ComposeO,
	ComposeW,
	ComposeR,
	ComposeT,
	ComposeP,
	ComposeS,
	ComposeD,
	ComposeF,
	ComposeG,
	ComposeH,
	ComposeJ,
	ComposeK,
	ComposeL,
	ComposeZ,
	ComposeX,
	ComposeC,
	ComposeV,
	ComposeB,
	ComposeN,
	ComposeM,
	ComposeMath,
	ComposePunctuation,
	ComposeBracket,
	ComposeCyrI,
	ComposeCyrJ,
	ComposeCyrE,
	ComposeCyrL,
	ComposeCyrU,
	ComposeCyrN,
	ComposeCyrTse,
	ComposeCyrChe,
	ComposeCyrG,
	ComposeCyrK,
	Index,
	NumLayouts,
};

static struct key keys_full[], keys_special[], keys_kana_flick[],
  keys_kana_dakuten[], keys_kana_handakuten[], keys_kana_small[],
  keys_cyrillic[],
  keys_compose_a[],
  keys_compose_e[], keys_compose_y[], keys_compose_u[], keys_compose_i[],
  keys_compose_o[], keys_compose_w[], keys_compose_r[], keys_compose_t[],
  keys_compose_p[], keys_compose_s[], keys_compose_d[], keys_compose_f[],
  keys_compose_g[], keys_compose_h[], keys_compose_j[], keys_compose_k[],
  keys_compose_l[], keys_compose_z[], keys_compose_x[], keys_compose_c[],
  keys_compose_v[], keys_compose_b[], keys_compose_n[], keys_compose_m[],
  keys_compose_math[], keys_compose_punctuation[], keys_compose_bracket[],
  keys_compose_cyr_i[], keys_compose_cyr_j[], keys_compose_cyr_e[],
  keys_compose_cyr_u[], keys_compose_cyr_l[], keys_compose_cyr_n[],
  keys_compose_cyr_tse[], keys_compose_cyr_che[], keys_compose_cyr_g[],
  keys_compose_cyr_k[], keys_index[];

static struct layout layouts[NumLayouts] = {
  [Full] = {keys_full, "latin", "full", true}, // second parameter is the keymap name
                                         // third parameter is the layout name
										 // last parameter indicates if it's an alphabetical/primary layout
  [Special] = {keys_special, "latin", "special", false},
  // Japanese flick layers: kana_glyph keys drive the flick engine in
  // keyboard.c (5-way vector from the touch, romaji sent through the IME
  // buffer). Modifier layers (dakuten/handakuten/small) apply their mark to
  // the next flick and fall back to KanaFlick; all use the latin keymap
  // because output goes through the romaji codes below, not kana glyphs.
  [KanaFlick] = {keys_kana_flick, "latin", "kana_flick", true},
  [KanaDakuten] = {keys_kana_dakuten, "latin", "kana_dakuten", false},
  [KanaHandakuten] = {keys_kana_handakuten, "latin", "kana_handakuten", false},
  [KanaSmall] = {keys_kana_small, "latin", "kana_small", false},
  [Cyrillic] = {keys_cyrillic, "cyrillic", "cyrillic", true},
  [ComposeA] = {keys_compose_a, "latin"},
  [ComposeE] = {keys_compose_e, "latin"},
  [ComposeY] = {keys_compose_y, "latin"},
  [ComposeU] = {keys_compose_u, "latin"},
  [ComposeI] = {keys_compose_i, "latin"},
  [ComposeO] = {keys_compose_o, "latin"},
  [ComposeW] = {keys_compose_w, "latin"},
  [ComposeR] = {keys_compose_r, "latin"},
  [ComposeT] = {keys_compose_t, "latin"},
  [ComposeP] = {keys_compose_p, "latin"},
  [ComposeS] = {keys_compose_s, "latin"},
  [ComposeD] = {keys_compose_d, "latin"},
  [ComposeF] = {keys_compose_f, "latin"},
  [ComposeG] = {keys_compose_g, "latin"},
  [ComposeH] = {keys_compose_h, "latin"},
  [ComposeJ] = {keys_compose_j, "latin"},
  [ComposeK] = {keys_compose_k, "latin"},
  [ComposeL] = {keys_compose_l, "latin"},
  [ComposeZ] = {keys_compose_z, "latin"},
  [ComposeX] = {keys_compose_x, "latin"},
  [ComposeC] = {keys_compose_c, "latin"},
  [ComposeV] = {keys_compose_v, "latin"},
  [ComposeB] = {keys_compose_b, "latin"},
  [ComposeN] = {keys_compose_n, "latin"},
  [ComposeM] = {keys_compose_m, "latin"},
  [ComposeMath] = {keys_compose_math, "latin"},
  [ComposePunctuation] = {keys_compose_punctuation, "latin"},
  [ComposeBracket] = {keys_compose_bracket, "latin"},
  [ComposeCyrI] = {keys_compose_cyr_i, "cyrillic"},
  [ComposeCyrJ] = {keys_compose_cyr_j, "cyrillic"},
  [ComposeCyrE] = {keys_compose_cyr_e, "cyrillic"},
  [ComposeCyrU] = {keys_compose_cyr_u, "cyrillic"},
  [ComposeCyrL] = {keys_compose_cyr_l, "cyrillic"},
  [ComposeCyrN] = {keys_compose_cyr_n, "cyrillic"},
  [ComposeCyrTse] = {keys_compose_cyr_tse, "cyrillic"},
  [ComposeCyrChe] = {keys_compose_cyr_che, "cyrillic"},
  [ComposeCyrG] = {keys_compose_cyr_g, "cyrillic"},
  [ComposeCyrK] = {keys_compose_cyr_k, "cyrillic"},

  [Index] = {keys_index,"latin","index", false},
};

/* key layouts
 *
 * define keys like:
 *
 *  `{
 *     "label",
 *     "SHIFT_LABEL",
 *     1,
 *     [Code, Mod, Layout, EndRow, Last],
 *     [KEY_CODE, Modifier],
 *     [&layout]
 *  },`
 *
 * - label: normal label for key
 *
 * - shift_label: label for key in shifted (uppercase) layout
 *
 * - width: column width of key
 *
 * - type: what kind of action this key peforms (emit keycode, toggle modifier,
 *   switch layout, or end the layout)
 *
 * - code: key scancode or modifier name (see
 *   `/usr/include/linux/input-event-codes.h` for scancode names, and
 *   `keyboard.h` for modifiers)
 *
 * - layout: layout to switch to when key is pressed
 */
static struct key keys_full[] = {
  {"1", "!", 1.0, Code, KEY_1},
  {"2", "@", 1.0, Code, KEY_2},
  {"3", "#", 1.0, Code, KEY_3},
  {"4", "$", 1.0, Code, KEY_4},
  {"5", "%", 1.0, Code, KEY_5},
  {"6", "^", 1.0, Code, KEY_6},
  {"7", "&", 1.0, Code, KEY_7},
  {"8", "*", 1.0, Code, KEY_8},
  {"9", "(", 1.0, Code, KEY_9},
  {"0", ")", 1.0, Code, KEY_0},
  // the number row's right edge is the easiest reach from the Ally grip:
  // the layer switch lives here
  {"⌨͕", "⌨͔", 2.0, NextLayer, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"Tab", "Tab", 2.0, Code, KEY_TAB, .scheme = 1},
  {"q", "Q", 1.0, Code, KEY_Q},
  {"w", "W", 1.0, Code, KEY_W},
  {"e", "E", 1.0, Code, KEY_E},
  {"r", "R", 1.0, Code, KEY_R},
  {"t", "T", 1.0, Code, KEY_T},
  {"y", "Y", 1.0, Code, KEY_Y},
  {"u", "U", 1.0, Code, KEY_U},
  {"i", "I", 1.0, Code, KEY_I},
  {"o", "O", 1.0, Code, KEY_O},
  {"p", "P", 1.0, Code, KEY_P},
  {"", "", 0.0, EndRow},

  {"Ctr", "Ctr", 2.5, Mod, Ctrl, .scheme = 1},
  {"a", "A", 1.0, Code, KEY_A},
  {"s", "S", 1.0, Code, KEY_S},
  {"d", "D", 1.0, Code, KEY_D},
  {"f", "F", 1.0, Code, KEY_F},
  {"g", "G", 1.0, Code, KEY_G},
  {"h", "H", 1.0, Code, KEY_H},
  {"j", "J", 1.0, Code, KEY_J},
  {"k", "K", 1.0, Code, KEY_K},
  {"l", "L", 1.0, Code, KEY_L},
  // Enter dropped: this row was the widest; a half-unit spacer that is not
  // a key anywhere near the letters is the least harmful filler
  {"", "", 0.5, Pad},
  {"", "", 0.0, EndRow},

  {"Cap", "Cap", 1.0, Mod, CapsLock, .scheme = 1},
  {"⇧", "⇫", 2.0, Mod, Shift, .scheme = 1},
  {"z", "Z", 1.0, Code, KEY_Z},
  {"x", "X", 1.0, Code, KEY_X},
  {"c", "C", 1.0, Code, KEY_C},
  {"v", "V", 1.0, Code, KEY_V},
  {"b", "B", 1.0, Code, KEY_B},
  {"n", "N", 1.0, Code, KEY_N},
  {"m", "M", 1.0, Code, KEY_M},
  {"⌫", "⌫", 2.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"Alt", "Alt", 1.0, Mod, Alt, .scheme = 1},
  {"Sup", "Sup", 2.0, Mod, Super, .scheme = 1},
  {"無", "無", 2.0, Code, KEY_MUHENKAN, .scheme = 1},
  {"", "", 3.0, Code, KEY_SPACE},
  {"漢", "漢", 2.0, KanaOn, KEY_HIRAGANA, .scheme = 1},
  {"Enter", "Enter", 2.0, Code, KEY_ENTER, .scheme = 1},
  /* end of layout */
  {"", "", 0.0, Last},
};

static struct key keys_special[] = {
  {"Esc", "Esc", 1.0, Code, KEY_ESC, .scheme = 1},
  {"`", "~", 1.0, Code, KEY_GRAVE, .scheme = 1},
  {"3", "#", 1.0, Code, KEY_3},
  {"4", "$", 1.0, Code, KEY_4},
  {"5", "%", 1.0, Code, KEY_5},
  {"6", "^", 1.0, Code, KEY_6},
  {"7", "&", 1.0, Code, KEY_7},
  {"8", "*", 1.0, Code, KEY_8},
  {"-", "_", 1.0, Code, KEY_MINUS, .scheme = 1},
  {"=", "+", 1.0, Code, KEY_EQUAL, .scheme = 1},
  {"⌨͕", "⌨͔", 2.0, NextLayer, .scheme = 1},
  {"", "", 0.0, EndRow},

  // The back layer keeps the surface's physical arrangement exactly --
  // muscle memory maps a slot to one position, changing what fills it as
  // the layer flips. The letter slots that carry missing symbols print
  // the symbol instead; everything else is untouched.
  // Slots: 1->`, 9->-, 0->=, i->[ (with {), o->] (with }), p->\ (with |),
  // k->; (with :), l->' (with ''), b->, (with <), n->. (with >), m->/ (with ?)
  {"Tab", "Tab", 2.0, Code, KEY_TAB, .scheme = 1},
  {"q", "Q", 1.0, Code, KEY_Q},
  {"w", "W", 1.0, Code, KEY_W},
  {"e", "E", 1.0, Code, KEY_E},
  {"r", "R", 1.0, Code, KEY_R},
  {"t", "T", 1.0, Code, KEY_T},
  {"y", "Y", 1.0, Code, KEY_Y},
  {"u", "U", 1.0, Code, KEY_U},
  {"[", "{", 1.0, Code, KEY_LEFTBRACE, .scheme = 1},
  {"]", "}", 1.0, Code, KEY_RIGHTBRACE, .scheme = 1},
  {"\\", "|", 1.0, Code, KEY_BACKSLASH, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"Ctr", "Ctr", 2.5, Mod, Ctrl, .scheme = 1},
  {"a", "A", 1.0, Code, KEY_A},
  {"s", "S", 1.0, Code, KEY_S},
  {"d", "D", 1.0, Code, KEY_D},
  {"f", "F", 1.0, Code, KEY_F},
  {"g", "G", 1.0, Code, KEY_G},
  {"h", "H", 1.0, Code, KEY_H},
  {"j", "J", 1.0, Code, KEY_J},
  {";", ":", 1.0, Code, KEY_SEMICOLON, .scheme = 1},
  {"'", "''", 1.0, Code, KEY_APOSTROPHE, .scheme = 1},
  {"", "", 0.5, Pad},
  {"", "", 0.0, EndRow},

  {"Cap", "Cap", 1.0, Mod, CapsLock, .scheme = 1},
  {"⇧", "⇫", 2.0, Mod, Shift, .scheme = 1},
  {"z", "Z", 1.0, Code, KEY_Z},
  {"x", "X", 1.0, Code, KEY_X},
  {"c", "C", 1.0, Code, KEY_C},
  {"v", "V", 1.0, Code, KEY_V},
  {",", "<", 1.0, Code, KEY_COMMA, .scheme = 1},
  {".", ">", 1.0, Code, KEY_DOT, .scheme = 1},
  {"/", "?", 1.0, Code, KEY_SLASH, .scheme = 1},
  {"⌫", "⌫", 2.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"Alt", "Alt", 1.0, Mod, Alt, .scheme = 1},
  {"Sup", "Sup", 2.0, Mod, Super, .scheme = 1},
  {"無", "無", 2.0, Code, KEY_MUHENKAN, .scheme = 1},
  {"", "", 3.0, Code, KEY_SPACE},
  {"漢", "漢", 2.0, KanaOn, KEY_HIRAGANA, .scheme = 1},
  {"Enter", "Enter", 2.0, Code, KEY_ENTER, .scheme = 1},
  /* end of layout */
  {"", "", 0.0, Last},
};

/* Japanese flick (kana) keyboard, 6 columns x 4 rows, every row 6.0U so all
 * vertical grid lines line up and the kana 10-key sits as a clean 3x4 grid in
 * columns 3-5 (column indices 2,3,4).
 *
 * Column roles:
 *   0: left function strip (tab / emoji / [ab] cursor strip / handakuten)
 *   1: mode strip (\u6570, \uff21, kana anchor, small)
 *   2..4: the kana 10-key (flick: C=L=left, U=up, R=right, D=down)
 *   5: right function strip (qwerty-exit, space, backspace, enter)
 *
 * Key type KanaGly (glyph carried in `label`) is the flick engine input; the
 * sequences per direction live in keyboard.c::kana_flick_table. Functional
 * keys:
 *   "\u2a08\u0338"  (col5 row0): KanaOff - KEY_MUHENKAN + back to Full, on unpress
 *   "\u3000"/"\u2302"/"\u21a9\ufe0f": Code keys, standard
 *   "\u309b" "\u309c" "\u5c0f": KanaMod - one-shot modifier layers
 * Keys marked F are THIS-phase no-ops (drawn in a muted scheme, no action):
 *   \u21a5(tab), \u6570, \ud83d\ude42, \uff21, the \u2190\u2192 strip, and the kana anchor
 *   " \u3042" in row2 col1 (returns from a future katakana face; noop there).
 *
 * Flick table (rows L U R D, first char = tap):
 *  \u3042  \u3044\u3046\u3048\u304a | \u304b ...(standard jis 12-key)
 *  \u3084: L/- U=\u3086 R/- D=\u3088 ; \u308f: L=\u3092 U=\u3093 R=\u30fc ; \u3001: L=\u3002 U=\uff1f R=\uff01
 */
#define KANA_FLICK_ROWS(...) ((const char *[5]){__VA_ARGS__})

static struct key keys_kana_flick[] = {
  {"Tab", "", 1.0, Code, KEY_TAB, .scheme = 1},
  {"☆123", "", 1.0, KanaNoop, .scheme = 1},
  {"あ", "", 1.0, KanaGly, .scheme = 0},
  {"か", "", 1.0, KanaGly, .scheme = 0},
  {"さ", "", 1.0, KanaGly, .scheme = 0},
  {"⌫", "", 1.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"🙂", "", 1.0, KanaNoop, .scheme = 1},
  {"ABC", "", 1.0, KanaNoop, .scheme = 1},
  {"た", "", 1.0, KanaGly, .scheme = 0},
  {"な", "", 1.0, KanaGly, .scheme = 0},
  {"は", "", 1.0, KanaGly, .scheme = 0},
  {"空白", "", 1.0, Code, KEY_SPACE},
  {"", "", 0.0, EndRow},

  {"゜", "", 1.0, KanaMod, 0, &layouts[KanaHandakuten], .scheme = 0},
  {"あいう", "", 1.0, KanaAnchor, 0, &layouts[KanaFlick], .scheme = 0},
  {"ま", "", 1.0, KanaGly, .scheme = 0},
  {"や", "", 1.0, KanaGly, .scheme = 0},
  {"ら", "", 1.0, KanaGly, .scheme = 0},
  {"Enter", "", 1.0, Code, KEY_ENTER, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"⌨͕", "", 1.0, KanaOff, .scheme = 1},
  {"小", "", 1.0, KanaMod, 0, &layouts[KanaSmall], .scheme = 0},
  {"゛", "", 1.0, KanaMod, 0, &layouts[KanaDakuten], .scheme = 0},
  {"わ", "", 1.0, KanaGly, .scheme = 0},
  {"、", "", 1.0, KanaGly, .scheme = 0},
  {"", "", 1.0, Pad, .scheme = 1},

  /* end of layout */
  {"", "", 0.0, Last},
};

/* One-shot modifier layers. INVARIANT: the grid is cell-for-cell identical
 * to keys_kana_flick — same glyph column belongs to the same row and
 * direction. Only the modified column HEAD changes: a cell whose kana has a
 * voiced/handakuten/small form shows the modified head (KanaGlyMut /
 * KanaGlySmall) and its 5-way flick behaves exactly like the plain head's;
 * every other cell shows the plain head and stays KanaGlyPlain, so typing
 * through a one-shot layer works like the plain layer with just one head
 * changed. Modifier cells (゛゜小) return/cross-switch at press. */
static struct key keys_kana_dakuten[] = {
  {"Tab", "", 1.0, Code, KEY_TAB, .scheme = 1},
  {"☆123", "", 1.0, KanaNoop, .scheme = 1},
  {"ゔ", "", 1.0, KanaGlyMut, .scheme = 0},
  {"が", "", 1.0, KanaGlyMut, .scheme = 0},
  {"ざ", "", 1.0, KanaGlyMut, .scheme = 0},
  {"⌫", "", 1.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"🙂", "", 1.0, KanaNoop, .scheme = 1},
  {"ABC", "", 1.0, KanaNoop, .scheme = 1},
  {"だ", "", 1.0, KanaGlyMut, .scheme = 0},
  {"な", "", 1.0, KanaGlyPlain, .scheme = 0},
  {"ば", "", 1.0, KanaGlyMut, .scheme = 0},
  {"空白", "", 1.0, Code, KEY_SPACE},
  {"", "", 0.0, EndRow},

  {"゜", "", 1.0, KanaMod, 0, &layouts[KanaHandakuten], .scheme = 0},
  {"あいう", "", 1.0, KanaAnchor, 0, &layouts[KanaFlick], .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"Enter", "", 1.0, Code, KEY_ENTER, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"⌨͕", "", 1.0, KanaOff, .scheme = 1},
  {"小", "", 1.0, KanaMod, 0, &layouts[KanaSmall], .scheme = 0},
  {"゛", "", 1.0, KanaCancel, 0, &layouts[KanaFlick], .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, Pad, .scheme = 1},
  {"", "", 1.0, Pad, .scheme = 1},

  /* end of layout */
  {"", "", 0.0, Last},
};

/* handakuten (゜): only the は column voices (ぱ row head); plain elsewhere */
static struct key keys_kana_handakuten[] = {
  {"Tab", "", 1.0, Code, KEY_TAB, .scheme = 1},
  {"☆123", "", 1.0, KanaNoop, .scheme = 1},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"⌫", "", 1.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"🙂", "", 1.0, KanaNoop, .scheme = 1},
  {"ABC", "", 1.0, KanaNoop, .scheme = 1},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"ぱ", "", 1.0, KanaGlyMut, .scheme = 0},
  {"空白", "", 1.0, Code, KEY_SPACE},
  {"", "", 0.0, EndRow},

  {"゜", "", 1.0, KanaMod, 0, &layouts[KanaHandakuten], .scheme = 0},
  {"あいう", "", 1.0, KanaAnchor, 0, &layouts[KanaFlick], .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"Enter", "", 1.0, Code, KEY_ENTER, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"⌨͕", "", 1.0, KanaOff, .scheme = 1},
  {"小", "", 1.0, KanaMod, 0, &layouts[KanaSmall], .scheme = 0},
  {"゛", "", 1.0, KanaMod, 0, &layouts[KanaDakuten], .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, Pad, .scheme = 1},
  {"", "", 1.0, Pad, .scheme = 1},

  /* end of layout */
  {"", "", 0.0, Last},
};

/* small (小): small a-row head (ぁ...), っ on the た column, ゃ on the や
 * column, ゎ on the わ cell. R flicks (i/u/e/o) produce the small vowel/ya
 * variants via the kana_small_table. */
static struct key keys_kana_small[] = {
  {"Tab", "", 1.0, Code, KEY_TAB, .scheme = 1},
  {"☆123", "", 1.0, KanaNoop, .scheme = 1},
  {"ぁ", "", 1.0, KanaGlySmall, .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"⌫", "", 1.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"🙂", "", 1.0, KanaNoop, .scheme = 1},
  {"ABC", "", 1.0, KanaNoop, .scheme = 1},
  {"っ", "", 1.0, KanaGlySmall, .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"空白", "", 1.0, Code, KEY_SPACE},
  {"", "", 0.0, EndRow},

  {"゜", "", 1.0, KanaMod, 0, &layouts[KanaHandakuten], .scheme = 0},
  {"あいう", "", 1.0, KanaAnchor, 0, &layouts[KanaFlick], .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"ゃ", "", 1.0, KanaGlySmall, .scheme = 0},
  {"", "", 1.0, KanaHide, .scheme = 2},
  {"Enter", "", 1.0, Code, KEY_ENTER, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"⌨͕", "", 1.0, KanaOff, .scheme = 1},
  {"小", "", 1.0, KanaCancel, 0, &layouts[KanaFlick], .scheme = 0},
  {"゛", "", 1.0, KanaMod, 0, &layouts[KanaDakuten], .scheme = 0},
  {"ゎ", "", 1.0, KanaGlySmall, .scheme = 0},
  {"", "", 1.0, Pad, .scheme = 1},
  {"", "", 1.0, Pad, .scheme = 1},

  /* end of layout */
  {"", "", 0.0, Last},
};

static struct key keys_cyrillic[] = {
  {"Tab", "Tab", 2.0, Code, KEY_TAB, .scheme = 1},
  {"й", "Й", 1.0, Code, KEY_Q},
  {"ц", "Ц", 1.0, Code, KEY_W},
  {"у", "У", 1.0, Code, KEY_E},
  {"к", "К", 1.0, Code, KEY_R},
  {"е", "Е", 1.0, Code, KEY_T},
  {"н", "Н", 1.0, Code, KEY_Y},
  {"г", "Г", 1.0, Code, KEY_U},
  {"ш", "Ш", 1.0, Code, KEY_I},
  {"щ", "Щ", 1.0, Code, KEY_O},
  {"з", "З", 1.0, Code, KEY_P},
  {"", "", 0.0, EndRow},

  {"Ctr", "Ctr", 2.5, Mod, Ctrl, .scheme = 1},
  {"ф", "Ф", 1.0, Code, KEY_A},
  {"ы", "Ы", 1.0, Code, KEY_S},
  {"в", "В", 1.0, Code, KEY_D},
  {"а", "А", 1.0, Code, KEY_F},
  {"п", "П", 1.0, Code, KEY_G},
  {"р", "Р", 1.0, Code, KEY_H},
  {"о", "О", 1.0, Code, KEY_J},
  {"л", "Л", 1.0, Code, KEY_K},
  {"д", "Д", 1.0, Code, KEY_L},
  {"", "", 0.5, Pad},
  {"", "", 0.0, EndRow},

  {"Cap", "Cap", 1.0, Mod, CapsLock, .scheme = 1},
  {"⇧", "⇫", 2.0, Mod, Shift, .scheme = 1},
  {"я", "Я", 1.0, Code, KEY_Z},
  {"ч", "Ч", 1.0, Code, KEY_X},
  {"с", "С", 1.0, Code, KEY_C},
  {"м", "М", 1.0, Code, KEY_V},
  {"и", "И", 1.0, Code, KEY_B},
  {"т", "Т", 1.0, Code, KEY_N},
  {"ь", "Ь", 1.0, Code, KEY_M},
  {"⌫", "⌫", 2.0, Code, KEY_BACKSPACE, .scheme = 1},
  {"", "", 0.0, EndRow},

  {"Alt", "Alt", 1.0, Mod, Alt, .scheme = 1},
  {"Sup", "Sup", 2.0, Mod, Super, .scheme = 1},
  {"無", "無", 2.0, Code, KEY_MUHENKAN, .scheme = 1},
  {"", "", 3.0, Code, KEY_SPACE},
  {"漢", "漢", 2.0, Code, KEY_HIRAGANA, .scheme = 1},
  {"Enter", "Enter", 2.0, Code, KEY_ENTER, .scheme = 1},
  /* end of layout */
  {"", "", 0.0, Last},
};

static struct key keys_compose_a[] = {
  {"à", "À", 1.0, Copy, 0x00E0, 0, 0x00C0},
  {"á", "Á", 1.0, Copy, 0x00E1, 0, 0x00C1},
  {"â", "Â", 1.0, Copy, 0x00E2, 0, 0x00C2},
  {"ã", "Ã", 1.0, Copy, 0x00E3, 0, 0x00C3},
  {"ä", "Ä", 1.0, Copy, 0x00E4, 0, 0x00C4},
  {"å", "Å", 1.0, Copy, 0x00E5, 0, 0x00C5},
  {"æ", "Æ", 1.0, Copy, 0x00E7, 0, 0x00C6},
  {"ā", "Ā", 1.0, Copy, 0x0101, 0, 0x0100},
  {"ă", "Ă", 1.0, Copy, 0x0103, 0, 0x0102},
  {"ą", "Ą", 1.0, Copy, 0x0105, 0, 0x0104},
  {"", "", 0.0, EndRow},
  {"α", "Α", 1.0, Copy, 0x03B1, 0, 0x0391},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_e[] = {
  {"è", "È", 1.0, Copy, 0x00E8, 0, 0x00C8},
  {"é", "É", 1.0, Copy, 0x00E9, 0, 0x00C9},
  {"ê", "Ê", 1.0, Copy, 0x00EA, 0, 0x00CA},
  {"ë", "Ë", 1.0, Copy, 0x00EB, 0, 0x00CB},
  {"ē", "Ē", 1.0, Copy, 0x0113, 0, 0x0112},
  {"ĕ", "Ĕ", 1.0, Copy, 0x0115, 0, 0x0114},
  {"ė", "Ė", 1.0, Copy, 0x0117, 0, 0x0116},
  {"ę", "Ę", 1.0, Copy, 0x0119, 0, 0x0118},
  {"ě", "Ě", 1.0, Copy, 0x011B, 0, 0x011A},
  {"", "", 1.0, Pad},
  {"", "", 0.0, EndRow},
  {"ε", "Ε", 1.0, Copy, 0x03B5, 0, 0x0395},
  {"ǝ", "Ə", 1.0, Copy, 0x0259, 0, 0x018F},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_y[] = {
  {"ý", "Ý", 1.0, Copy, 0x00FD, 0, 0x00DD},
  {"ÿ", "Ÿ", 1.0, Copy, 0x00FF, 0, 0x0178},
  {"ŷ", "Ŷ", 1.0, Copy, 0x0177, 0, 0x0176},
  {"", "", 7.0, Pad},
  {"", "", 0.0, EndRow},
  {"υ", "Υ", 1.0, Copy, 0x03C5, 0, 0x03A5},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_u[] = {
  {"ù", "Ù", 1.0, Copy, 0x00F9, 0, 0x00D9},
  {"ú", "Ú", 1.0, Copy, 0x00FA, 0, 0x00DA},
  {"û", "Û", 1.0, Copy, 0x00FB, 0, 0x00DB},
  {"ü", "Ü", 1.0, Copy, 0x00FC, 0, 0x00DC},
  {"ũ", "Ũ", 1.0, Copy, 0x0169, 0, 0x0168},
  {"ū", "Ū", 1.0, Copy, 0x016B, 0, 0x016A},
  {"ŭ", "Ŭ", 1.0, Copy, 0x016D, 0, 0x016C},
  {"ů", "Ů", 1.0, Copy, 0x016F, 0, 0x016E},
  {"ű", "Ű", 1.0, Copy, 0x0171, 0, 0x0170},
  {"ų", "Ų", 1.0, Copy, 0x0173, 0, 0x0172},
  {"", "", 0.0, EndRow},
  {"υ", "Υ", 1.0, Copy, 0x03C5, 0, 0x03A5},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_o[] = {
  {"ò", "Ò", 1.0, Copy, 0x00F2, 0, 0x00D2},
  {"ó", "Ó", 1.0, Copy, 0x00F3, 0, 0x00D3},
  {"ô", "Ô", 1.0, Copy, 0x00F4, 0, 0x00D4},
  {"õ", "Õ", 1.0, Copy, 0x00F5, 0, 0x00D5},
  {"ö", "Ö", 1.0, Copy, 0x00F6, 0, 0x00D6},
  {"ø", "Ø", 1.0, Copy, 0x00F8, 0, 0x00D8},
  {"ō", "Ō", 1.0, Copy, 0x014D, 0, 0x014C},
  {"ŏ", "Ŏ", 1.0, Copy, 0x014F, 0, 0x014E},
  {"ő", "Ő", 1.0, Copy, 0x0151, 0, 0x0150},
  {"œ", "Œ", 1.0, Copy, 0x0153, 0, 0x0152},
  {"", "", 0.0, EndRow},
  {"ο", "Ο", 1.0, Copy, 0x03BF, 0, 0x039F},
  {"ω", "Ο", 1.0, Copy, 0x03C9, 0, 0x03A9},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_i[] = {
  {"ì", "Ì", 1.0, Copy, 0x00EC, 0, 0x00CC},
  {"í", "Í", 1.0, Copy, 0x00ED, 0, 0x00CD},
  {"î", "Î", 1.0, Copy, 0x00EE, 0, 0x00CE},
  {"ï", "Ï", 1.0, Copy, 0x00EF, 0, 0x00CF},
  {"ĩ", "Ĩ", 1.0, Copy, 0x0129, 0, 0x0128},
  {"ī", "Ī", 1.0, Copy, 0x012B, 0, 0x012A},
  {"ĭ", "Ĭ", 1.0, Copy, 0x012D, 0, 0x012C},
  {"į", "Į", 1.0, Copy, 0x012F, 0, 0x012E},
  {"ı", "I", 1.0, Copy, 0x0131, 0, 0x0049},
  {"i", "İ", 1.0, Copy, 0x0069, 0, 0x0130},
  {"", "", 0.0, EndRow},
  {"ι", "Ι", 1.0, Copy, 0x03B9, 0, 0x0399},
  {"η", "Η", 1.0, Copy, 0x03B7, 0, 0x0397},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_index[] = {
  {"Full", "Full", 1.0, Layout, 0, &layouts[Full], .scheme = 1},
  {"Special", "Special", 1.0, Layout, 0, &layouts[Special], .scheme = 1},
  {"Абв", "Абв", 1.0, Layout, 0, &layouts[Cyrillic], .scheme = 1},
  {"", "", 0.0, Last},
};

static struct key keys_compose_w[] = {
  {"ŵ", "Ŵ", 1.0, Copy, 0x0175, 0, 0x0174},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_r[] = {
  {"ŕ", "Ŕ", 1.0, Copy, 0x0155, 0, 0x0154},
  {"ŗ", "Ŗ", 1.0, Copy, 0x0157, 0, 0x0156},
  {"ř", "Ř", 1.0, Copy, 0x0159, 0, 0x0158},
  {"", "", 7.0, Pad},
  {"", "", 0.0, EndRow},
  {"ρ", "Ρ", 1.0, Copy, 0x03C1, 0, 0x03A1},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_t[] = {
  {"ț", "Ț", 1.0, Copy, 0x021B, 0, 0x021A},
  {"ť", "Ť", 1.0, Copy, 0x0165, 0, 0x0164},
  {"ŧ", "Ŧ", 1.0, Copy, 0x0167, 0, 0x0166},
  {"þ", "Þ", 1.0, Copy, 0x00FE, 0, 0x00DE},
  {"", "", 6.0, Pad},
  {"", "", 0.0, EndRow},
  {"τ", "Τ", 1.0, Copy, 0x03C4, 0, 0x03A4},
  {"θ", "Θ", 1.0, Copy, 0x03B8, 0, 0x0398},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_p[] = {
  {"π", "Π", 1.0, Copy, 0x03C0, 0, 0x03A0},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_s[] = {
  {"ś", "Ś", 1.0, Copy, 0x015B, 0, 0x015A},
  {"ŝ", "Ŝ", 1.0, Copy, 0x015D, 0, 0x015C},
  {"ş", "Ş", 1.0, Copy, 0x015F, 0, 0x015E},
  {"š", "Š", 1.0, Copy, 0x0161, 0, 0x0160},
  {"ß", "ẞ", 1.0, Copy, 0x00DF, 0, 0x1E9E},
  {"", "", 5.0, Pad},
  {"", "", 0.0, EndRow},
  {"σ", "Σ", 1.0, Copy, 0x03C3, 0, 0x03A3},
  {"ς", "Σ", 1.0, Copy, 0x03C2, 0, 0x03A3},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_d[] = {
  {"ð", "Ð", 1.0, Copy, 0x00F0, 0, 0x00D0},
  {"ď", "Ď", 1.0, Copy, 0x010F, 0, 0x010E},
  {"đ", "Đ", 1.0, Copy, 0x0111, 0, 0x0110},
  {"", "", 7.0, Pad},
  {"", "", 0.0, EndRow},
  {"δ", "Δ", 1.0, Copy, 0x03B4, 0, 0x0394},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_f[] = {
  {"φ", "Φ", 1.0, Copy, 0x03C6, 0, 0x03A6},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_g[] = {
  {"ĝ", "Ĝ", 1.0, Copy, 0x011D, 0, 0x011C},
  {"ğ", "Ğ", 1.0, Copy, 0x011F, 0, 0x011E},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"γ", "Γ", 1.0, Copy, 0x03B3, 0, 0x0393},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_h[] = {
  {"ĥ", "Ĥ", 1.0, Copy, 0x0125, 0, 0x0124},
  {"ħ", "Ħ", 1.0, Copy, 0x0127, 0, 0x0126},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"η", "Η", 1.0, Copy, 0x03B7, 0, 0x0397},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_j[] = {
  {"ĵ", "Ĵ", 1.0, Copy, 0x0135, 0, 0x0134},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 10.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_k[] = {
  {"ķ", "Ķ", 1.0, Copy, 0x0137, 0, 0x0136},
  {"ǩ", "Ǩ", 1.0, Copy, 0x01E9, 0, 0x01E8},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"κ", "Κ", 1.0, Copy, 0x03BA, 0, 0x039A},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_l[] = {
  {"ľ", "Ľ", 1.0, Copy, 0x013E, 0, 0x013D},
  {"ŀ", "Ŀ", 1.0, Copy, 0x0140, 0, 0x013F},
  {"ł", "Ł", 1.0, Copy, 0x0142, 0, 0x0141},
  {"", "", 7.0, Pad},
  {"", "", 0.0, EndRow},
  {"λ", "Λ", 1.0, Copy, 0x03BB, 0, 0x039B},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_z[] = {
  {"ź", "Ź", 1.0, Copy, 0x017A, 0, 0x0179},
  {"ż", "Ż", 1.0, Copy, 0x017C, 0, 0x017B},
  {"ž", "Ž", 1.0, Copy, 0x017E, 0, 0x017D},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"ζ", "Ζ", 1.0, Copy, 0x03B6, 0, 0x0396},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_x[] = {
  {"χ", "Χ", 1.0, Copy, 0x03C7, 0, 0x03A7},
  {"ξ", "Ξ", 1.0, Copy, 0x03BE, 0, 0x039E},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_c[] = {
  {"ç", "Ç", 1.0, Copy, 0x00E7, 0, 0x00C7},
  {"ć", "Ć", 1.0, Copy, 0x0107, 0, 0x0106},
  {"ĉ", "Ĉ", 1.0, Copy, 0x0109, 0, 0x0108},
  {"ċ", "Ċ", 1.0, Copy, 0x010B, 0, 0x010A},
  {"č", "Č", 1.0, Copy, 0x010D, 0, 0x010C},
  {"", "", 5.0, Pad},
  {"", "", 0.0, EndRow},
  {"χ", "Χ", 1.0, Copy, 0x03C7, 0, 0x03A7},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_v[] = {
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_b[] = {
  {"β", "Β", 1.0, Copy, 0x03B2, 0, 0x0392},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_n[] = {
  {"ñ", "Ñ", 1.0, Copy, 0x00F1, 0, 0x00D1},
  {"ń", "Ń", 1.0, Copy, 0x0144, 0, 0x0143},
  {"ņ", "Ņ", 1.0, Copy, 0x0146, 0, 0x0145},
  {"ň", "Ň", 1.0, Copy, 0x0148, 0, 0x0147},
  {"ŋ", "Ŋ", 1.0, Copy, 0x014B, 0, 0x014A},
  {"", "", 5.0, Pad},
  {"", "", 0.0, EndRow},
  {"ν", "Ν", 1.0, Copy, 0x03BD, 0, 0x039D},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_m[] = {
  {"μ", "Μ", 1.0, Copy, 0x03BC, 0, 0x039C},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_math[] = {
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"+", "+", 1, Code, KEY_EQUAL, 0, Shift},
  {"/", "/", 1, Code, KEY_SLASH},
  {"*", "*", 1, Code, KEY_8, 0, Shift},
  {"-", "-", 1, Code, KEY_MINUS},
  {"=", "=", 1, Code, KEY_EQUAL},
  {"_", "_", 1, Code, KEY_MINUS, 0, Shift},
  {"—", "—", 1, Copy, 0x2014, 0, 0x2014},
  {"", "", 1.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_punctuation[] = {
  {"", "", 0.0, EndRow},
  {"", "", 4.5, Pad},
  {".", ".", 1, Code, KEY_DOT},
  {"…", "…", 1, Copy, 0x2026, 0, 0x2026},
  {":", ":", 1, Code, KEY_SEMICOLON, 0, Shift},
  {";", ";", 1, Code, KEY_SEMICOLON, 0},
  {"⍽", "⍽", 1, Copy, 0x202F, 0, 0x00A0},
  {"", "", 0.5, Pad},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 3, Pad},
  {"!", "!", 1, Code, KEY_1, 0, Shift},
  {"?", "?", 1, Code, KEY_DOT, 0, Shift},
  {"·", "·", 1, Copy, 0x2027, 0, 0x2027},
  {",", ",", 1, Code, KEY_COMMA},
  {"", "", 1.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_bracket[] = {
  {"", "", 0, EndRow},
  {"", "", 1.5, Pad},
  {"{", "{", 1, Code, KEY_LEFTBRACE, 0, Shift},
  {"}", "}", 1, Code, KEY_RIGHTBRACE, 0, Shift},
  {"[", "[", 1, Code, KEY_LEFTBRACE},
  {"]", "]", 1, Code, KEY_RIGHTBRACE},
  {"", "", 4.5, Pad},
  {"", "", 0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"<", "<", 1, Code, KEY_COMMA, 0, AltGr},
  {">", ">", 1, Code, KEY_SLASH, 0, Shift},
  {"`", "`", 1, Code, KEY_GRAVE},
  {"\"", "\"", 1, Code, KEY_APOSTROPHE, 0, Shift},
  {"'", "'", 1, Code, KEY_APOSTROPHE},
  {"", "", 3.5, Pad},
  {"", "", 0.0, EndRow},
  {"Abc", "Abc", 1.0, BackLayer, .scheme = 1},
  {"Ctr", "Ctr", 1.0, Mod, Ctrl, .scheme = 1},
  {"", "", 8, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_i[] = {
  {"і", "І", 1.0, Copy, 0x0456, 0, 0x0406},
  {"ї", "Ї", 1.0, Copy, 0x0457, 0, 0x0407},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_j[] = {
  {"ј", "Ј", 1.0, Copy, 0x0458, 0, 0x0408},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_e[] = {
  {"є", "Є", 1.0, Copy, 0x0454, 0, 0x0404},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_u[] = {
  {"ў", "Ў", 1.0, Copy, 0x045E, 0, 0x040E},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_l[] = {
  {"љ", "Љ", 1.0, Copy, 0x0459, 0, 0x0409},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_n[] = {
  {"њ", "Њ", 1.0, Copy, 0x045A, 0, 0x040A},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_che[] = {
  {"ћ", "Ћ", 1.0, Copy, 0x045B, 0, 0x040B},
  {"ђ", "Ђ", 1.0, Copy, 0x0452, 0, 0x0402},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_tse[] = {
  {"џ", "Џ", 1.0, Copy, 0x045F, 0, 0x040F},
  {"ѕ", "Ѕ", 1.0, Copy, 0x0455, 0, 0x0405},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_g[] = {
  {"ѓ", "Ѓ", 1.0, Copy, 0x0453, 0, 0x0403},
  {"ґ", "Ґ", 1.0, Copy, 0x0491, 0, 0x0490},
  {"", "", 8.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};

static struct key keys_compose_cyr_k[] = {
  {"ќ", "Ќ", 1.0, Copy, 0x0453, 0, 0x040C},
  {"", "", 9.0, Pad},
  {"", "", 0.0, EndRow},
  {"", "", 0.0, EndRow},
  {"⇧", "⇫", 1.5, Mod, Shift, .scheme = 1},
  {"", "", 8.5, Pad},
  {"", "", 0.0, EndRow},
  {"Абв", "Абв", 1.0, BackLayer, .scheme = 1},
  {"", "", 9, Pad},
  {"", "", 0.0, Last},
};
