/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2019  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include <gum/widgets.h>
#include <gum/events.h>


int main()
{
    gum_initialize();

    GUM_container *window = gum_container_window(0);
    GUM_container *grp = gum_container_create(window, "VGroupExtend", 5);
    GUM_container *grp1 = gum_container_create(grp, "HGroupTop", 10);
    GUM_container *grp2 = gum_container_create(grp, "Grid", 0);

    GUM_container *grp21 = gum_container_create(grp2, "VGroupExtend", 10);
    GUM_container *grp22 = gum_container_create(grp2, "VGroupExtend", 10);
    GUM_container *grp23 = gum_container_create(grp2, "VGroupExtend", 10);
    GUM_container *grp24 = gum_container_create(grp2, "VGroupExtend", 10);

    gum_create_widget(grp1, "Label", "Style: ");
    gum_create_widget(grp1, "ComboBox", "Fusion");
    gum_create_widget(grp1, "CheckBox", "Use style standard palette");
    gum_create_widget(grp1, "CheckBox", "Disable widget");
    gum_create_widget(grp21, "RadioButton", "Radio button 1");
    gum_create_widget(grp21, "RadioButton", "Radio button 2");
    gum_create_widget(grp21, "RadioButton", "Radio button 3");
    gum_create_widget(grp21, "CheckBox", "Tri-state check box");
    gum_create_widget(grp22, "PushButton", "Default push button");
    gum_create_widget(grp22, "PushButton", "Toggle push button");
    gum_create_widget(grp22, "PushButton", "Flat push button");
    // Add stack view
    gum_create_widget(grp24, "TextEdit", "Text edit");
    gum_create_widget(grp24, "SpinBox", "50");
    gum_create_widget(grp24, "SpinBox", "2019-08-12 20:54");
    gum_create_widget(grp24, "Slider", NULL);
    gum_create_widget(grp, "ProgressBar", "Progress 10%");

    gum_event_loop(window->box.manager);
    gum_close_manager(window->box.manager);
    // Destroy widgets
    return 0;
}

