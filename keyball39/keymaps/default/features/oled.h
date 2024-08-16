#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
  // デバッグ用に変数を表示する
  // oled_write_P(PSTR("Debug:"), false);
  // oled_write(get_u8_str(xxx, ' '), false);

  keyball_oled_render_keyinfo();   // キー情報を表示
  keyball_oled_render_ballinfo();  // トラックボール情報を表示
  // keyball_oled_render_layerinfo();  // トラックボール情報を表示

  // <Layer>を表示する
  oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
  for (uint8_t i = 1; i < DYNAMIC_KEYMAP_LAYER_COUNT; i++) {
      oled_write_char(keyball_get_oled_layer_char(i), false);
  }

  // <マウス移動量 / クリックレイヤーしきい値>を表示
  // oled_write_P(PSTR(" MV:"), false);
  // oled_write(get_u8_str(mouse_movement, ' '), false);
  // oled_write_P(PSTR("/"), false);
  // oled_write(get_u8_str(to_clickable_movement, ' '), false);

  // <state>を表示
  switch (state) {
    case WAITING:
      oled_write_ln_P(PSTR(" WAITING"), false);
      break;
    case CLICKABLE:
      oled_write_ln_P(PSTR(" CLICKABLE"), false);
      break;
    case CLICKING:
      oled_write_ln_P(PSTR(" CLICKING"), false);
      break;
    case CLICKED:
      oled_write_ln_P(PSTR(" CLICKED"), false);
      break;
    case NONE:
      oled_write_ln_P(PSTR(" NONE"), false);
      break;
  }
}
#endif
