# HomeTUI
A TUI library for home TUI-based projects

All pmh-* fonts are taken from: https://github.com/PhMajerus/FIGfonts

## TODO:
- [ ] Pass custom on/off text to checkbox
- [ ] Text box text should fit the box (render last letters)
- [ ] **Memory Leaks**:
  - [ ] Implement `ui_destroy` to free the UI context and its controls
  - [ ] Implement `dispose_textures` in `ncurses_io.c`
  - [ ] Close file descriptor on error paths in `load_figlet_texture`
  - [ ] Check for allocation failure of `t->textures` in `allocate_texture_atlas`
- [ ] **Buffer Overflows & Safety**:
  - [ ] Add bounds checking against `box->text_buf_len` when typing in textboxes
  - [ ] Replace unsafe `strcpy` with `strncpy` in `ui_click_textbox` and `ui_switch_mode_and_swap`
  - [ ] Add bounds checking (or resizable array) for `ui_controls` in `ui_add_control`
- [ ] **CPU & Performance**:
  - [ ] Optimize the main loop to only redraw the screen when state changes or on resize
- [ ] **UTF-8 Support**:
  - [ ] Use visual character width (e.g. `wcwidth`) instead of `strlen` for border rendering and text alignment
  - [ ] Scan backward on backspace (`DEL`) to delete full UTF-8 multi-byte characters
- [ ] **Keyboard Navigation**:
  - [ ] Enable `keypad(stdscr, TRUE)` in `render_init`
  - [ ] Support Arrow keys, `Tab`, and `Shift+Tab` for navigation

