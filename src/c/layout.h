#pragma once
#include <pebble.h>

typedef struct {
  GRect rect;
  int padding;
} LayoutBox;

#define LayoutBox(rect, padding) ((LayoutBox){(rect), (padding)})

// Configuration for one entry in a series of entries within a parent LayoutBox. Can either represent a row in a
// vertical layout, or a column in a horizonatl layout.
typedef struct {
  // Represents the size of the element: In case of a row, the row height. In case of a column, the column width.
  // If set to 0: an element with dynamic size. The size is determined automatically to fill the available space in the
  // enclosing parent.
  // If set to >0: an element with a fixed size in pixels.
  int fixed_size;
} EntryConfig;

GRect get_effective_rect(LayoutBox layout_box);
void split_into_rows(LayoutBox parent, EntryConfig *entry_configs, GRect *result, int count);
void split_into_columns(LayoutBox parent, EntryConfig *entry_configs, GRect *result, int count);
