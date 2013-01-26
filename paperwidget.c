/*
    Paper widget for displaying paper fields of an individual paper.
   
       Copyright (C) 2013 Robert Lowe <pngwen@acm.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "paperwidget.h"

static void addLabelRow(gra_paper_widget *pw, const gchar* text, GtkWidget *widget);
static void addFieldRow(gra_paper_widget *pw, GtkWidget *w1, GtkWidget *w2);


gra_paper_widget *
gra_paper_widget_new(gra_paper_t *paper) {
  gra_paper_widget *pw;
  
  
  /* TODO: Magic to glue to the paper struct */

  /* create the widgets */
  pw = g_malloc(sizeof(gra_paper_widget));
  pw->widget = gtk_vbox_new(FALSE, 1);
  pw->fieldBox = gtk_vbox_new(TRUE, 2);
  pw->type = gtk_combo_box_new_with_entry();
  pw->title = gtk_entry_new();
  pw->author = gtk_entry_new();
  pw->year = gtk_entry_new();
  pw->fieldNames = NULL;
  pw->fieldValues = NULL;

  /* pack widgets and label */
  gtk_box_pack_start(GTK_BOX(pw->widget), pw->fieldBox, FALSE, FALSE, 0);
  gtk_widget_show(pw->fieldBox);
  addLabelRow(pw, "Type", pw->type);
  addLabelRow(pw, "Title", pw->title);
  addLabelRow(pw, "Author", pw->author);
  addLabelRow(pw, "Year", pw->year);

  /* show the last widet in the container.
     Caller is responsible for the container */
  gtk_widget_show(pw->fieldBox);
  return pw;
}



/*
 * Static Methods
 */
static void
addLabelRow(gra_paper_widget *pw, const gchar* text, GtkWidget *widget) {
  GtkWidget *label;

  /* create the label, then then the field row */
  label = gtk_label_new(text);
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
  addFieldRow(pw, label, widget);
}


static void
addFieldRow(gra_paper_widget *pw, GtkWidget *w1, GtkWidget *w2) {
  GtkWidget *row;

  /* allocate this row */
  row = gtk_hbox_new(TRUE, 2);

  gtk_box_pack_start(GTK_BOX(row), w1, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(row), w2, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(pw->fieldBox), row, FALSE, FALSE, 0);
  gtk_widget_show(w1);
  gtk_widget_show(w2);
  gtk_widget_show(row);

}
