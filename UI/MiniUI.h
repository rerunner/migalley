#pragma once
#include "gtk/gtk.h"
// -----------------------------------------------------------------------------
// App state mirroring FullPane.cpp semantics
// -----------------------------------------------------------------------------
struct AppState {
    int state{IDS_TITLE1};
    bool in3d{false};
    bool incomms{false};
};
extern AppState g_app; // shared app state

int UIMain(int argc, char *argv[]);
void apply_yellow_label(GtkWidget* label);
void apply_lightblue_label(GtkWidget* label);
