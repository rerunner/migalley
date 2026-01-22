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
#include "QuickMissionUI.h"
extern std::unordered_map<UniqueID, const char*> UIDName;

bool cb_set_quick_state(Screen* /*next*/) {
    g_app.state = IDS_SINGLEPLAYER2;
    g_print("[SP] Quick Mission\n");
    return true;
}

bool cb_quickmission_fly(Screen* /*next*/) {
    g_app.in3d = true;
    miginit();
    migselect(1); // Ensure quick mission state is set after selecting mission
    migfly();
    g_print("[QM] Fly\n");
    return true;
}

bool cb_quickmission_variants(Screen* /*next*/) {
    g_app.in3d = false;
    g_print("[QM] Variants\n");
    return true;
}

bool cb_quickmission_debrief(Screen* /*next*/) {
    g_app.in3d = false;
    g_print("[QM] Debrief\n");
    return true;
}

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

static void on_rb_scenario_toggled(GtkToggleButton* btn, gpointer user)
{
    if (!gtk_toggle_button_get_active(btn))
        return;

    GtkWidget* mission_desc_box = GTK_WIDGET(user); 
    gtk_widget_show(mission_desc_box);
#if 0
    QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
    qd.plside = 2;   // Scenario mode

    auto* sw = static_cast<SideWidgets*>(user);
    rebuild_flight_combos(sw->flight_name_combo, sw->flight_lead_combo);
#endif

    printf("Scenario selected\n");
}


static void on_rb_un_toggled(GtkToggleButton* btn, gpointer user)
{
    if (gtk_toggle_button_get_active(btn)) {
        GtkWidget* mission_desc_box = GTK_WIDGET(user); 
        gtk_widget_hide(mission_desc_box); 

#if 0
        QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
        qd.plside = 0;

        GtkComboBox* flight_name_combo = GTK_COMBO_BOX(user);
        rebuild_flight_names(flight_name_combo);
#endif
        printf("UN selected\n");
    }
}

static void on_rb_red_toggled(GtkToggleButton* btn, gpointer user)
{
    if (gtk_toggle_button_get_active(btn)) {
        GtkWidget* mission_desc_box = GTK_WIDGET(user); 
        gtk_widget_hide(mission_desc_box); 
#if 0
        QuickDef& qd = CSQuick1::quickmissions[CSQuick1::currquickmiss];
        qd.plside = 1;

        GtkComboBox* flight_name_combo = GTK_COMBO_BOX(user);
        rebuild_flight_names(flight_name_combo);
#endif
        printf("Red selected\n");
    }
}



GtkWidget* make_quickmission_selector() {
    // Vertical box to hold everything (no black overlay)
    GtkWidget *panel = gtk_vbox_new(FALSE, 0);
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
    gtk_box_pack_start(GTK_BOX(mission_hbox), mission_label, FALSE, FALSE, 15);

    // Combo box on the right
    GtkWidget* mission_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(mission_combo, 260, -1); // widen if needed
    gtk_box_pack_start(GTK_BOX(mission_hbox), mission_combo, FALSE, FALSE, 15);

    // Fill mission combo
    int missionCount = 0;
    for (int i = 0; CSQuick1::quickmissions[i].missionname != 0; ++i) {
        CString cname = LoadResString(CSQuick1::quickmissions[i].missionname);
        gtk_combo_box_append_text(GTK_COMBO_BOX(mission_combo), cname);
        missionCount++;
    }

    // -------------------------------------------------
    // 2) Flight Name + Flight Lead (side-by-side)
    // -------------------------------------------------
    GtkWidget* flight_row_hbox = gtk_hbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(vbox), flight_row_hbox, FALSE, FALSE, 0);

    // --- Flight Name ---
    GtkWidget* flight_name_label = gtk_label_new("Flight  ");
    gtk_misc_set_alignment(GTK_MISC(flight_name_label), 1.0, 0.5);
    apply_lightblue_label(flight_name_label);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_name_label, FALSE, FALSE, 30);

    GtkWidget* flight_name_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(flight_name_combo, 160, -1);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_name_combo, FALSE, FALSE, 0);

    // Spacer between the two combos (optional)
    GtkWidget* spacer = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), spacer, FALSE, FALSE, 0);

    // --- Flight Lead ---
    GtkWidget* flight_lead_label = gtk_label_new(""); // Lead
    gtk_misc_set_alignment(GTK_MISC(flight_lead_label), 1.0, 0.5);
    apply_lightblue_label(flight_lead_label);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_lead_label, FALSE, FALSE, 0);

    GtkWidget* flight_lead_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(flight_lead_combo, 120, -1);
    gtk_box_pack_start(GTK_BOX(flight_row_hbox), flight_lead_combo, FALSE, FALSE, 0);

    // -------------------------------------------------
    // 3) Target Zone label + Target Type combo
    // -------------------------------------------------

    // Horizontal box for label + combo
    GtkWidget* target_type_hbox = gtk_hbox_new(FALSE, 12);   // spacing = 12px
    gtk_box_pack_start(GTK_BOX(vbox), target_type_hbox, FALSE, FALSE, 0);

    // Label on the left
    GtkWidget* target_type_label = gtk_label_new("Target Zone");
    gtk_misc_set_alignment(GTK_MISC(target_type_label), 1.0, 0.5); // right-align text
    apply_lightblue_label(target_type_label);
    gtk_box_pack_start(GTK_BOX(target_type_hbox), target_type_label, FALSE, FALSE, 40);

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

    // Combo box on the right
    GtkWidget* target_name_combo = gtk_combo_box_new_text();
    gtk_widget_set_size_request(target_name_combo, 260, -1); // same width as Target Zone
    gtk_box_pack_start(GTK_BOX(target_name_hbox), target_name_combo, FALSE, FALSE, 215);


    // -------------------------------------------------
    // 5) Side selection radio buttons (Scenario / UN / Red)
    // -------------------------------------------------
    GtkWidget* side_hbox = gtk_hbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(vbox), side_hbox, FALSE, FALSE, 0);

    // First button: Scenario (default)
    GtkWidget* rb_scenario = gtk_radio_button_new_with_label(NULL, "Scenario");
    apply_lightblue_label(gtk_bin_get_child(GTK_BIN(rb_scenario)));
    gtk_box_pack_start(GTK_BOX(side_hbox), rb_scenario, FALSE, FALSE, 30);

    // Second button: UN (same group as Scenario)
    GtkWidget* rb_un =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb_scenario), "UN");
    apply_lightblue_label(gtk_bin_get_child(GTK_BIN(rb_un)));
    gtk_box_pack_start(GTK_BOX(side_hbox), rb_un, FALSE, FALSE, 0);

    // Third button: Red (same group)
    GtkWidget* rb_red =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb_scenario), "Red");
    apply_lightblue_label(gtk_bin_get_child(GTK_BIN(rb_red)));
    gtk_box_pack_start(GTK_BOX(side_hbox), rb_red, FALSE, FALSE, 0);

    // Default selection
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_scenario), TRUE);



    // -------------------------------------------------
    // Mission description last
    // -------------------------------------------------
    // Mission description below
    GtkWidget* mission_desc = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(mission_desc), TRUE);
    gtk_label_set_width_chars(GTK_LABEL(mission_desc), 60);
    apply_lightblue_label(mission_desc);

    // Add some extra vertical spacing
    GtkWidget* desc_spacer = gtk_label_new("");
    gtk_widget_set_size_request(desc_spacer, -1, 80); // 80 pixels tall
    gtk_box_pack_start(GTK_BOX(vbox), desc_spacer, FALSE, FALSE, 0);

    // Now add the mission description label
    GtkWidget* mission_desc_box = gtk_hbox_new(FALSE, 12);   // spacing = 12px
    gtk_box_pack_start(GTK_BOX(vbox), mission_desc_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mission_desc_box), mission_desc, FALSE, FALSE, 0);

    gtk_widget_hide(mission_desc_box);

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
            //if (qd.plside == 0)
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
            //else
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
    g_signal_connect(rb_scenario, "toggled",
        G_CALLBACK(on_rb_scenario_toggled), mission_desc_box);

    g_signal_connect(rb_un, "toggled",
        G_CALLBACK(on_rb_un_toggled), mission_desc_box);

    g_signal_connect(rb_red, "toggled",
        G_CALLBACK(on_rb_red_toggled), mission_desc_box);



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

