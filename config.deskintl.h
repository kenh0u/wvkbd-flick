#ifndef config_h_INCLUDED
#define config_h_INCLUDED

#define DEFAULT_FONT "Sans 18"
#define DEFAULT_ROUNDING 5
static const int transparency = 255;

struct clr_scheme schemes[] = {
{
  /* colors */
  .bg = {.bgra = {15, 15, 15, transparency}},
  .fg = {.bgra = {45, 45, 45, transparency}},
  .high = {.bgra = {100, 100, 100, transparency}},
  .swipe = {.bgra = {100, 255, 100, 64}},
  .text = {.color = UINT32_MAX},
  .text_press = {.color = UINT32_MAX},
  .text_swipe = {.color = UINT32_MAX},
  .font = DEFAULT_FONT,
  .rounding = DEFAULT_ROUNDING,
},
{
  /* colors */
  .bg = {.bgra = {15, 15, 15, transparency}},
  .fg = {.bgra = {32, 32, 32, transparency}},
  .high = {.bgra = {100, 100, 100, transparency}},
  .swipe = {.bgra = {100, 255, 100, 64}},
  .text_press = {.color = UINT32_MAX},
  .text_swipe = {.color = UINT32_MAX},
  .text = {.color = UINT32_MAX},
  .font = DEFAULT_FONT,
  .rounding = DEFAULT_ROUNDING,
},
/* scheme 2: KanaHide -- invisible key on one-shot modifier layers. Same
 * colors as the keyboard background so the cell reads as a hole. */
{
  .bg = {.bgra = {15, 15, 15, transparency}},
  .fg = {.bgra = {15, 15, 15, transparency}},
  .high = {.bgra = {15, 15, 15, transparency}},
  .swipe = {.bgra = {15, 15, 15, 0}},
  .text = {.color = 0},
  .font = DEFAULT_FONT,
  .rounding = DEFAULT_ROUNDING,
},
/* scheme 3: Emoji face keys -- Sans 13 so 3-line emoji labels fit comfortably */
{
  .bg = {.bgra = {15, 15, 15, transparency}},
  .fg = {.bgra = {45, 45, 45, transparency}},
  .high = {.bgra = {100, 100, 100, transparency}},
  .swipe = {.bgra = {100, 255, 100, 64}},
  .text_press = {.color = UINT32_MAX},
  .text_swipe = {.color = UINT32_MAX},
  .text = {.color = UINT32_MAX},
  .font = "Sans 13",
  .rounding = DEFAULT_ROUNDING,
}
};

/* layers is an ordered list of layouts, used to cycle through */
static enum layout_id layers[] = {
  Full, // First layout is the default layout on startup
  Special,
  NumLayouts // signals the last item, may not be omitted
};

/* layers is an ordered list of layouts, used to cycle through */
static enum layout_id landscape_layers[] = {
  Full, // First layout is the default layout on startup
  Special,
  NumLayouts // signals the last item, may not be omitted
};

#endif // config_h_INCLUDED
