// mig_alley_panels_gtk2.cpp
// Build:
//   g++ -std=c++17 mig_alley_panels_gtk2.cpp -o mig_alley_panels_gtk2 `pkg-config --cflags --libs gtk+-2.0`

#include <gtk/gtk.h>
#include <fontconfig/fontconfig.h>
#include <string>
#include <vector>
#include <array>
#include <stack>
#include <functional>
#include <cstdio>
#include <regex>
#include "DOSDEFS.H" //RERUN
#include "WINMOVE.H" // RERUN

#include "MigAlley.h"
#define QUICKQUICKQUICK //RERUN
#include "SQUICK1.H" //RERUN

// -----------------------------------------------------------------------------
// Helpers: style and transparent button
// -----------------------------------------------------------------------------
static void apply_yellow_label(GtkWidget* label) {
    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_family_new("Intel"));
    pango_attr_list_insert(attrs, pango_attr_size_new(14 * PANGO_SCALE)); //RERUN does this crash?
    pango_attr_list_insert(attrs, pango_attr_scale_new(PANGO_SCALE_XX_LARGE));
    pango_attr_list_insert(attrs, pango_attr_foreground_new(65535, 65535, 0));   // yellow
    pango_attr_list_insert(attrs, pango_attr_foreground_alpha_new(65535));       // opaque text
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);
}

static void apply_lightblue_label(GtkWidget* label) {
    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_scale_new(PANGO_SCALE_X_LARGE));
    // RGB values: light blue (e.g. #66CCFF)
    pango_attr_list_insert(attrs, pango_attr_foreground_new(0x6666, 0xCCCC, 0xFFFF));
    pango_attr_list_insert(attrs, pango_attr_foreground_alpha_new(65535)); // opaque text
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);
}


static GtkWidget* make_transparent_button(const char* text) {
    GtkWidget* btn = gtk_button_new_with_label(text);
    gtk_widget_set_size_request(btn, 240, 40);
    gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);
    gtk_widget_set_name(btn, "transparent_button");

    GtkWidget* child = gtk_bin_get_child(GTK_BIN(btn));
    if (GTK_IS_LABEL(child)) {
        apply_yellow_label(child);
    }

    gtk_widget_ensure_style(btn);

    GtkRcStyle* rcstyle = gtk_rc_style_new();
    rcstyle->bg[GTK_STATE_NORMAL]   = (GdkColor){0, 0, 0, 0};
    rcstyle->bg[GTK_STATE_PRELIGHT] = (GdkColor){0, 22000, 28000, 12000};
    rcstyle->color_flags[GTK_STATE_NORMAL]   = GTK_RC_BG;
    rcstyle->color_flags[GTK_STATE_PRELIGHT] = GTK_RC_BG;
    gtk_widget_modify_style(btn, rcstyle);
    g_object_unref(rcstyle);

    return btn;
}

// ------------------------------------------------------------------
// Transparent container – works with GTK2 + any C++ compiler
// ------------------------------------------------------------------
static gboolean transparent_expose_handler(GtkWidget *widget,
                                           GdkEventExpose *event,
                                           gpointer data)
{
    (void)event; (void)data;

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.8);   // 80% black
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    cairo_destroy(cr);
    return FALSE;   // let children (labels, combo) draw on top
}

static GtkWidget* make_transparent_container()
{
    GtkWidget* eventbox = gtk_event_box_new();

    gtk_widget_set_app_paintable(eventbox, TRUE);

    GdkScreen *screen = gtk_widget_get_screen(eventbox);
    if (gdk_screen_is_composited(screen))
    {
        GdkColormap *rgba = gdk_screen_get_rgba_colormap(screen);
        if (rgba)
            gtk_widget_set_colormap(eventbox, rgba);
    }

    g_signal_connect(G_OBJECT(eventbox), "expose-event",
                     G_CALLBACK(transparent_expose_handler), NULL);

    return eventbox;
}

// Helper: Open ROOTS.DIR for path name
const char* read_root_path(void) {
    static char buffer[1024];  // static so it persists after function returns
    FILE *fp = fopen("ROOTS.DIR", "r");
    if (!fp) {
        perror("Failed to open ROOTS.DIR");
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '0') {
            char *start = strchr(line, '"');
            char *end   = start ? strchr(start + 1, '"') : NULL;
            if (start && end) {
                size_t len = end - start - 1;
                if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
                strncpy(buffer, start + 1, len);
                buffer[len] = '\0';
                fclose(fp);
                return buffer; // safe as long as caller treats it as const
            }
        }
    }

    fclose(fp);
    return NULL;
}

// Concatenate filename to the root path
// Returns a static buffer with "path/filename"
const char* build_full_path(const char* filename) {
    static char fullpath[1024];
    const char* root = read_root_path();
    if (!root) return NULL;

    // Copy root path
    strncpy(fullpath, root, sizeof(fullpath) - 1);
    fullpath[sizeof(fullpath) - 1] = '\0';

    // Add a slash if not already present
    size_t len = strlen(fullpath);
    if (len > 0 && fullpath[len - 1] != '/')
        strncat(fullpath, "/", sizeof(fullpath) - strlen(fullpath) - 1);

    // Append filename
    strncat(fullpath, filename, sizeof(fullpath) - strlen(fullpath) - 1);

    return fullpath; // static buffer, safe as const char*
}

// -----------------------------------------------------------------------------
// App state mirroring FullPane.cpp semantics
// -----------------------------------------------------------------------------

struct AppState {
    int state{IDS_TITLE1};
    bool in3d{false};
    bool incomms{false};
};

static AppState g_app; // shared app state

// Forward decls
class Screen;
using EntryCallback = std::function<bool(Screen*)>;

// Utility: placeholder widget for a dial
static GtkWidget* make_placeholder(int idstextnumber) {
    GtkWidget* transparent = make_transparent_container();

    GtkWidget* align = gtk_alignment_new(0.5, 0.5, 0, 0);
    CString ctext = LoadResString(idstextnumber);
    std::string text = ctext;

    // Replace *all* escaped "\n" sequences with real newlines
    text = std::regex_replace(text, std::regex("\\\\n"), "\n");

    GtkWidget* lab = gtk_label_new(nullptr);

    gtk_label_set_use_markup(GTK_LABEL(lab), TRUE);
    gtk_label_set_markup(GTK_LABEL(lab), text.c_str());

    // Enable word wrapping
    gtk_label_set_line_wrap(GTK_LABEL(lab), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(lab), PANGO_WRAP_WORD);
    gtk_label_set_width_chars(GTK_LABEL(lab), 60);   // wrap after ~60 chars

    gtk_container_add(GTK_CONTAINER(align), lab);
    gtk_container_add(GTK_CONTAINER(transparent), align);
    return transparent;
}

// -----------------------------------------------------------------------------
// ListPanel: parity with CRListBox-like text lists
// -----------------------------------------------------------------------------
class ListPanel {
public:
    using ActivateCallback = std::function<void(int)>;

    explicit ListPanel(bool horizontal)
        : horizontal_(horizontal) {
        widget_ = horizontal_ ? gtk_hbox_new(FALSE, 8) : gtk_vbox_new(FALSE, 0);
        container_ = widget_;
    }

    ~ListPanel() {
        clear();
        // container_ and widget_ are owned by GTK container hierarchy
    }

    GtkWidget* root() const { return widget_; }

    void addItem(const char* label) {
        GtkWidget* btn = make_transparent_button(label);
        gtk_box_pack_start(GTK_BOX(container_), btn, FALSE, FALSE, 0); // no extra padding
        buttons_.push_back(btn);
        g_signal_connect(btn, "clicked", G_CALLBACK(&ListPanel::onButtonThunk), this);
    }


    void clear() {
        for (auto* w : buttons_) {
            if (GTK_IS_WIDGET(w)) {
                gtk_container_remove(GTK_CONTAINER(container_), w);
            }
        }
        buttons_.clear();
        highlight_index_ = -1;
    }

    void setHighlight(int index) {
        highlight_index_ = index;
        int i = 0;
        for (auto* btn : buttons_) {
            if (i == index) {
                gtk_widget_set_state(btn, GTK_STATE_SELECTED);
            } else {
                gtk_widget_set_state(btn, GTK_STATE_NORMAL);
            }
            ++i;
        }
    }

    void setOnActivate(ActivateCallback cb) { on_activate_ = std::move(cb); }

    bool isHorizontal() const { return horizontal_; }

private:
    static void onButtonThunk(GtkWidget* btn, gpointer user) {
        auto* self = static_cast<ListPanel*>(user);
        self->onButton(btn);
    }

    void onButton(GtkWidget* btn) {
        int index = 0;
        for (auto* b : buttons_) {
            if (b == btn) break;
            ++index;
        }
        setHighlight(index);
        if (on_activate_) on_activate_(index);
    }

    GtkWidget* widget_{nullptr};
    GtkWidget* container_{nullptr};
    std::vector<GtkWidget*> buttons_;
    int highlight_index_{-1};
    bool horizontal_{true};
    ActivateCallback on_activate_;
};

// -----------------------------------------------------------------------------
// Dial panes: up to 3 panels, parity with pdial[0..2]
// -----------------------------------------------------------------------------
class DialPanes {
public:
    DialPanes() {
        notebook_ = gtk_notebook_new();
        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook_), GTK_POS_TOP);
        //gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook_), FALSE); // don't show tabs
        //gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook_), FALSE); // don't show border
    }

    ~DialPanes() {
        clear();
        // notebook_ owned by outer container
    }

    GtkWidget* notebook() const { return notebook_; }

    void addPage(const char* title, GtkWidget* content) {
        if (count_ >= 3) return;
        // Append content directly, no frame, no tab label
        //gtk_notebook_append_page(GTK_NOTEBOOK(notebook_), content, nullptr);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook_), content,
                         gtk_label_new("Select Mission"));
        pages_[count_++] = content;
        }

    void clear() {
        for (int i = 0; i < count_; ++i) {
            gtk_widget_destroy(pages_[i]);
            pages_[i] = nullptr;
        }
        count_ = 0;
    }

private:
    GtkWidget* notebook_{nullptr};
    std::array<GtkWidget*, 3> pages_{nullptr, nullptr, nullptr};
    int count_{0};
};

// -----------------------------------------------------------------------------
// Screen model: parity with FullScreen textlists + dials
// -----------------------------------------------------------------------------
struct Entry {
    const char* label{nullptr};
    Screen* next{nullptr};
    EntryCallback onselect{};
};

class Screen {
public:
    using DialFactory = std::function<GtkWidget*()>;

    Screen(int ids_title, bool list_horizontal)
    {
        CString winTitle =  nullptr;//(ids_title);
        title_ = winTitle.c_str();
        list_horizontal_ = list_horizontal;
    }

    const char* title() const { return title_.c_str(); }
    bool listHorizontal() const { return list_horizontal_; }

    void addEntry(const char* label, Screen* next, EntryCallback cb) {
        entries_.push_back(Entry{label, next, std::move(cb)});
    }

    const std::vector<Entry>& entries() const { return entries_; }

    void setDialFactory(int idx, DialFactory f) {
        if (idx >= 0 && idx < 3) dial_factory_[idx] = std::move(f);
    }

    void setBackground(const char* file) { backgroundFile_ = file; }
    const char* backgroundFile() const { return backgroundFile_; }

    const std::array<DialFactory, 3>& dialFactories() const { return dial_factory_; }

private:
    std::string title_;
    bool list_horizontal_;
    std::vector<Entry> entries_;
    std::array<DialFactory, 3> dial_factory_{};
    const char* backgroundFile_{nullptr};
};

// -----------------------------------------------------------------------------
// Callback stubs mapped to FullPane.cpp semantics
// -----------------------------------------------------------------------------
static bool cb_preferences_continue(Screen* /*next*/) {
    g_print("[PREFS] Continue\n");
    return true;
}

static bool cb_setup_hotshot(Screen* next) {
    g_app.state = IDS_SINGLEPLAYER1;
    g_app.in3d = true;
    miginit();
    migselect(0);
    g_print("[SP] Hot Shot Setup & Fly\n");
    migfly();
    return true;
}
static bool cb_set_quick_state(Screen* /*next*/) {
    g_app.state = IDS_SINGLEPLAYER2;
    g_print("[SP] Quick Mission\n");
    return true;
}
static bool cb_campaign_select(Screen* /*next*/) {
    g_app.state = IDS_SINGLEPLAYER3;
    miginit();
    migselect(2);
    g_print("[SP] Campaign Select\n");
    return true;
}
static bool cb_set_up_full_war(Screen* /*next*/) {
    g_app.state = IDS_SINGLEPLAYER4;
    miginit();
    migselect(3);
    g_print("[SP] Entire War\n");
    migfly(); // TEST ONLY, remove this when full war setup is implemented
    return true;
}
static bool cb_quickmission_fly(Screen* /*next*/) {
    g_app.in3d = true;
    miginit();
    migselect(1); // Ensure quick mission state is set after selecting mission
    migfly();
    g_print("[QM] Fly\n");
    return true;
}
static bool cb_quickmission_debrief(Screen* /*next*/) {
    g_app.in3d = false;
    g_print("[QM] Debrief\n");
    return true;
}
static bool cb_campaign_begin(Screen* /*next*/) {
    g_print("[CAMP] Begin\n");
    return true;
}
static bool cb_confirm_exit(Screen* /*next*/) {
    g_print("[EXIT] Quit\n");
    gtk_main_quit();
    return false;
}

// -----------------------------------------------------------------------------
// Controller: owns the main window layout
// -----------------------------------------------------------------------------
class Controller {
public:
    Controller() {
        // Window
        win_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        // 2. ONE fixed container that covers the whole window
        GtkWidget *fixed = gtk_fixed_new();
        gtk_container_add(GTK_CONTAINER(win_), fixed);

        // 3. Background image – add it FIRST
        bg_ = gtk_image_new();
        gtk_fixed_put(GTK_FIXED(fixed), bg_, 0, 0);

        gtk_window_set_title(GTK_WINDOW(win_), "MiG Alley");
        gtk_window_set_default_size(GTK_WINDOW(win_), 1024, 768);
        gtk_window_set_resizable(GTK_WINDOW(win_), FALSE);
        g_signal_connect(win_, "destroy", G_CALLBACK(+gtk_main_quit), nullptr);

        // Root container: fixed for background + overlay layout
        //GtkWidget* fixed = gtk_fixed_new();
        //gtk_container_add(GTK_CONTAINER(win_), fixed);

        // Background image
        std::string bg_path = build_full_path("artwork/DIAL1024/title.bmp");
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(bg_path.c_str(), NULL);
        bg_ = gtk_image_new_from_pixbuf(pixbuf);
        gtk_fixed_put(GTK_FIXED(fixed), bg_, 0, 0);


        // Overlay stack (title + list + dials); spans full window
        vbox_ = gtk_vbox_new(FALSE, 4);
        gtk_fixed_put(GTK_FIXED(fixed), vbox_, 0, 0);

        // Right-side, vertically centered menu container
        // Alignment container: fills window, right aligned, vertically centered
        GtkWidget* align = gtk_alignment_new(0.85, 0.5, 0.0, 0.0);
        gtk_alignment_set_padding(GTK_ALIGNMENT(align),
                                  70,   // top padding
                                  0,   // bottom padding
                                  0,   // left padding
                                  0); // right padding in pixels
        gtk_fixed_put(GTK_FIXED(fixed), align, 0, 0);
        gtk_widget_set_size_request(align, 1024, 768); // match window size

        // Vertical list panel inside alignment
        list_ = new ListPanel(false);
        gtk_container_add(GTK_CONTAINER(align), list_->root());

        // Dial panes below (optional, stays visible on non-title screens)
        dials_ = new DialPanes();
        gtk_box_pack_start(GTK_BOX(vbox_), dials_->notebook(), TRUE, TRUE, 6);

        // Hook selection handler
        list_->setOnActivate([this](int idx) { onSelect(idx); });
    }

    ~Controller() {
        delete list_;
        delete dials_;
    }

    void renderScreen(Screen* scr, bool pushCurrent = true) {
        const char* bgfile = scr && scr->backgroundFile() ? scr->backgroundFile()
        : build_full_path("artwork/DIAL1024/title.bmp");

        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(bgfile, NULL);
        if (pixbuf) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(bg_), pixbuf);
            g_object_unref(pixbuf);
        }

        if (pushCurrent && current_) {
            nav_stack_.push(current_);
        }
        current_ = scr;

        // Rebuild list items
        if (!list_) return; // defensive guard
        list_->clear();

        // Recreate orientation only if needed (we keep vertical for title and others)
        if (list_->isHorizontal() != scr->listHorizontal()) {
            // Replace the widget inside the alignment container
            GtkWidget* parent = gtk_widget_get_parent(list_->root());
            if (parent) {
                gtk_container_remove(GTK_CONTAINER(parent), list_->root());
            }
            delete list_;
            list_ = new ListPanel(scr->listHorizontal());
            list_->setOnActivate([this](int idx) { onSelect(idx); });
            if (parent) {
                gtk_container_add(GTK_CONTAINER(parent), list_->root());
            }
            gtk_widget_show(list_->root());
        }

        for (const auto& e : scr->entries()) {
            if (!e.label) break;
            list_->addItem(e.label);
        }
        list_->setHighlight(0);

        // Dial panes
        dials_->clear();
        const auto& dfs = scr->dialFactories();
        for (int i = 0; i < 3; ++i) {
            if (dfs[i]) {
                GtkWidget* content = dfs[i]();
                const char* title = (i == 0 ? "Panel 1" : i == 1 ? "Panel 2" : "Panel 3");
                dials_->addPage(title, content);
            }
        }

        gtk_widget_show_all(win_);
    }

    void show() { gtk_widget_show_all(win_); }

    void setScreens(Screen* title, Screen* single, Screen* prefs,
                    Screen* quick, Screen* camp) {
        title_ = title;
        singlePlayer_ = single;
        prefs_ = prefs;
        quickMission_ = quick;
        campaignSelect_ = camp;
    }

private:
    void onSelect(int index) {
        if (!current_) return;
        if (index < 0) return;
        const auto& entries = current_->entries();
        if (index >= static_cast<int>(entries.size())) return;

        const Entry& e = entries[index];
        if (!e.label) return;

        Screen* next = e.next;
        bool ok = true;
        if (e.onselect) ok = e.onselect(next);
        if (!ok) return;

        if (std::string(e.label) == "Back") {
            if (!nav_stack_.empty()) {
                Screen* prev = nav_stack_.top();
                nav_stack_.pop();
                renderScreen(prev, false);
            }
        } else if (next) {
            renderScreen(next);
        }
    }

    GtkWidget* win_{nullptr};
    GtkWidget* vbox_{nullptr};
    GtkWidget* title_label_{nullptr};
    ListPanel* list_{nullptr};
    DialPanes* dials_{nullptr};

    GtkWidget* bg_{nullptr};

    Screen* current_{nullptr};
    Screen* title_{nullptr};
    Screen* singlePlayer_{nullptr};
    Screen* prefs_{nullptr};
    Screen* quickMission_{nullptr};
    Screen* campaignSelect_{nullptr};

    std::stack<Screen*> nav_stack_;
};

static GtkWidget* make_quickmission_selector() {
    GtkWidget* transparent = make_transparent_container();

    // Vertical box to hold combo + description
    GtkWidget* vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_add(GTK_CONTAINER(transparent), vbox);

    // Combo box with mission names
    GtkWidget* combo = gtk_combo_box_new_text();

    // Iterate until sentinel {0}
    int missionCount = 0;
    for (int i = 0; CSQuick1::quickmissions[i].missionname != 0; ++i) {
        CString cname = LoadResString(CSQuick1::quickmissions[i].missionname);
        std::string name = cname;
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), name.c_str());
        missionCount++;
    }
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);

    // Label for mission description
    GtkWidget* desc = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(desc), TRUE);
    gtk_label_set_width_chars(GTK_LABEL(desc), 60);
    apply_lightblue_label(desc);   // style text
    gtk_box_pack_start(GTK_BOX(vbox), desc, TRUE, TRUE, 0);

    // Transparent background for vbox
    GtkRcStyle* rcstyle = gtk_rc_style_new();
    rcstyle->bg[GTK_STATE_NORMAL] = (GdkColor){0,0,0,0};
    rcstyle->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG;
    gtk_widget_modify_style(vbox, rcstyle);
    g_object_unref(rcstyle);

    // Callback: update description when selection changes
    g_signal_connect(combo, "changed", G_CALLBACK(+[] (GtkComboBox* cb, gpointer user) {
        GtkWidget* desc = GTK_WIDGET(user);
        int idx = gtk_combo_box_get_active(cb);
        if (idx >= 0 && CSQuick1::quickmissions[idx].missionname != 0) {
            CSQuick1::currquickmiss = idx; // Update current mission index, used by SelectQuickMission()
            CString ctext = LoadResString(CSQuick1::quickmissions[idx].missiondesc);
            std::string text = ctext;
            gtk_label_set_text(GTK_LABEL(desc), text.c_str());
        }
    }), desc);

    // Default selection = mission 0
    if (missionCount > 16) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        CSQuick1::currquickmiss = 0;
        CString ctext = LoadResString(CSQuick1::quickmissions[9].missiondesc);
        std::string text = ctext;
        gtk_label_set_text(GTK_LABEL(desc), text.c_str());
    }

    return transparent;
}

// -----------------------------------------------------------------------------
// Screens definition
// -----------------------------------------------------------------------------
int UIMain(int argc, char** argv) {
    FcInit(); // initialize Fontconfig

    gtk_init(&argc, &argv);

    // Create screens
    Screen prefs(IDS_TITLE1, true);
    prefs.addEntry("3D Display", nullptr, {});
    prefs.addEntry("Continue",   nullptr, cb_preferences_continue);
    prefs.setDialFactory(0, [](){ return make_placeholder(IDS_PREFERENCES5_SELECT); });

    // Quick Mission screen
    Screen quick(IDS_SINGLEPLAYER2, false);
    std::string quick_bg = build_full_path("artwork/DIAL1024/quickmis.bmp");
    quick.setBackground(quick_bg.c_str());
    quick.addEntry("Back",    nullptr, {});
    quick.addEntry("Fly",     nullptr, cb_quickmission_fly);
    quick.addEntry("Debrief", nullptr, cb_quickmission_debrief);
    quick.setDialFactory(0, [](){ return make_quickmission_selector(); });

    Screen camp(IDS_SINGLEPLAYER3, true);
    std::string camp_bg = build_full_path("artwork/DIAL1024/campaign.bmp");
    camp.setBackground(camp_bg.c_str());
    camp.addEntry("Back",  nullptr, {});
    camp.addEntry("Begin", nullptr, cb_campaign_begin);
    camp.setDialFactory(0, [](){ return make_placeholder(IDS_L_SCAMPAIGNSELECT1); });

    Screen single(IDS_TITLE2, false);
    single.addEntry("Hot Shot",      nullptr, cb_setup_hotshot);
    single.addEntry("Quick Mission", &quick,  cb_set_quick_state);
    single.addEntry("Campaign",      &camp,   cb_campaign_select);
    single.addEntry("Entire War",    nullptr, cb_set_up_full_war);
    single.addEntry("Back",          nullptr, {});
    // no dials for Single Player

    Screen title(IDS_MIGALLEY, false);
    std::string title_bg = build_full_path("artwork/DIAL1024/title.bmp");
    title.setBackground(title_bg.c_str());
    title.addEntry("Preferences",   &prefs,  {});
    title.addEntry("Single Player", &single, {});
    title.addEntry("Multi-Player",  nullptr, {});
    title.addEntry("Load Game",     nullptr, {});
    title.addEntry("Replay",        nullptr, {});
    title.addEntry("Credits",       nullptr, {});
    title.addEntry("Quit",          nullptr, cb_confirm_exit);

    // Controller
    Controller ctl;
    ctl.setScreens(&title, &single, &prefs, &quick, &camp);
    ctl.renderScreen(&title);

    ctl.show();
    gtk_main();
    return 0;
}

