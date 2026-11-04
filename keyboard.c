#include "proto/virtual-keyboard-unstable-v1-client-protocol.h"
#include <linux/input-event-codes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "keyboard.h"
#include <linux/input-event-codes.h>

#include "drw.h"
#include "os-compatibility.h"

#define MAX_LAYERS 25

/* ---------------------------------------------------------------------------
 * Kana flick engine (see keyboard.h for the key types).
 * ------------------------------------------------------------------------- */

/* flick direction */
enum flick_dir { FLICK_C = 0, FLICK_L, FLICK_U, FLICK_R, FLICK_D };

/* Minimum travel (surface px) for a touch to count as a flick. Below it the
 * touch is a plain tap (center). A key is ~90px wide on this build; a flick
 * should be intentional travel, not fat-finger jitter. */
#define FLICK_MIN_DIST 30

/* (glyph)[dir] -> evdev scancodes, 0-terminated. MSB (0x80) set = wrap that
 * key in Shift press/release. At most 3 keys + terminator. */
struct flick_entry {
    const char *glyph;
    uint8_t dirs[5][4]; /* C L U R D */
};

#define SH 0x80
#define END 0

#define K(g, c, l, u, r, d)                                                    \
    { .glyph = g, .dirs = {{c}, {l}, {u}, {r}, {d}} }

/* plain kana 12-key (JIS flip layout) */
static const struct flick_entry kana_flick_table[] = {
    /* a-row keeps its xN romaji: mozc maps those through even when the
     * parser would not produce a distinct small kana otherwise */
    { "あ", { {KEY_A}, {KEY_I}, {KEY_U}, {KEY_E}, {KEY_O} } },
    { "か", { {KEY_K, KEY_A}, {KEY_K, KEY_I}, {KEY_K, KEY_U}, {KEY_K, KEY_E}, {KEY_K, KEY_O} } },
    { "さ", { {KEY_S, KEY_A}, {KEY_S, KEY_I}, {KEY_S, KEY_U}, {KEY_S, KEY_E}, {KEY_S, KEY_O} } },
    { "た", { {KEY_T, KEY_A}, {KEY_T, KEY_I}, {KEY_T, KEY_U}, {KEY_T, KEY_E}, {KEY_T, KEY_O} } },
    { "な", { {KEY_N, KEY_A}, {KEY_N, KEY_I}, {KEY_N, KEY_U}, {KEY_N, KEY_E}, {KEY_N, KEY_O} } },
    { "は", { {KEY_H, KEY_A}, {KEY_H, KEY_I}, {KEY_H, KEY_U}, {KEY_H, KEY_E}, {KEY_H, KEY_O} } },
    { "ま", { {KEY_M, KEY_A}, {KEY_M, KEY_I}, {KEY_M, KEY_U}, {KEY_M, KEY_E}, {KEY_M, KEY_O} } },
    { "や", { {KEY_Y, KEY_A}, {END}, {KEY_Y, KEY_U}, {END}, {KEY_Y, KEY_O} } },
    { "ら", { {KEY_R, KEY_A}, {KEY_R, KEY_I}, {KEY_R, KEY_U}, {KEY_R, KEY_E}, {KEY_R, KEY_O} } },
    { "わ", { {KEY_W, KEY_A}, {KEY_W, KEY_O}, {KEY_N, KEY_N}, {KEY_MINUS}, END } },
    /* punctuation cell: plain ASCII , . ? ! (evdev 242-245, dedicated XKB
     * keys). fcitx5-mozc's romaji map turns , . into 、。 and passes
     * ? ! straight through; the old SH+SLASH / SH+1 wraps lost the shift
     * on the way to the compositor (user saw ・ and １). */
    {0}, /* 、 lives in kana_punct_table (unicode path) */
    {0},
};

/* punctuation (、 cell): exact unicode through the COMP/Copy path */
struct punct_entry {
    const char *glyph;
    uint32_t cps[5]; /* C L U R D, 0 = none */
};
static const struct punct_entry kana_punct_table[] = {
    { "、。!?", {KEY_COMMA, KEY_DOT, 0xFF1F, 0xFF01, END} }, /* C, L. U？ R！ (COMP: shift lost on umbriel seat path) */
    {0},
};

/* emoji (🙂) face: 10 flick cells, each cell has up to 5 emojis on C L U R D (UTF-8 strings) */
struct emoji_entry {
    const char *glyph;
    const char *dirs[5]; /* C L U R D */
};

static const struct emoji_entry kana_emoji_table[] = {
    { "😇\n🤣 😂 😋\n😌", { "😂", "🤣", "😇", "😋", "😌" } },
    { "😩\n🥺 😭 😔", { "😭", "🥺", "😩", "😔", NULL } },
    { "🤬\n😎 🤔 🤗\n😴", { "🤔", "😎", "🤬", "🤗", "😴" } },
    { "🫩 🫡", { "🫡", "🫩", NULL, NULL, NULL } },
    { "🤝\n👏 👍 👌", { "👍", "👏", "🤝", "👌", NULL } },
    { "✊\n👊 ✌️ ✋\n🖕", { "✌️", "👊", "✊", "✋", "🖕" } },
    { "👆\n👈 👆 👉\n👇", { "👆", "👈", "👆", "👉", "👇" } },
    { "🙆‍♂️\n🙇‍♂️", { "🙇‍♂️", NULL, "🙆‍♂️", NULL, NULL } },
    { "💖\n🩷 ❤️ 🩵\n💜", { "❤️", "🩷", "💖", "🩵", "💜" } },
    { "💯\n🆗 🎉 🆖\n💢", { "🎉", "🆗", "💯", "🆖", "💢" } },
    {0},
};

/* ---------------------------------------------------------------------------
 * Numeric (☆123) face. Same flick engine as the kana faces: label indexes
 * the table, release vector picks the direction. Unicode-only glyphs go
 * through the COMP/Copy path with the FLICK_UC marker in the sequence.
 * ------------------------------------------------------------------------- */
#define C(c) {c}
#define S(k) ((k) | SH) /* shift-wrapped scancode (shifted symbol on that key) */
#define FLICK_UC 0x01 /* flick-seq marker: next 4 bytes = UCS-4 codepoint */
#define UC(cp) {FLICK_UC, ((cp) & 0xFF), (((cp) >> 8) & 0xFF), 0} /* marker + 16-bit codepoint (LE) */

/* Numeric (☆123) face, per user spec (each cell = head + 4 symbols on
 * L/U/R/D, 中左上右下):
 *  1: L:` U:~ R:! D:@   2: L:# U:$ R:% D:^   3: L:& U:* R:- D:=
 *  4: L:← U:↑ R:→ D:↓   5: L:+ U:✕ R:÷ D:×   6: L:{ U:} R:| D:\
 *  7: L:「 U:」 R:： D:☆  8: L:〒 U:々 R:〆 D:○  9: L:; U:: R:' D:"
 *  (: L:) U:[ R:]       0: L:〜 U:… R:・      .: L:- U:/
 * ( 0 . are the three heads of row 4. EVERY symbol that needs Shift on the
 * US layout goes through the COMP path (the shift wrap is lost between
 * umbriel's physical-keymap fold and the client); only unshifted ASCII
 * (digits, - = ; ' , . /) stay as plain scancodes. */
static const struct flick_entry kana_num_table[] = {
    { "~\n` 1 !\n@", { C(KEY_1), UC('`'), UC('~'), UC('!'), UC('@') } },
    { "$\n# 2 %\n^", { C(KEY_2), UC('#'), UC('$'), UC('%'), UC('^') } },
    { "&\n* 3 -\n=", { C(KEY_3), UC('&'), UC('*'), C(KEY_MINUS), C(KEY_EQUAL) } },
    { "↑\n← 4 →\n↓", { C(KEY_4), UC(0x2190), UC(0x2191), UC(0x2192), UC(0x2193) } },
    { "+\n✕ 5 ÷\n×", { C(KEY_5), UC('+'), UC(0x2715), UC(0x00F7), UC(0x00D7) } },
    { "{\n} 6 |\n\\", { C(KEY_6), UC('{'), UC('}'), UC('|'), UC('\\') } },
    { "「\n」 7 ：\n☆", { C(KEY_7), UC(0x300C), UC(0x300D), UC(0xFF1A), UC(0x2606) } },
    { "〒\n々 8 〆\n○", { C(KEY_8), UC(0x3012), UC(0x3005), UC(0x3006), UC(0x25CB) } },
    { ":\n; 9 '\n\"", { C(KEY_9), C(KEY_SEMICOLON), UC(':'), C(KEY_APOSTROPHE), UC('"') } },
    { "()[]", { UC('('), UC(')'), UC('['), UC(']'), END } },
    { "…\n〜 0 ・", { C(KEY_0), UC(0x301C), UC(0x2026), UC(0x30FB), END } },
    { ".,-/", { C(KEY_DOT), C(KEY_COMMA), C(KEY_MINUS), C(KEY_SLASH), END } },
    {0},
};

/* Alphabetic (ABC) face, T9-style: head letter + the rest of the letter
 * group across L/U/R/D (C=a L=b U=c for "abc"; pqrs and wxyz use all four
 * directions). Punctuation cells per the user's row 4:
 *  ': L=' U:" R:( D:)   .: L:? U:! R:, D:-  (Shift cells are layout keys) */
static const struct flick_entry kana_abc_table[] = {
    { "@#/&_", { UC('@'), UC('#'), UC('/'), UC('&'), UC('_') } },
    { "abc", { C(KEY_A), C(KEY_B), C(KEY_C), END, END } },
    { "def", { C(KEY_D), C(KEY_E), C(KEY_F), END, END } },
    { "ghi", { C(KEY_G), C(KEY_H), C(KEY_I), END, END } },
    { "jkl", { C(KEY_J), C(KEY_K), C(KEY_L), END, END } },
    { "mno", { C(KEY_M), C(KEY_N), C(KEY_O), END, END } },
    { "pqrs", { C(KEY_P), C(KEY_Q), C(KEY_R), C(KEY_S), END } },
    { "tuv", { C(KEY_T), C(KEY_U), C(KEY_V), END, END } },
    { "wxyz", { C(KEY_W), C(KEY_X), C(KEY_Y), C(KEY_Z), END } },
    { "'\"()", { C(KEY_APOSTROPHE), UC('"'), UC('('), UC(')'), UC(',') } },
    { ".,?!", { C(KEY_DOT), C(KEY_COMMA), UC('?'), UC('!'), END } },
    {0},
};

/* dakuten (゛) one-shot layer. The grid mirrors the plain flick layer
 * cell-for-cell: only cells WITH a voiced form carry the modified glyph
 * (column-head が ざ だ ば, full 5 directions like their plain parents);
 * everything else is KanaGlyPlain and falls through to the plain table. */
static const struct flick_entry kana_dakuten_table[] = {
    { "ゔ", { {KEY_V, KEY_A}, {KEY_V, KEY_I}, {KEY_V, KEY_U}, {KEY_V, KEY_E}, {KEY_V, KEY_O} } },
    { "が", { {KEY_G, KEY_A}, {KEY_G, KEY_I}, {KEY_G, KEY_U}, {KEY_G, KEY_E}, {KEY_G, KEY_O} } },
    { "ざ", { {KEY_Z, KEY_A}, {KEY_Z, KEY_I}, {KEY_Z, KEY_U}, {KEY_Z, KEY_E}, {KEY_Z, KEY_O} } },
    { "だ", { {KEY_D, KEY_A}, {KEY_D, KEY_I}, {KEY_D, KEY_U}, {KEY_D, KEY_E}, {KEY_D, KEY_O} } },
    { "ば", { {KEY_B, KEY_A}, {KEY_B, KEY_I}, {KEY_B, KEY_U}, {KEY_B, KEY_E}, {KEY_B, KEY_O} } },
    {0},
};

/* handakuten (゜) one-shot layer: only the は-row voices; same mirrored grid */
static const struct flick_entry kana_handakuten_table[] = {
    { "ぱ", { {KEY_P, KEY_A}, {KEY_P, KEY_I}, {KEY_P, KEY_U}, {KEY_P, KEY_E}, {KEY_P, KEY_O} } },
    {0},
};

/* small-kana (小) one-shot layer: same grid, the small glyph replaces each
 * column head; flicks produce the small column (small a-row, っ, ゃゅょ, ゎ) */
static const struct flick_entry kana_small_table[] = {
    { "ぁ", { {KEY_X, KEY_A}, {KEY_X, KEY_I}, {KEY_X, KEY_U}, {KEY_X, KEY_E}, {KEY_X, KEY_O} } },
    { "っ", { {KEY_X, KEY_T, KEY_U}, {KEY_X, KEY_T, KEY_U}, {KEY_X, KEY_T, KEY_U}, {KEY_X, KEY_T, KEY_U}, {KEY_X, KEY_T, KEY_U} } },
    { "ゃ", { {KEY_X, KEY_Y, KEY_A}, {END}, {KEY_X, KEY_Y, KEY_U}, {END}, {KEY_X, KEY_Y, KEY_O} } },
    { "ゎ", { {KEY_X, KEY_W, KEY_A}, {END}, {END}, {END}, {END} } },
    {0},
};

static const struct flick_entry *
table_find(const struct flick_entry *t, const char *glyph)
{
    for (; t->glyph; t++) {
        if (strcmp(t->glyph, glyph) == 0)
            return t;
    }
    return NULL;
}

/* Resolve (layout, label, direction) to a scancode sequence. Falls back to
 * the plain-flick table when the current one-shot layer has no modified form
 * for this glyph (that is how KanaGlyPlain/KanaGlyMut behave). */
static const uint8_t *
flick_lookup(enum layout_id cur, const char *glyph, enum flick_dir dir,
             bool plain_glyph_lookup)
{
    const struct flick_entry *t = kana_flick_table;
    const struct flick_entry *e;
    if (cur == KanaDakuten)
        t = kana_dakuten_table;
    else if (cur == KanaHandakuten)
        t = kana_handakuten_table;
    else if (cur == KanaSmall)
        t = kana_small_table;
    else if (cur == KanaNum)
        t = kana_num_table;
    else if (cur == KanaAbc)
        t = kana_abc_table;
    /* One-shot modifier (dakuten/handakuten/small) layers mirror the plain
     * grid: KanaGlyMut/KanaGlySmall carry the modified column head, but the
     * flick LOOKUP is by the plain head the same cell has on KanaFlick, so
     * both resolve per (cell, direction) identically. */
    e = table_find(t, glyph);
    if (e == NULL && t != kana_flick_table)
        e = table_find(kana_flick_table, glyph);
    if (e == NULL)
        return NULL;
    return e->dirs[dir];
}

/* direction from the touch vector (signs matter) */
static enum flick_dir
kana_flick_dir(int32_t dx, int32_t dy)
{
    int32_t adx = dx < 0 ? -dx : dx;
    int32_t ady = dy < 0 ? -dy : dy;
    if (adx < FLICK_MIN_DIST && ady < FLICK_MIN_DIST)
        return FLICK_C;
    if (adx > ady)
        return dx > 0 ? FLICK_R : FLICK_L;
    else
        return dy > 0 ? FLICK_D : FLICK_U;
}

/* Deliver arbitrary text (including emojis and unicode sequences) by setting the
 * Wayland clipboard (and primary selection) via wl-copy and sending Shift+Insert.
 * Upstream wvkbd's approach of uploading a keymap with COMP (evdev 127 / XKB 135)
 * fails whenever a physical keyboard exists or dynamic keymaps are not relayed,
 * causing Chrome/Chromium to interpret 135 as the Menu (Context Menu / Right-Click) key. */
static void
flick_copy_and_paste(struct kbd *kb, uint32_t time, const char *str)
{
    if (!str || !*str)
        return;

    if (kb->debug)
        fprintf(stderr, "flick paste (sanitized): '%s'\n", str);

    /* Double-fork to perform paste and restore asynchronously without blocking the OSK */
    pid_t p1 = fork();
    if (p1 == 0) {
        pid_t p2 = fork();
        if (p2 == 0) {
            int devnull = open("/dev/null", O_RDWR);
            if (devnull >= 0) {
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
                close(devnull);
            }
            execlp("sh", "sh", "-c",
                   "exec 200>\"${XDG_RUNTIME_DIR:-/run/user/1000}/wvkbd-paste.lock\"\n"
                   "flock 200\n"
                   "has_cb=0; sens_cb=0\n"
                   "if prev_cb=$(wl-paste -n 2>/dev/null); then\n"
                   "    has_cb=1\n"
                   "    if wl-paste -l 2>/dev/null | grep -q 'passwordManagerHint'; then sens_cb=1; fi\n"
                   "fi\n"
                   "has_prim=0; sens_prim=0\n"
                   "if prev_prim=$(wl-paste -p -n 2>/dev/null); then\n"
                   "    has_prim=1\n"
                   "    if wl-paste -p -l 2>/dev/null | grep -q 'passwordManagerHint'; then sens_prim=1; fi\n"
                   "fi\n"
                   "wl-copy --sensitive -- \"$1\"\n"
                   "wl-copy -p --sensitive -- \"$1\"\n"
                   "wtype -M shift -k Insert -m shift\n"
                   "sleep 0.08\n"
                   "if [ $has_cb -eq 1 ]; then\n"
                   "    if [ $sens_cb -eq 1 ]; then wl-copy --sensitive -- \"$prev_cb\"; else wl-copy -- \"$prev_cb\"; fi\n"
                   "else\n"
                   "    wl-copy --clear\n"
                   "fi\n"
                   "if [ $has_prim -eq 1 ]; then\n"
                   "    if [ $sens_prim -eq 1 ]; then wl-copy -p --sensitive -- \"$prev_prim\"; else wl-copy -p -- \"$prev_prim\"; fi\n"
                   "else\n"
                   "    wl-copy -p --clear\n"
                   "fi\n",
                   "sh", str, (char *)NULL);
            _exit(127);
        }
        _exit(0);
    }
    if (p1 > 0)
        waitpid(p1, NULL, 0);
}

static void
flick_send_unicode_str(struct kbd *kb, uint32_t time, const char *str)
{
    flick_copy_and_paste(kb, time, str);
}

static void
flick_send_unicode(struct kbd *kb, uint32_t time, uint32_t uc)
{
    if (kb->debug)
        fprintf(stderr, "unicode: %04x via COMP\n", uc);
    create_and_upload_keymap(kb, kb->layout->keymap_name, uc);
    zwp_virtual_keyboard_v1_modifiers(kb->vkbd, 0, 0, 0, 0);
    zwp_virtual_keyboard_v1_key(kb->vkbd, time, 127, /* COMP key */
                                WL_KEYBOARD_KEY_STATE_PRESSED);
    zwp_virtual_keyboard_v1_key(kb->vkbd, time, 127,
                                WL_KEYBOARD_KEY_STATE_RELEASED);
}

/* current layout id (kb->layout is a pointer into kb->layouts) */

static void
flick_send_key(struct kbd *kb, uint32_t time, uint8_t sc)
{
    if (sc & SH) {
        uint32_t c = sc & ~SH;
        /* wl_keyboard modifiers event carries the shift; the keycode alone
         * got lost between umbriel's physical/virtual keyboard fold and
         * mozc (user got ・ and １ instead of ? and !) */
        zwp_virtual_keyboard_v1_modifiers(kb->vkbd, 1 /* Shift */, 0, 0, 0);
        zwp_virtual_keyboard_v1_key(kb->vkbd, time, c,
                                    WL_KEYBOARD_KEY_STATE_PRESSED);
        zwp_virtual_keyboard_v1_key(kb->vkbd, time, c,
                                    WL_KEYBOARD_KEY_STATE_RELEASED);
        zwp_virtual_keyboard_v1_modifiers(kb->vkbd, 0, 0, 0, 0);
    } else {
        zwp_virtual_keyboard_v1_key(kb->vkbd, time, sc,
                                    WL_KEYBOARD_KEY_STATE_PRESSED);
        zwp_virtual_keyboard_v1_key(kb->vkbd, time, sc,
                                    WL_KEYBOARD_KEY_STATE_RELEASED);
    }
}

/* play a 0-terminated scancode sequence */
static void
flick_send_seq(struct kbd *kb, uint32_t time, const uint8_t *seq)
{
    for (; *seq; seq++) {
        if (*seq == FLICK_UC) {
            /* marker + 16-bit codepoint LE (unicode-only glyphs on the
             * numeric/abc faces): exact character via the COMP/Copy path */
            uint32_t cp = (uint32_t)seq[1] | ((uint32_t)seq[2] << 8);
            flick_send_unicode(kb, time, cp);
            seq += 2;
            continue;
        }
        flick_send_key(kb, time, *seq);
    }
}

static enum layout_id
kana_layout_id(struct kbd *kb)
{
    for (size_t i = 0; i < NumLayouts; i++) {
        if (kb->layout == &kb->layouts[i])
            return i;
    }
    return Full;
}

/* Emit the flick result for a released kana key and handle the one-shot
 * fallback to the plain layer. Called at unpress with the touch vector. */
/* evdev letter scancodes are three non-contiguous runs (QWERTY rows) */
static bool
is_letter_scancode(uint8_t sc)
{
    return (sc >= KEY_Q && sc <= KEY_P) || (sc >= KEY_A && sc <= KEY_L) ||
           (sc >= KEY_Z && sc <= KEY_M);
}

void
kbd_flick_emit(struct kbd *kb, struct key *k, int32_t dx, int32_t dy,
               uint32_t time)
{
    enum flick_dir dir = kana_flick_dir(dx, dy);
    enum layout_id cur = kana_layout_id(kb);
    /* punctuation cell first: exact unicode, no scancode/keymap games */
    for (const struct punct_entry *p = kana_punct_table; p->glyph; p++) {
        if (strcmp(p->glyph, k->label) != 0)
            continue;
        uint32_t cp = p->cps[dir];
        if (kb->debug)
            fprintf(stderr, "flick: glyph='%s' dir=%d -> %04x\n",
                    k->label, dir, cp);
        if (cp >= 0x100)
            flick_send_unicode(kb, time, cp);
        else if (cp)
            flick_send_key(kb, time, (uint8_t)cp);
        if (cur == KanaDakuten || cur == KanaHandakuten || cur == KanaSmall) {
            kbd_switch_layout(kb, &kb->layouts[KanaFlick],
                              kbd_get_layer_index(kb, &kb->layouts[KanaFlick]));
        } else {
            kbd_draw_key(kb, k, Unpress);
        }
        return;
    }
    /* emoji cells: exact unicode through COMP/Copy path */
    for (const struct emoji_entry *p = kana_emoji_table; p->glyph; p++) {
        if (strcmp(p->glyph, k->label) != 0)
            continue;
        const char *em = p->dirs[dir];
        if (kb->debug)
            fprintf(stderr, "flick emoji: glyph='%s' dir=%d -> %s\n",
                    k->label, dir, em ? em : "(none)");
        if (em)
            flick_send_unicode_str(kb, time, em);
        kbd_draw_key(kb, k, Unpress);
        return;
    }
    const uint8_t *seq = flick_lookup(cur, k->label, dir, k->type == KanaGlySmall);
    if (kb->debug) {
        fprintf(stderr, "flick: glyph='%s' dx=%d dy=%d dir=%d -> seq",
                k->label, dx, dy, dir);
        if (seq) {
            for (int i = 0; seq[i]; i++)
                fprintf(stderr, " %03x", seq[i]);
        } else {
            fprintf(stderr, " (none)");
        }
        fprintf(stderr, "\n");
    }
    /* ABC Shift latch or CapsLock: uppercase the letters (via SH wrap),
     * everything else unchanged. The Shift latch consumes on emit; CapsLock persists. */
    bool abc_uppercase = (cur == KanaAbc) && ((kb->mods & Shift) || (kb->mods & CapsLock));
    if (seq && abc_uppercase) {
        bool any_letter = false;
        for (const uint8_t *p = seq; *p; p++) {
            if (*p == FLICK_UC) {
                p += 2;
                continue;
            }
            if (is_letter_scancode(*p)) {
                /* SH wrap: Shift modifiers + the letter keycode */
                flick_send_key(kb, time, *p | SH);
                any_letter = true;
            } else {
                flick_send_key(kb, time, *p);
            }
        }
        if (any_letter && (kb->mods & Shift)) {
            kb->mods &= ~Shift; /* one-shot consume only Shift */
            /* the latch (and thus the ⇧ highlight) changed state: repaint so
             * the key colors match the actual modifier state */
            kbd_draw_layout(kb);
        }
        kbd_draw_key(kb, k, Unpress);
        return;
    }
    if (seq) {
        flick_send_seq(kb, time, seq);
    }
    /* one-shot consumed: fall back to the plain flick layer */
    if (cur == KanaDakuten || cur == KanaHandakuten || cur == KanaSmall) {
        kbd_switch_layout(kb, &kb->layouts[KanaFlick],
                          kbd_get_layer_index(kb, &kb->layouts[KanaFlick]));
    } else {
        /* un-highlight: without this the key keeps the swipe color and
         * reads as "still pressed" (user report) */
        kbd_draw_key(kb, k, Unpress);
    }
}

/* ------------------------------------------------------------------------- */

/* lazy die macro */
#define die(...)                                                               \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(1)

#ifndef KEYMAP
#error "make sure to define KEYMAP"
#endif
#include KEYMAP

void
kbd_switch_layout(struct kbd *kb, struct layout *l, size_t layer_index)
{
    kb->prevlayout = kb->layout;
    if ((kb->layer_index != kb->last_abc_index) && (kb->layout->abc)) {
        kb->last_abc_layout = kb->layout;
        kb->last_abc_index = kb->layer_index;
    }
    kb->layer_index = layer_index;
    kb->layout = l;
    if (kb->debug)
        fprintf(stderr, "Switching to layout %s, layer_index %ld\n",
                kb->layout->name, layer_index);
    if (!l->keymap_name)
        fprintf(stderr, "Layout has no keymap!"); // sanity check
    if ((!kb->prevlayout) ||
        (strcmp(kb->prevlayout->keymap_name, kb->layout->keymap_name) != 0)) {
        fprintf(stderr, "Switching to keymap %s\n", kb->layout->keymap_name);
        create_and_upload_keymap(kb, kb->layout->keymap_name, 0);
    }
    kbd_draw_layout(kb);
}

void
kbd_next_layer(struct kbd *kb, struct key *k, bool invert)
{
    size_t layer_index = kb->layer_index;
    if ((kb->mods & Ctrl) || (kb->mods & Alt) || (kb->mods & AltGr) ||
        ((bool)kb->compose)) {
        // with modifiers ctrl/alt/altgr: switch to the first layer
        layer_index = 0;
        kb->mods = 0;
    } else if ((kb->mods & Shift) || (kb->mods & CapsLock) || (invert)) {
        // with modifiers shift/capslock or invert set: switch to the previous
        // layout in the layer sequence
        if (layer_index > 0) {
            layer_index--;
        } else {
            size_t layercount = 0;
            for (size_t i = 0; layercount == 0; i++) {
                if (kb->landscape) {
                    if (kb->landscape_layers[i] == NumLayouts)
                        layercount = i;
                } else {
                    if (kb->layers[i] == NumLayouts)
                        layercount = i;
                }
            }
            layer_index = layercount - 1;
        }
        if (!invert)
            kb->mods ^= Shift;
    } else {
        // normal behaviour: switch to the next layout in the layer sequence
        layer_index++;
    }
    size_t layercount = 0;
    for (size_t i = 0; layercount == 0; i++) {
        if (kb->landscape) {
            if (kb->landscape_layers[i] == NumLayouts)
                layercount = i;
        } else {
            if (kb->layers[i] == NumLayouts)
                layercount = i;
        }
    }
    if (layer_index >= layercount) {
        if (kb->debug)
            fprintf(stderr, "wrapping layer_index back to start\n");
        layer_index = 0;
    }
    enum layout_id layer;
    if (kb->landscape) {
        layer = kb->landscape_layers[layer_index];
    } else {
        layer = kb->layers[layer_index];
    }
    if (((bool)kb->compose) && (k)) {
        kb->compose = 0;
        kbd_draw_key(kb, k, Unpress);
    }
    kbd_switch_layout(kb, &kb->layouts[layer], layer_index);
}

uint8_t
kbd_get_rows(struct layout *l)
{
    uint8_t rows = 0;
    struct key *k = l->keys;
    while (k->type != Last) {
        if (k->type == EndRow) {
            rows++;
        }
        k++;
    }
    return rows + 1;
}

enum layout_id *
kbd_init_layers(char *layer_names_list)
{
    enum layout_id *layers;
    uint8_t numlayers = 0;
    bool found;
    char *s;
    int i;

    layers = malloc(MAX_LAYERS * sizeof(enum layout_id));
    s = strtok(layer_names_list, ",");
    while (s != NULL) {
        if (numlayers + 1 == MAX_LAYERS) {
            fprintf(stderr, "too many layers specified");
            exit(3);
        }
        found = false;
        for (i = 0; i < NumLayouts - 1; i++) {
            if (layouts[i].name && strcmp(layouts[i].name, s) == 0) {
                fprintf(stderr, "layer #%d = %s\n", numlayers + 1, s);
                layers[numlayers++] = i;
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "No such layer: %s\n", s);
            exit(3);
        }
        s = strtok(NULL, ",");
    }
    layers[numlayers] = NumLayouts; // mark the end of the sequence
    if (numlayers == 0) {
        fprintf(stderr, "No layers defined\n");
        exit(3);
    }

    return layers;
}

void
kbd_init(struct kbd *kb, struct layout *layouts, char *layer_names_list,
         char *landscape_layer_names_list)
{
    int i;

    fprintf(stderr, "Initializing keyboard\n");

    kb->layouts = layouts;

    for (i = 0; i < NumLayouts - 1; i++)
        ;
    fprintf(stderr, "Found %d layouts\n", i);

    kb->layer_index = 0;
    kb->last_abc_index = 0;

    if (layer_names_list)
        kb->layers = kbd_init_layers(layer_names_list);
    if (landscape_layer_names_list)
        kb->landscape_layers = kbd_init_layers(landscape_layer_names_list);

    i = 0;
    enum layout_id lid = kb->layers[0];
    while (lid != NumLayouts) {
        lid = kb->layers[++i];
    }
    fprintf(stderr, "Found %d layers\n", i);

    enum layout_id layer;
    if (kb->landscape) {
        layer = kb->landscape_layers[kb->layer_index];
    } else {
        layer = kb->layers[kb->layer_index];
    }

    kb->layout = &kb->layouts[layer];
    kb->last_abc_layout = &kb->layouts[layer];

    /* upload keymap */
    create_and_upload_keymap(kb, kb->layout->keymap_name, 0);
}

void
kbd_init_layout(struct layout *l, uint32_t width, uint32_t height)
{
    uint32_t x = 0, y = 0;
    uint8_t rows = kbd_get_rows(l);

    l->keyheight = height / rows;

    struct key *k = l->keys;
    double rowlength = kbd_get_row_length(k);
    double rowwidth = 0.0;
    while (k->type != Last) {
        if (k->type == EndRow) {
            y += l->keyheight;
            x = 0;
            rowwidth = 0.0;
            rowlength = kbd_get_row_length(k + 1);
        } else if (k->width > 0) {
            k->x = x;
            k->y = y;
            k->w = ((double)width / rowlength) * k->width;
            x += k->w;
            rowwidth += k->width;
            if (x < (rowwidth / rowlength) * (double)width) {
                k->w++;
                x++;
            }
        }
        k->h = l->keyheight;
        k++;
    }

    /* kana flick faces: the (2,5) Enter merges the last two rows of the
     * right column (the (3,5) slot is a Pad underneath it). Detect the
     * pattern post-layout: if the key directly below the last row's last
     * cell is a Pad, extend the Enter key's height over it. */
    k = l->keys;
    struct key *k_last = NULL;    /* last key of row 2 */
    struct key *k_below = NULL;   /* the Pad under it */
    uint32_t row2_y = 2 * l->keyheight;
    while (k->type != Last) {
        if ((k->type != EndRow) && (k->width > 0)) {
            if (k->y == row2_y) {
                k_last = k;
            } else if (k_last && k->y == row2_y + l->keyheight &&
                       k->x == k_last->x) {
                k_below = k;
                break;
            }
        }
        k++;
    }
    /* the Pad under the Enter is invisible and receives no events, so
     * extending the Enter over it only changes rendering */
    if (k_last && k_below && k_below->type == Pad &&
        k_last->type == Code && k_last->code == KEY_ENTER) {
        k_last->h = l->keyheight * 2;
    }
}

double
kbd_get_row_length(struct key *k)
{
    double l = 0.0;
    while ((k->type != Last) && (k->type != EndRow)) {
        l += k->width;
        k++;
    }
    return l;
}

struct key *
kbd_get_key(struct kbd *kb, uint32_t x, uint32_t y)
{
    struct layout *l = kb->layout;
    struct key *k = l->keys;
    if (kb->debug)
        fprintf(stderr, "get key: +%d+%d\n", x, y);
    while (k->type != Last) {
        if ((k->type != EndRow) && (k->type != Pad) && (k->type != KanaHide) &&
            (x >= k->x) && (y >= k->y) && (x < k->x + k->w) &&
            (y < k->y + k->h)) {
            return k;
        }
        k++;
    }
    return NULL;
}

size_t
kbd_get_layer_index(struct kbd *kb, struct layout *l)
{
    for (size_t i = 0; i < NumLayouts - 1; i++) {
        if (l == &kb->layouts[i]) {
            return i;
        }
    }
    return 0;
}

void
zwp_virtual_keyboard_v1_key_mods(struct zwp_virtual_keyboard_v1 *vkbd, uint32_t time, uint32_t mods, enum wl_keyboard_key_state state)
{
    if ((mods & Shift) == Shift)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_LEFTSHIFT, state);
    if ((mods & CapsLock) == CapsLock)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_CAPSLOCK, state);
    if ((mods & Ctrl) == Ctrl)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_LEFTCTRL, state);
    if ((mods & Alt) == Alt)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_LEFTALT, state);
    if ((mods & Super) == Super)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_LEFTMETA, state);
    if ((mods & AltGr) == AltGr)
        zwp_virtual_keyboard_v1_key(vkbd, time, KEY_RIGHTALT, state);
}

void
kbd_unpress_key(struct kbd *kb, uint32_t time)
{
    bool unlatch_shift, unlatch_ctrl, unlatch_alt, unlatch_super, unlatch_altgr, unlatch_tab;
    unlatch_shift = unlatch_ctrl = unlatch_alt = unlatch_super = unlatch_altgr = unlatch_tab = false;
    if (kb->last_press) {
        unlatch_shift = (kb->mods & Shift) == Shift;
        unlatch_ctrl = (kb->mods & Ctrl) == Ctrl;
        unlatch_alt = (kb->mods & Alt) == Alt;
        unlatch_super = (kb->mods & Super) == Super;
        unlatch_altgr = (kb->mods & AltGr) == AltGr;

        if (kb->last_press->type == KanaGly ||
            kb->last_press->type == KanaGlyMut ||
            kb->last_press->type == KanaGlyPlain ||
            kb->last_press->type == KanaGlySmall) {
            /* kana keys emit nothing on press: at release (here) the touch
             * vector picks the flick direction and the modifier-layer
             * one-shot is consumed. No keycode release to send. */
            struct key *k = kb->last_press;
            int32_t dx = (int32_t)kb->cur_up_x - (int32_t)kb->cur_down_x;
            int32_t dy = (int32_t)kb->cur_up_y - (int32_t)kb->cur_down_y;
            kbd_flick_emit(kb, k, dx, dy, time);
            kb->last_press = NULL;
            return;
        } else if (kb->last_press->type == KanaOn ||
                   kb->last_press->type == KanaOff) {
            /* IME toggle + layer switch at unpress: both switch between a
             * 4-row flick layout and the 5-row QWERTY ones, so switching at
             * press would re-hit-test the release on a different grid.
             * KanaOn's KEY_HIRAGANA was already sent by the Code press
             * path (shared union member); KanaOff sends KEY_MUHENKAN here
             * (nothing was sent at press). The layer switch itself is
             * performed by kbd_release_key via kb->kana_pending_switch. */
            kb->kana_pending_switch = kb->last_press->type;
            if (kb->last_press->type == KanaOff) {
                zwp_virtual_keyboard_v1_key(kb->vkbd, time, KEY_MUHENKAN,
                                            WL_KEYBOARD_KEY_STATE_PRESSED);
                zwp_virtual_keyboard_v1_key(kb->vkbd, time, KEY_MUHENKAN,
                                            WL_KEYBOARD_KEY_STATE_RELEASED);
            }
            kbd_draw_key(kb, kb->last_press, Unpress);
            kb->last_press = NULL;
            return;
        } else if (kb->last_press->type == Copy) {
            if (kb->debug) fprintf(stderr, "release copy key (unlatch_shift=%d, mods=%d)\n", unlatch_shift, kb->mods);
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, 127, // COMP key
                                        WL_KEYBOARD_KEY_STATE_RELEASED);
        } else {
            if (kb->debug) fprintf(stderr, "release key %d", kb->last_press->code);
            if ((kb->shift_space_is_tab) && (kb->last_press->code == KEY_SPACE) && (unlatch_shift)) {
                // shift + space is tab
                unlatch_shift = false;
                unlatch_tab = true;
                kb->mods ^= Shift;
                zwp_virtual_keyboard_v1_key(kb->vkbd, time, KEY_TAB,
                                            WL_KEYBOARD_KEY_STATE_RELEASED);
            } else {
                zwp_virtual_keyboard_v1_key(kb->vkbd, time,
                                            unlatch_shift && kb->last_press->shift_code ?
                                            kb->last_press->shift_code : kb->last_press->code,
                                            WL_KEYBOARD_KEY_STATE_RELEASED);
            }
        }

        if (unlatch_shift) {
            if (kb->debug) fprintf(stderr, "unlatch shift");
            kb->mods ^= Shift;
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Shift, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        if (unlatch_ctrl) {
            kb->mods ^= Ctrl;
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Ctrl, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        if (unlatch_alt) {
            kb->mods ^= Alt;
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Alt, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        if (unlatch_super) {
            kb->mods ^= Super;
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Super, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        if (unlatch_altgr) {
            kb->mods ^= AltGr;
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, AltGr, WL_KEYBOARD_KEY_STATE_RELEASED);
        }

        if (unlatch_shift||unlatch_ctrl||unlatch_alt||unlatch_super||unlatch_altgr||unlatch_tab) {
            zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods, 0, 0, 0);
        }

        if (kb->compose >= 2) {
            kb->compose = 0;
            kbd_switch_layout(kb, kb->last_abc_layout, kb->last_abc_index);
        } else if (unlatch_shift||unlatch_ctrl||unlatch_alt||unlatch_super||unlatch_altgr||unlatch_tab) {
            kbd_draw_layout(kb);
        } else {
            kbd_draw_key(kb, kb->last_press, Unpress);
        }

        kb->last_press = NULL;
    }
}

void
kbd_release_key(struct kbd *kb, uint32_t time)
{
    kbd_unpress_key(kb, time);
    /* deferred kana layer switch (KanaOn/KanaOff): runs AFTER unpress, when
     * no key is held, so the release's hit-testing never happens against
     * the next layout (5-row <-> 4-row). */
    if (kb->kana_pending_switch) {
        if (kb->kana_pending_switch == KanaOn) {
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, KEY_HIRAGANA,
                                        WL_KEYBOARD_KEY_STATE_RELEASED);
            if (kb->mods & Shift) {
                zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Shift,
                                                WL_KEYBOARD_KEY_STATE_RELEASED);
                kb->mods &= ~Shift;
            }
            if (kb->mods & CapsLock) {
                zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, CapsLock,
                                                WL_KEYBOARD_KEY_STATE_RELEASED);
                kb->mods &= ~CapsLock;
            }
            zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods, 0, 0, 0);
            kbd_switch_layout(kb, &kb->layouts[KanaFlick],
                              kbd_get_layer_index(kb, &kb->layouts[KanaFlick]));
        } else {
            /* cleanup: KanaOn's press sent KEY_HIRAGANA with no release
             * (kana keys short-circuit unpress) - release it here */
            kbd_switch_layout(kb, &kb->layouts[Full],
                              kbd_get_layer_index(kb, &kb->layouts[Full]));
        }
        kb->kana_pending_switch = 0;
    }
    if (kb->print_intersect && kb->last_swipe) {
        printf("\n");
        // Important so autocompleted words get typed in time
        fflush(stdout);
        kbd_draw_layout(kb);
        kb->last_swipe = NULL;
    }

    kbd_clear_last_popup(kb);
}

void
kbd_motion_key(struct kbd *kb, uint32_t time, uint32_t x, uint32_t y)
{
    struct key *intersect_key;
    intersect_key = kbd_get_key(kb, x, y);

    // Ignore keyboard motion until a new key is reached.
    if (kb->last_press == intersect_key) {
        return;
    }

    // Output intersecting keys
    // (for external 'swiping'-based accelerators).
    if (kb->print_intersect) {
        if (kb->last_press) {
            kbd_unpress_key(kb, time);
            // Redraw last press as a swipe.
            kbd_draw_key(kb, kb->last_swipe, Swipe);
        }

        if (intersect_key && (!kb->last_swipe ||
                              intersect_key->label != kb->last_swipe->label)) {
            kbd_print_key_stdout(kb, intersect_key);
            kb->last_swipe = intersect_key;
            kbd_draw_key(kb, kb->last_swipe, Swipe);
        }
    } else if (kb->last_press &&
               (kb->last_press->type == KanaGly ||
                kb->last_press->type == KanaGlyMut ||
                kb->last_press->type == KanaGlyPlain ||
                kb->last_press->type == KanaGlySmall)) {
        /* kana press in progress: keep it held (visual), DO NOT unpress on
         * motion, or the flick vector is lost; just track the up point so
         * the release direction can also use the last motion instead of the
         * raw up coords if a motion arrived after crossing the key edge */
        kb->cur_up_x = x;
        kb->cur_up_y = y;
        kbd_clear_last_popup(kb);
    } else {
        kbd_unpress_key(kb, time);
    }

    kbd_clear_last_popup(kb);
}

void
kbd_press_key(struct kbd *kb, struct key *k, uint32_t time)
{
    if ((kb->compose == 1) && (k->type != Compose) && (k->type != Mod)) {
        if ((k->type == NextLayer) || (k->type == BackLayer) ||
            ((k->type == Code) && (k->code == KEY_SPACE))) {
            kb->compose = 0;
            if (kb->debug)
                fprintf(stderr, "showing layout index\n");
            kbd_switch_layout(kb, &kb->layouts[Index], 0);
            return;
        } else if (k->layout) {
            kb->compose++;
            if (kb->debug)
                fprintf(stderr, "showing compose %d\n", kb->compose);
            kbd_switch_layout(kb, k->layout,
                              kbd_get_layer_index(kb, k->layout));
            return;
        } else {
            return;
        }
    }

    switch (k->type) {
    case KanaAnchor:
        /* the あいう cell: leave any one-shot layer, back to the plain face */
        kbd_switch_layout(kb, &kb->layouts[KanaFlick],
                          kbd_get_layer_index(kb, &kb->layouts[KanaFlick]));
        break;
    case KanaNoop:
    case KanaHide:
        /* phase-1 placeholder / hidden slot: drawn only, reacts to nothing */
        kb->last_swipe = kb->last_press = k;
        kbd_draw_key(kb, k, Press);
        break;
    case KanaMod:
        /* one-shot modifier layer (or a face switch like ☆123/ABC when the
         * key carries a code): switch immediately so the labels change
         * while the finger is still down (visual feedback requirement);
         * safe because these layers have the SAME 4-row grid. A nonzero
         * code is a plain keycode sent on press (e.g. KEY_MUHENKAN = IME
         * off when leaving the kana face) - no release here; kbd_unpress_key
         * does NOT touch KanaMod, so release it right away. */
        if (k->code) {
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, k->code,
                                        WL_KEYBOARD_KEY_STATE_PRESSED);
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, k->code,
                                        WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        kbd_switch_layout(kb, k->layout, kbd_get_layer_index(kb, k->layout));
        break;
    case KanaCancel:
        /* second tap on the modifier layer's own glyph (or the cancel
         * cell): back to the plain flick layer without sending anything */
        kbd_switch_layout(kb, k->layout, kbd_get_layer_index(kb, k->layout));
        break;
    case KanaGly:
    case KanaGlyMut:
    case KanaGlyPlain:
    case KanaGlySmall:
        /* register only; emission happens at unpress with the vector */
        kb->last_swipe = kb->last_press = k;
        kbd_draw_key(kb, k, Press);
        break;
    case KanaOn:
    case KanaOff:
        /* keycode on press (Code-style), layer switch at unpress */
        kb->last_swipe = kb->last_press = k;
        kbd_draw_key(kb, k, Press);
        if (k->type == KanaOn)
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, k->code,
                                        WL_KEYBOARD_KEY_STATE_PRESSED);
        break;
    case Code:
        if (k->code_mod) {
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, k->code_mod, WL_KEYBOARD_KEY_STATE_PRESSED);
            if (k->reset_mod) {
                zwp_virtual_keyboard_v1_modifiers(kb->vkbd, k->code_mod, 0, 0,
                                                  0);
            } else {
                zwp_virtual_keyboard_v1_modifiers(
                    kb->vkbd, kb->mods ^ k->code_mod, 0, 0, 0);
            }
        } else {
            zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods, 0, 0, 0);
        }
        kb->last_swipe = kb->last_press = k;
        kb->last_press_offset = time - monotime();
        kbd_draw_key(kb, k, Press);
        if ((kb->shift_space_is_tab) && (k->code == KEY_SPACE) && (kb->mods & Shift)) {
            // shift space is tab
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, Shift, WL_KEYBOARD_KEY_STATE_RELEASED);
            zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods ^ Shift, 0, 0, 0);
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, KEY_TAB,
                                        WL_KEYBOARD_KEY_STATE_PRESSED);
        } else {
            zwp_virtual_keyboard_v1_key(kb->vkbd, time, (kb->mods & Shift) && k->shift_code ?
                                        k->shift_code : k->code,
                                        WL_KEYBOARD_KEY_STATE_PRESSED);
        }
        if (kb->print || kb->print_intersect)
            kbd_print_key_stdout(kb, k);
        if (kb->compose) {
            if (kb->debug)
                fprintf(stderr, "pressing composed key\n");
            kb->compose++;
        }
        break;
    case Mod:
        kb->mods ^= k->code;
        if (kb->mods & k->code) {
            if (kb->debug)
                fprintf(stderr, "mod pressed\n");
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, k->code, WL_KEYBOARD_KEY_STATE_PRESSED);
        } else {
            if (kb->debug)
                fprintf(stderr, "mod released\n");
            zwp_virtual_keyboard_v1_key_mods(kb->vkbd, time, k->code, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
        if ((k->code == Shift) || (k->code == CapsLock)) {
            kbd_draw_layout(kb);
        } else {
            if (kb->mods & k->code) {
                kbd_draw_key(kb, k, Press);
            } else {
                kbd_draw_key(kb, k, Unpress);
            }
        }
        zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods, 0, 0, 0);
        break;
    case Layout:
        // switch to the layout determined by the key
        kbd_switch_layout(kb, k->layout, kbd_get_layer_index(kb, k->layout));
        // reset previous layout to default/first so we don't get any weird
        // cycles
        kb->last_abc_index = 0;
        if (kb->landscape) {
            kb->last_abc_layout = &kb->layouts[kb->landscape_layers[0]];
        } else {
            kb->last_abc_layout = &kb->layouts[kb->layers[0]];
        }
        break;
    case Compose:
        // switch to the associated layout determined by the *next* keypress
        if (kb->compose == 0) {
            kb->compose = 1;
        } else {
            kb->compose = 0;
        }
        if ((bool)kb->compose) {
            kbd_draw_key(kb, k, Press);
        } else {
            kbd_draw_key(kb, k, Unpress);
        }
        break;
    case NextLayer: //(also handles previous layer when shift modifier is on, or
                    //"first layer" with other modifiers)
        kbd_next_layer(kb, k, false);
        break;
    case BackLayer: // triggered when "Abc" keys are pressed
        // switch to the last active alphabetical layout
        if (kb->last_abc_layout) {
            kb->compose = 0;
            kbd_switch_layout(kb, kb->last_abc_layout, kb->last_abc_index);
            // reset previous layout to default/first so we don't get any weird
            // cycles
            kb->last_abc_index = 0;
            if (kb->landscape) {
                kb->last_abc_layout = &kb->layouts[kb->landscape_layers[0]];
            } else {
                kb->last_abc_layout = &kb->layouts[kb->layers[0]];
            }
        }
        break;
    case Copy:
        // copy code as unicode chr by setting a temporary keymap
        kb->last_swipe = kb->last_press = k;
        kb->last_press_offset = time - monotime();
        kbd_draw_key(kb, k, Press);
        if (kb->mods & Shift) {
            if (kb->debug)
                    fprintf(stderr, "Pressing copy key (with shift)\n");
            create_and_upload_keymap(kb, kb->layout->keymap_name, k->code_mod);
        } else {
            if (kb->debug)
                    fprintf(stderr, "Pressing copy key\n");
            create_and_upload_keymap(kb, kb->layout->keymap_name, k->code);
        }
        zwp_virtual_keyboard_v1_modifiers(kb->vkbd, kb->mods, 0, 0, 0);
        zwp_virtual_keyboard_v1_key(kb->vkbd, time, 127, // COMP key
                                    WL_KEYBOARD_KEY_STATE_PRESSED);
        if (kb->print || kb->print_intersect)
            kbd_print_key_stdout(kb, k);
        break;
    default:
        break;
    }
}

void
kbd_print_key_stdout(struct kbd *kb, struct key *k)
{
    /* Printed keys may slightly differ from the actual output
     * we generally print what is on the key LABEL and only support the normal
     * and shift layers. Other modifiers produce no output (Ctrl,Alt).
     * If a label has more than one character,
     * only the first character is printed,
     * because downpipe programs otherwise have no way of differentiating a swipe
     * from a single keypress.
     * */

    bool handled = true;
    if (k->type == Code) {
        switch (k->code) {
        case KEY_SPACE:
            printf(" ");
            break;
        case KEY_ENTER:
            printf("\n");
            break;
        case KEY_BACKSPACE:
            printf("\b");
            break;
        case KEY_TAB:
            printf("\t");
            break;
        default:
            handled = false;
            break;
        }
    } else if (k->type != Copy) {
        return;
    }

    if (!handled) {
        if ((kb->mods & Shift) || 
            ((kb->mods & CapsLock) & (strlen(k->label) == 1 && isalpha(k->label[0]))))
            kbd_print_first_utf8_char_stdout(k->shift_label);
        else if (!(kb->mods & Ctrl) && !(kb->mods & Alt) && !(kb->mods & Super))
            kbd_print_first_utf8_char_stdout(k->label);
    }
    fflush(stdout);
}

void kbd_print_first_utf8_char_stdout(const char *str) {
    unsigned char c = (unsigned char)str[0];
    int len;

    if ((c & 0x80) == 0) {
        len = 1;
    } else if ((c & 0xE0) == 0xC0) {
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
    } else {
        len = 1; // Invalid UTF-8
    }

    for (int i = 0; i < len && str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

void
kbd_clear_last_popup(struct kbd *kb)
{
    if (kb->last_popup_w && kb->last_popup_h) {
        drw_do_clear(kb->popup_surf, kb->last_popup_x, kb->last_popup_y,
                     kb->last_popup_w, kb->last_popup_h);
        kb->last_popup_w = kb->last_popup_h = 0;
    }
}

void
kbd_draw_key(struct kbd *kb, struct key *k, enum key_draw_type type)
{
    const char *label = k->label;
    if (k->shift_label[0] &&
        ((kb->mods & Shift) ||
         ((kb->mods & CapsLock) &&
          ((strlen(k->label) == 1 && isalpha(k->label[0])) ||
           kbd_get_layer_index(kb, kb->layout) == KanaAbc))))
        label = k->shift_label;
    if (kb->debug)
        fprintf(stderr, "Draw key +%d+%d %dx%d -> %s\n", k->x, k->y, k->w, k->h,
                label);
    struct clr_scheme *scheme = &kb->schemes[k->scheme];

    switch (type) {
    case None:
    case Unpress:
        draw_inset(kb->surf, k->x, k->y, k->w, k->h, KBD_KEY_BORDER,
                   scheme->fg, scheme->rounding);
        drw_draw_text(kb->surf, scheme->text, k->x, k->y, k->w, k->h,
                  KBD_KEY_BORDER, label, scheme->font_description);
        break;
    case Press:
        draw_inset(kb->surf, k->x, k->y, k->w, k->h, KBD_KEY_BORDER,
                   kb->show_highlight ? scheme->high : scheme->fg,
                   scheme->rounding);
        drw_draw_text(kb->surf,
                  kb->show_highlight ? scheme->text_press : scheme->text,
                  k->x, k->y, k->w, k->h, KBD_KEY_BORDER, label,
                  scheme->font_description);
        break;
    case Swipe:
        draw_over_inset(kb->surf, k->x, k->y, k->w, k->h, KBD_KEY_BORDER,
                        scheme->swipe, scheme->rounding);
        drw_draw_text(kb->surf, scheme->text_swipe, k->x, k->y, k->w, k->h,
                  KBD_KEY_BORDER, label, scheme->font_description);
        break;
    default:
        drw_draw_text(kb->surf, scheme->text, k->x, k->y, k->w, k->h,
                  KBD_KEY_BORDER, label, scheme->font_description);
    }


    if (0) { /* popups disabled: flick overlay would cover the fingers' cells */
        kbd_clear_last_popup(kb);

        kb->last_popup_x = k->x;
        kb->last_popup_y = kb->h + k->y - k->h;
        kb->last_popup_w = k->w;
        kb->last_popup_h = k->h;

        drw_fill_rectangle(kb->popup_surf, scheme->bg, k->x,
                           kb->last_popup_y, k->w, k->h, scheme->rounding);
        draw_inset(kb->popup_surf, k->x, kb->last_popup_y, k->w, k->h,
                   KBD_KEY_BORDER,
                   kb->show_highlight ? scheme->high : scheme->fg,
                   scheme->rounding);
        drw_draw_text(kb->popup_surf,
                      kb->show_highlight ? scheme->text_press : scheme->text,
                      k->x, kb->last_popup_y, k->w, k->h, KBD_KEY_BORDER, label,
                      scheme->font_description);
    }
}

void
kbd_draw_layout(struct kbd *kb)
{
    struct drwsurf *d = kb->surf;
    struct key *next_key = kb->layout->keys;
    if (kb->debug)
        fprintf(stderr, "Draw layout\n");

    if (kb->corner_radius > 0) {
        drw_do_clear(d, 0, 0, kb->w, kb->h);
        drw_fill_rectangle(d, kb->schemes[0].bg, 0, 0, kb->w, kb->h, kb->corner_radius);
    } else {
        drw_fill_rectangle(d, kb->schemes[0].bg, 0, 0, kb->w, kb->h, 0);
    }

    while (next_key->type != Last) {
        if ((next_key->type == Pad) || (next_key->type == EndRow)) {
            next_key++;
            continue;
        }
        if ((next_key->type == Mod && kb->mods & next_key->code) ||
            (next_key->type == Compose && kb->compose)) {
            kbd_draw_key(kb, next_key, Press);
        } else {
            kbd_draw_key(kb, next_key, None);
        }
        next_key++;
    }
}

void
kbd_resize(struct kbd *kb, struct layout *layouts, uint8_t layoutcount)
{
    fprintf(stderr, "Resize %dx%d %f, %d layouts\n", kb->w, kb->h, kb->scale,
            layoutcount);

    drwsurf_resize(kb->surf, kb->w, kb->h, kb->scale);
    drwsurf_resize(kb->popup_surf, kb->w, kb->h * 2, kb->scale);
    for (int i = 0; i < layoutcount; i++) {
        if (kb->debug) {
            if (layouts[i].name)
                fprintf(stderr, "Initialising layout %s, keymap %s\n",
                        layouts[i].name, layouts[i].keymap_name);
            else
                fprintf(stderr, "Initialising unnamed layout %d, keymap %s\n",
                        i, layouts[i].keymap_name);
        }
        kbd_init_layout(&layouts[i], kb->w, kb->h);
    }
    kbd_draw_layout(kb);
}

void
draw_inset(struct drwsurf *ds, uint32_t x, uint32_t y, uint32_t width,
           uint32_t height, uint32_t border, Color color, int rounding)
{
    drw_fill_rectangle(ds, color, x + border, y + border, width - (border * 2),
                       height - (border * 2), rounding);
}
void
draw_over_inset(struct drwsurf *ds, uint32_t x, uint32_t y, uint32_t width,
                uint32_t height, uint32_t border, Color color, int rounding)
{
    drw_over_rectangle(ds, color, x + border, y + border, width - (border * 2),
                       height - (border * 2), rounding);
}

void
create_and_upload_keymap(struct kbd *kb, const char *name, uint32_t comp_unichr)
{
    int keymap_index = -1;
    for (int i = 0; i < NUMKEYMAPS; i++) {
        if (!strcmp(keymap_names[i], name)) {
            keymap_index = i;
        }
    }
    if (keymap_index == -1) {
        fprintf(stderr, "No such keymap defined: %s\n", name);
        exit(9);
    }
    const char *keymap_template = keymaps[keymap_index];
    size_t keymap_size = strlen(keymap_template) + 64;
    char *keymap_str = malloc(keymap_size);
    sprintf(keymap_str, keymap_template, comp_unichr, comp_unichr);
    keymap_size = strlen(keymap_str);
    int keymap_fd = os_create_anonymous_file(keymap_size);
    if (keymap_fd < 0) {
        die("could not create keymap fd\n");
    }
    void *ptr = mmap(NULL, keymap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                     keymap_fd, 0);
    if (ptr == (void *)-1) {
        die("could not map keymap data\n");
    }
    if (kb->vkbd == NULL) {
        die("kb.vkbd = NULL\n");
    }
    strcpy(ptr, keymap_str);
    zwp_virtual_keyboard_v1_keymap(kb->vkbd, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                   keymap_fd, keymap_size);
    free((void *)keymap_str);
}

// Get a monotonic millisecond timer.
int64_t
monotime()
{
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);

    return (spec.tv_sec * 1000 + spec.tv_nsec / 1000000);
}
