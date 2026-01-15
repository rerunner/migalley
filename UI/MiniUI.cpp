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

#include "GeneratedResources.h" //RERUN
extern std::unordered_map<UniqueID, const char*> UIDName;

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

static void make_widget_transparent(GtkWidget* w)
{
    gtk_widget_set_app_paintable(w, TRUE);

    GdkScreen* screen = gtk_widget_get_screen(w);
    if (gdk_screen_is_composited(screen)) {
        GdkColormap* rgba = gdk_screen_get_rgba_colormap(screen);
        if (rgba)
            gtk_widget_set_colormap(w, rgba);
    }

    g_signal_connect(G_OBJECT(w), "expose-event",
                     G_CALLBACK(transparent_expose_handler), NULL);
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
// DialPanes WITHOUT GtkNotebook (transparent, stack-like)
// -----------------------------------------------------------------------------
class DialPanes {
public:
    DialPanes() {
        // A simple vbox that will hold up to 3 panels stacked vertically.
        // Only one will be shown at a time.
        container_ = gtk_vbox_new(FALSE, 0);
        gtk_widget_set_app_paintable(container_, TRUE);
    }

    ~DialPanes() {
        clear();
    }

    GtkWidget* notebook() const { return container_; }

    void addPage(const char* /*title*/, GtkWidget* content) {
        if (count_ >= 3) return;

        // Make panel transparent
        make_widget_transparent(content);

        // Add to container but hide initially
        gtk_box_pack_start(GTK_BOX(container_), content, TRUE, TRUE, 0);
        gtk_widget_hide(content);

        pages_[count_++] = content;

        // First added page becomes visible
        if (count_ == 1)
            gtk_widget_show(content);
    }

    void clear() {
        for (int i = 0; i < count_; ++i) {
            if (pages_[i]) {
                gtk_widget_destroy(pages_[i]);
                pages_[i] = nullptr;
            }
        }
        count_ = 0;
    }

    // Optional: switch visible panel (if you ever need it)
    void showPage(int index) {
        for (int i = 0; i < count_; ++i) {
            if (pages_[i])
                gtk_widget_set_visible(pages_[i], i == index);
        }
    }

private:
    GtkWidget* container_{nullptr};
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
static bool cb_quickmission_variants(Screen* /*next*/) {
    g_app.in3d = false;
    g_print("[QM] Variants\n");
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


static void rebuild_flight_names(GtkComboBox* flight_name_combo)
{
    QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
    int side = qd.plside;   // 0 = UN, 1 = Red

    // Clear existing entries
    int count = gtk_tree_model_iter_n_children(
        gtk_combo_box_get_model(flight_name_combo), nullptr);
    for (int i = count - 1; i >= 0; --i)
        gtk_combo_box_remove_text(flight_name_combo, i);

    // Rebuild list
    int index = 0;
    for (int w = 0; w < 8; ++w) {
        for (int g = 0; g < 3; ++g) {
            QuickFields& qf = qd.line[side][w][g];
            if (qf.flights || qf.DutyFlags()) {
                CString cname = (side == 0 ? "UN " : "Red ") + LoadResString(qf.descID) + " Flight " + std::to_string(index + 1);
                gtk_combo_box_append_text(flight_name_combo, cname);
                index++;
            }
        }
    }

    // Auto-select first entry if available
    if (index > 0) {
        gtk_combo_box_set_active(flight_name_combo, 0);

        // Update player aircraft to match first entry
        for (int w = 0; w < 8; ++w) {
            for (int g = 0; g < 3; ++g) {
                QuickFields& qf = qd.line[side][w][g];
                if (qf.flights || qf.DutyFlags()) {
                    qd.plac = qf.actype;
                    return;
                }
            }
        }
    }
}

static void on_rb_un_toggled(GtkToggleButton* btn, gpointer user)
{
    if (gtk_toggle_button_get_active(btn)) {
        QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
        qd.plside = 0;

        GtkComboBox* flight_name_combo = GTK_COMBO_BOX(user);
        rebuild_flight_names(flight_name_combo);
    }
}

static void on_rb_red_toggled(GtkToggleButton* btn, gpointer user)
{
    if (gtk_toggle_button_get_active(btn)) {
        QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
        qd.plside = 1;

        GtkComboBox* flight_name_combo = GTK_COMBO_BOX(user);
        rebuild_flight_names(flight_name_combo);
    }
}



static GtkWidget* make_quickmission_selector() {
    // Vertical box to hold everything (no black overlay)
    GtkWidget* panel = make_transparent_container();   // draws your semi‑transparent black
    GtkWidget* vbox  = gtk_vbox_new(FALSE, 8);

    gtk_container_add(GTK_CONTAINER(panel), vbox);


    // -------------------------------------------------
    // 1) Mission label + combo + description
    // -------------------------------------------------

    // Horizontal box for label + combo
    GtkWidget* mission_hbox = gtk_hbox_new(FALSE, 12);   // spacing = 12px
    gtk_box_pack_start(GTK_BOX(vbox), mission_hbox, FALSE, FALSE, 0);

    // Label on the left
    GtkWidget* mission_label = gtk_label_new("Mission");
    gtk_misc_set_alignment(GTK_MISC(mission_label), 1.0, 0.5); // right-align text
    apply_lightblue_label(mission_label);
    gtk_box_pack_start(GTK_BOX(mission_hbox), mission_label, FALSE, FALSE, 0);

    // Combo box on the right
    GtkWidget* mission_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(mission_combo, 260, -1); // widen if needed
    gtk_box_pack_start(GTK_BOX(mission_hbox), mission_combo, FALSE, FALSE, 0);

    // Fill mission combo
    int missionCount = 0;
    for (int i = 0; CSQuick1::quickmissions[i].missionname != 0; ++i) {
        CString cname = LoadResString(CSQuick1::quickmissions[i].missionname);
        gtk_combo_box_append_text(GTK_COMBO_BOX(mission_combo), cname);
        missionCount++;
    }

    // Mission description below
    GtkWidget* mission_desc = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(mission_desc), TRUE);
    gtk_label_set_width_chars(GTK_LABEL(mission_desc), 60);
    apply_lightblue_label(mission_desc);

    // -------------------------------------------------
    // 2) Flight Name + Flight Lead (side-by-side)
    // -------------------------------------------------
    GtkWidget* flight_row_hbox = gtk_hbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(vbox), flight_row_hbox, FALSE, FALSE, 0);

    // --- Flight Name ---
    GtkWidget* flight_name_label = gtk_label_new("   Flight     ");
    gtk_misc_set_alignment(GTK_MISC(flight_name_label), 1.0, 0.5);
    apply_lightblue_label(flight_name_label);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_name_label, FALSE, FALSE, 0);

    GtkWidget* flight_name_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(flight_name_combo, 180, -1);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_name_combo, FALSE, FALSE, 0);

    // Spacer between the two combos (optional)
    GtkWidget* spacer = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), spacer, FALSE, FALSE, 0);

    // --- Flight Lead ---
    GtkWidget* flight_lead_label = gtk_label_new(""); // Lead
    gtk_misc_set_alignment(GTK_MISC(flight_lead_label), 1.0, 0.5);
    apply_lightblue_label(flight_lead_label);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_lead_label, FALSE, FALSE, 0);

    GtkWidget* flight_lead_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(flight_lead_combo, 180, -1);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_lead_combo, FALSE, FALSE, 0);

    // -------------------------------------------------
    // 3) Target Zone label + Target Type combo
    // -------------------------------------------------

    // Horizontal box for label + combo
    GtkWidget* target_type_hbox = gtk_hbox_new(FALSE, 12);   // spacing = 12px
    gtk_box_pack_start(GTK_BOX(vbox), target_type_hbox, FALSE, FALSE, 0);

    // Label on the left
    GtkWidget* target_type_label = gtk_label_new("     Target Zone");
    gtk_misc_set_alignment(GTK_MISC(target_type_label), 1.0, 0.5); // right-align text
    apply_lightblue_label(target_type_label);
    gtk_box_pack_start(GTK_BOX(target_type_hbox), target_type_label, FALSE, FALSE, 0);

    // Combo box on the right
    GtkWidget* target_type_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(target_type_combo, 260, -1); // widen if needed
    gtk_box_pack_start(GTK_BOX(target_type_hbox), target_type_combo, FALSE, FALSE, 0);

    // -------------------------------------------------
    // 4) Target Name label + combo (aligned with Target Zone)
    // -------------------------------------------------
    // Horizontal box for label + combo
    GtkWidget* target_name_hbox = gtk_hbox_new(FALSE, 12);   // same spacing as Target Zone
    gtk_box_pack_start(GTK_BOX(vbox), target_name_hbox, FALSE, FALSE, 0);

    // Empty label to align with "Target Zone"
    GtkWidget* target_name_label = gtk_label_new("");
    gtk_widget_set_size_request(target_name_label, 160, -1);   // adjust width as needed
    gtk_misc_set_alignment(GTK_MISC(target_name_label), 1.0, 0.5); // right-align text
    apply_lightblue_label(target_name_label);
    gtk_box_pack_start(GTK_BOX(target_name_hbox), target_name_label, FALSE, FALSE, 0);

    // Combo box on the right
    GtkWidget* target_name_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(target_name_combo, 260, -1); // same width as Target Zone
    gtk_box_pack_start(GTK_BOX(target_name_hbox), target_name_combo, FALSE, FALSE, 0);


    // -------------------------------------------------
    // 5) Side selection radio buttons (UN / Red)
    // -------------------------------------------------

    // Horizontal box for the radio buttons
    GtkWidget* side_hbox = gtk_hbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(vbox), side_hbox, FALSE, FALSE, 0);

    // Create the first radio button ("UN")
    GtkWidget* rb_un = gtk_radio_button_new_with_label(NULL, "UN");
    apply_lightblue_label(gtk_bin_get_child(GTK_BIN(rb_un)));
    gtk_box_pack_start(GTK_BOX(side_hbox), rb_un, FALSE, FALSE, 0);

    // Create the second radio button ("Red"), in the same group
    GtkWidget* rb_red = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(rb_un), "Red");
    apply_lightblue_label(gtk_bin_get_child(GTK_BIN(rb_red)));
    gtk_box_pack_start(GTK_BOX(side_hbox), rb_red, FALSE, FALSE, 0);

    // Default selection: UN
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_un), TRUE);


    // -------------------------------------------------
    // Mission description last
    // -------------------------------------------------
    // Add some extra vertical spacing
    GtkWidget* desc_spacer = gtk_label_new("");
    gtk_widget_set_size_request(desc_spacer, -1, 80); // 80 pixels tall
    gtk_box_pack_start(GTK_BOX(vbox), desc_spacer, FALSE, FALSE, 0);

    // Now add the mission description label
    gtk_box_pack_start(GTK_BOX(vbox), mission_desc, FALSE, FALSE, 0);

    // ---------------------------------------
    // Mission combo "changed" callback
    // ---------------------------------------
    g_signal_connect(mission_combo, "changed",
        G_CALLBACK(+[] (GtkComboBox* cb, gpointer user) {
            auto** widgets = static_cast<GtkWidget**>(user);
            GtkWidget* mission_desc = widgets[0];
            GtkComboBox* target_name_combo = GTK_COMBO_BOX(widgets[1]);
            GtkComboBox* target_type_combo = GTK_COMBO_BOX(widgets[2]);
            GtkComboBox* flight_name_combo = GTK_COMBO_BOX(widgets[3]);
            GtkComboBox* flight_lead_combo = GTK_COMBO_BOX(widgets[4]);

            int idx = gtk_combo_box_get_active(cb);
            if (idx < 0 || CSQuick1::quickmissions[idx].missionname == 0)
                return;

            CSQuick1::currquickmiss = idx;

            // Update mission description
            CString ctext = LoadResString(CSQuick1::quickmissions[idx].missiondesc);
            std::string text = ctext;
            gtk_label_set_text(GTK_LABEL(mission_desc), text.c_str());

            // Refill target combo for this mission
            QuickDef& qd = CSQuick1::quickmissions[idx];

            // Clear target NAME combo
            int count = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(target_name_combo), nullptr);
            for (int i = count - 1; i >= 0; --i) {
                gtk_combo_box_remove_text(target_name_combo, i);
            }

            // Clear target TYPE combo
            int tcount = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(target_type_combo), nullptr);
            for (int i = tcount - 1; i >= 0; --i) {
                gtk_combo_box_remove_text(target_type_combo, i);
            }

            // Clear FLIGHT NAME combo
            int fnCount = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(flight_name_combo), nullptr);
            for (int i = fnCount - 1; i >= 0; --i) {
                gtk_combo_box_remove_text(flight_name_combo, i);
            }

            // Clear FLIGHT LEAD combo
            int flCount = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(flight_lead_combo), nullptr);
            for (int i = flCount - 1; i >= 0; --i) {
                gtk_combo_box_remove_text(flight_lead_combo, i);
            }

            // Fill default player ac to first entry of new mission.
            // This will be updated again when flight is selected.
            qd.plac = qd.line[0][0][0].actype;

            UniqueID firstTarget = qd.target;
            for (int t = 0; t < 4; ++t) {
                if (qd.targtypeIDs[t] == 0)
                    continue;

                // TARGET TYPE
                UniqueID uidtest = qd.targets[t][0];
                if (uidtest != 0)
                {
                    CString typeName = LoadResString(qd.targtypeIDs[t]);
                    gtk_combo_box_append_text(target_type_combo, typeName);
                }

                // TARGET NAME
                bool anyInThisType = false;
                for (int i = 0; i < 4; ++i) {
                    UniqueID uid = qd.targets[t][i];
                    if (uid == 0)
                        continue;
                    
                    // Translate UID to resource string ID
                    auto it = UIDName.find(uid);
                    std::string enumName = it->second;
                    std::string idsName = "IDS_" + enumName;
                    // Translate resource string ID to integer ID
                    int idsInt = 0;
                    auto it2 = g_resIDMap.find(idsName);
                    if (it2 != g_resIDMap.end()) {
                        idsInt = it2->second;
                    }
                    CString targetName = LoadResString(idsInt);

                    anyInThisType = true;
                    gtk_combo_box_append_text(target_name_combo, targetName);


                    if (!firstTarget)
                        firstTarget = uid;
                }

                if (!anyInThisType)
                    continue;
            }

            // -------------------------------------------------
            // Populate FLIGHT NAME and FLIGHT LEAD combos
            // -------------------------------------------------
            if (qd.plside == 0)
            {
                printf("Player side: BLUE\n");
                // BLUE
                int	ilun[8][2]={{0}};
                int initind=0;
                for (int wave=0;wave<8;wave++)
                {
                    for (int grp=0;grp<3;grp++)
                    { 
                        if (qd.line[0][wave][grp].flights||qd.line[0][wave][grp].DutyFlags())
                        {
                            printf("  Found BLUE flight at wave %d group %d\n", wave, grp);
                            ilun[initind][0]=wave;
                            ilun[initind][1]=grp;
                            initind++;
                        }
                    }
                }
        
                for (int i = 0; i < 8; ++i) {
                    if (initind>i) 
                    { 
                        printf("Adding BLUE flight name for index %d (wave %d group %d)\n", i, ilun[i][0], ilun[i][1]);
                        CString fname = LoadResString(qd.line[0][ilun[i][0]][0].descID);
                        CString finalflightname;
                        finalflightname = "UN " + fname + " Flight " + std::to_string(i + 1);
                        gtk_combo_box_append_text(flight_name_combo, finalflightname);

                        // Flight LEAD
                        int g = ilun[i][1];
                        int ids__ = 0;
                        switch (g) {
                            case 0: ids__ = IDS_L_ELTPOS_0; break; // Lead 1
                            case 1: ids__ = IDS_ELTPOS_1; break; // Wing 1
                            case 2: ids__ = IDS_ELTPOS_2; break; // Lead 2
                            case 3: ids__ = IDS_ELTPOS_3; break; // Wing 2
                        }
                        CString posName = LoadResString(ids__);
                        gtk_combo_box_append_text(flight_lead_combo, posName);
                    }
                }
            }
            else
            {
                printf("Player side: RED\n");
                // RED
                int	ilch[8][2]={{0}};
                int initindch=0;

                for (int wave=0;wave<8;wave++)
                {
                    for (int grp=0;grp<3;grp++)
                    { 
                        if (qd.line[1][wave][grp].flights||qd.line[1][wave][grp].DutyFlags())
                        {      
                            ilch[initindch][0]=wave;
                            ilch[initindch][1]=grp;
                            initindch++;
                        }
                    }
                }
        
                for (int i = 0; i < 8; ++i) {
                    if (initindch>i) 
                    { 
                        CString fname = LoadResString(qd.line[1][ilch[i][0]][0].descID);
                        CString finalflightname;
                        finalflightname = "Red " + fname +" Flight " + std::to_string(i + 1);
                        gtk_combo_box_append_text(flight_name_combo, finalflightname);

                        // Flight LEAD
                        int g = ilch[i][1];
                        int ids__ = 0;
                        switch (g) {
                            case 0: ids__ = IDS_L_ELTPOS_0; break; // Lead 1
                            case 1: ids__ = IDS_ELTPOS_1; break; // Wing 1
                            case 2: ids__ = IDS_ELTPOS_2; break; // Lead 2
                            case 3: ids__ = IDS_ELTPOS_3; break; // Wing 2
                        }
                        CString posName = LoadResString(ids__);
                        gtk_combo_box_append_text(flight_lead_combo, posName);
                    }
                }
            }

            // Default selection
            int leadCount = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(flight_lead_combo), nullptr);

            if (leadCount > 0) {
                gtk_combo_box_set_active(flight_lead_combo, 0);
            }


            // Default selections
            gtk_combo_box_set_active(flight_name_combo, 0);
            gtk_combo_box_set_active(flight_lead_combo, 0);

            // ---------------------------------------
            // Set default target TYPE selection
            // ---------------------------------------
            int typeCount = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(target_type_combo), nullptr);

            if (typeCount > 0) {
                gtk_combo_box_set_active(target_type_combo, 0);
            }

            if (firstTarget) {
                gtk_combo_box_set_active(target_name_combo, 0);
                qd.target = firstTarget;
            } else {
                // Nothing
            }
        }),
        // pass [mission_desc, target_name_combo, target_type_combo] as user data
        ([&]() {
            auto arr = new GtkWidget*[5];
            arr[0] = mission_desc;
            arr[1] = target_name_combo;
            arr[2] = target_type_combo;
            arr[3] = flight_name_combo; 
            arr[4] = flight_lead_combo;
            return arr;
        })()
    );

    // ---------------------------------------
    // Flight Name combo "changed" callback
    // ---------------------------------------
    g_signal_connect(flight_name_combo, "changed",
        G_CALLBACK(+[] (GtkComboBox* cb, gpointer user) {
            int sel = gtk_combo_box_get_active(cb);
            if (sel < 0)
                return;

            QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
            // Determine if selection is BLUE or RED flight
            printf("Flight selection index: %d\n", sel);
            if (qd.plside == 0) 
            {
                // BLUE flight
                int wave = 0;
                int grp = 0;
                int count = 0;
                for (int w = 0; w < 8; ++w) {
                    for (int g = 0; g < 3; ++g) {
                        if (qd.line[0][w][g].flights || qd.line[0][w][g].DutyFlags()) {
                            if (count == sel) {
                                printf("Found BLUE flight for selection %d at wave %d group %d\n", sel, w, g);
                                wave = w;
                                grp = g;
                                break;
                            }
                            count++;
                        }
                    }
                }
                qd.plac = qd.line[0][wave][grp].actype;
                qd.plwave = wave;
                qd.plgrp = grp;
                printf("Selected BLUE flight at wave %d group %d, setting player ac to %d\n",
                       wave, grp, qd.plac);
            } 
            else 
            {
                // RED flight
                int wave = 0;
                int grp = 0;
                int count = 0;
                for (int w = 0; w < 8; ++w) {
                    for (int g = 0; g < 3; ++g) {
                        if (qd.line[1][w][g].flights || qd.line[1][w][g].DutyFlags()) {
                            if (count == sel) {
                                printf("Found RED flight for selection %d at wave %d group %d\n", sel, w, g);
                                wave = w;
                                grp = g;
                                break;
                            }
                            count++;
                        }
                    }
                }
                qd.plac = qd.line[1][wave][grp].actype;
                qd.plwave = wave;
                qd.plgrp = grp;
                printf("Selected RED flight at wave %d group %d, setting player ac to %d\n",
                       wave, grp, qd.plac);
            }
        }),
        NULL);

    // ---------------------------------------
    // Radio buttons "changed" callback
    // ---------------------------------------
    g_signal_connect(rb_un,  "toggled",
        G_CALLBACK(on_rb_un_toggled),  flight_name_combo);

    g_signal_connect(rb_red, "toggled",
        G_CALLBACK(on_rb_red_toggled), flight_name_combo);


    // ---------------------------------------
    // Target Type combo "changed" callback
    // ---------------------------------------
    g_signal_connect(target_type_combo, "changed",
        G_CALLBACK(+[] (GtkComboBox* cb, gpointer user) {

            // user = target_name_combo
            GtkComboBox* target_name_combo = GTK_COMBO_BOX(user);

            int sel = gtk_combo_box_get_active(cb);
            if (sel < 0)
                return;

            QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];

            // Determine which target type index corresponds to this selection
            int flatIndex = 0;
            int chosenTypeIndex = -1;

            for (int t = 0; t < 4; ++t) {
                if (qd.targtypeIDs[t] == 0)
                    continue;

                if (flatIndex == sel) {
                    chosenTypeIndex = t;
                    break;
                }
                ++flatIndex;
            }

            if (chosenTypeIndex < 0)
                return;

            // Clear target NAME combo
            int count = gtk_tree_model_iter_n_children(
                gtk_combo_box_get_model(target_name_combo), nullptr);
            for (int i = count - 1; i >= 0; --i) {
                gtk_combo_box_remove_text(target_name_combo, i);
            }

            // Refill only the names belonging to this type
            UniqueID firstTarget = (UniqueID)0;

            for (int i = 0; i < 4; ++i) {
                UniqueID uid = qd.targets[chosenTypeIndex][i];
                if (uid == 0)
                    continue;

                // UID → enum name → IDS_ → resource string
                auto it = UIDName.find(uid);
                if (it == UIDName.end())
                    continue;

                std::string enumName = it->second;
                std::string idsName = "IDS_" + enumName;

                int idsInt = 0;
                auto it2 = g_resIDMap.find(idsName);
                if (it2 != g_resIDMap.end())
                    idsInt = it2->second;

                CString targetName = LoadResString(idsInt);
                gtk_combo_box_append_text(target_name_combo, targetName);

                if (!firstTarget)
                    firstTarget = uid;
            }

            // Select first target name
            if (firstTarget) {
                gtk_combo_box_set_active(target_name_combo, 0);
                qd.target = firstTarget;
            }

        }),
        target_name_combo   // <-- IMPORTANT: pass target_name_combo as user data
    );


    // ---------------------------------------
    // Target Name combo "changed" callback
    // ---------------------------------------
    g_signal_connect(target_name_combo, "changed",
        G_CALLBACK(+[] (GtkComboBox* cb, gpointer user) {
            GtkWidget* label = GTK_WIDGET(user);
            int sel = gtk_combo_box_get_active(cb);

            if (sel < 0)
                return;

            QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];

            // Walk through target names in same order we used when filling the combo
            int flatIndex = 0;
            UniqueID chosen = (UniqueID)0;
            int chosenTypeIndex = -1;

            for (int t = 0; t < 4 && !chosen; ++t) {
                if (qd.targtypeIDs[t] == 0)
                    continue;

                for (int i = 0; i < 4; ++i) {
                    UniqueID uid = qd.targets[t][i];
                    if (uid == 0)
                        continue;

                    if (flatIndex == sel) {
                        chosen = uid;
                        chosenTypeIndex = t;
                        break;
                    }
                    ++flatIndex;
                }
            }

            if (chosen) {
                qd.target = chosen;
                if (chosenTypeIndex >= 0) {
                    CString tname = LoadResString(qd.targtypeIDs[chosenTypeIndex]);
                    gtk_label_set_text(GTK_LABEL(label), tname);
                }
            }
        }),
        NULL 
    );

    // ---------------------------------------
    // Default mission selection
    // ---------------------------------------
    if (missionCount > 0) {
        int defaultIndex = 0; // or your existing logic (e.g. 9, 16, etc.)
        gtk_combo_box_set_active(GTK_COMBO_BOX(mission_combo), defaultIndex);
        CSQuick1::currquickmiss = defaultIndex;

        CString ctext = LoadResString(CSQuick1::quickmissions[defaultIndex].missiondesc);
        std::string text = ctext;
        gtk_label_set_text(GTK_LABEL(mission_desc), text.c_str());

        // Also fill target combo for this mission
        QuickDef& qd = CSQuick1::quickmissions[defaultIndex];

        UniqueID firstTarget = qd.target;
        for (int t = 0; t < 4; ++t) {
            if (qd.targtypeIDs[t] == 0)
                continue;

            CString typeName = LoadResString(qd.targtypeIDs[t]);
            bool anyInThisType = false;
            for (int i = 0; i < 4; ++i) {
                UniqueID uid = qd.targets[t][i];
                if (uid == 0)
                    continue;
                anyInThisType = true;
                gtk_combo_box_append_text(GTK_COMBO_BOX(target_name_combo), typeName);
                if (!firstTarget)
                    firstTarget = uid;
            }
            if (!anyInThisType)
                continue;
        }

        if (firstTarget) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(target_name_combo), 0);
            qd.target = firstTarget;
        }
    }

    return panel;
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
    quick.addEntry("Variants", nullptr, cb_quickmission_variants);
    quick.addEntry("Fly",     nullptr, cb_quickmission_fly);
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

