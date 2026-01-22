#pragma once

#include "MiniUI.h"

class Screen;

bool cb_set_quick_state(Screen* /*next*/);
bool cb_quickmission_fly(Screen* /*next*/);
bool cb_quickmission_variants(Screen* /*next*/);
bool cb_quickmission_debrief(Screen* /*next*/);
GtkWidget* make_quickmission_selector();
