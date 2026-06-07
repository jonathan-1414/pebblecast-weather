#include "layout.h"

GRect get_effective_rect(LayoutBox layout_box) {
  GRect rect = layout_box.rect;
  int padding = layout_box.padding;
  return GRect(rect.origin.x + padding, rect.origin.y + padding, rect.size.w - 2 * padding, rect.size.h - 2 * padding);
}

int calculate_dynamic_height_per_element(LayoutBox parent, EntryConfig *rows, int count) {
  int total_height = parent.rect.size.h - parent.padding * 2;
  int fixed_height = 0;
  int dynamic_row_count = 0;
  for (int i = 0; i < count; i++) {
    if (rows[i].fixed_size > 0) {
      fixed_height += rows[i].fixed_size;
    } else {
      dynamic_row_count++;
    }
  }

  int dynamic_h = 0;
  int total_dynamic_space = total_height - fixed_height;
  if (dynamic_row_count > 0) {
    dynamic_h = total_dynamic_space / dynamic_row_count;
  }
  return dynamic_h;
}

void split_into_rows(LayoutBox parent, EntryConfig *entry_configs, GRect *result, int count) {

  int current_y = parent.rect.origin.y + parent.padding;
  int x = parent.rect.origin.x + parent.padding;
  int w = parent.rect.size.w - parent.padding * 2;

  int dynamic_h = calculate_dynamic_height_per_element(parent, entry_configs, count);

  for (int i = 0; i < count; i++) {
    int h;
    if (entry_configs[i].fixed_size > 0) {
      h = entry_configs[i].fixed_size;
    } else {
      h = dynamic_h;
    }
    result[i] = GRect(x, current_y, w, h);
    current_y += h;
  }
}

GRect swap(GRect rect) { return GRect(rect.origin.y, rect.origin.x, rect.size.h, rect.size.w); }

void split_into_columns(LayoutBox parent, EntryConfig *entry_configs, GRect *result, int count) {
  split_into_rows(LayoutBox(swap(parent.rect), parent.padding), entry_configs, result, count);
  for (int i = 0; i < count; i++) {
    result[i] = swap(result[i]);
  }
}
