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
static void addFieldRow(gra_paper_widget *pw);
static void addRow(gra_paper_widget *pw, GtkWidget *row, GtkWidget *w1, GtkWidget *w2);
static void addFieldButtonClicked(GtkWidget *widget, gpointer data);

/**
 * These are the standard bibtex fields.  Minus the ones which are
 * always defined.
 */
static gchar* standardFields[] = {
  "address", "editor", "eprint", "institution", "journal", "key",
    "month", "note", "organization", "pages", "publisher", "school",
    "series", "ur", "volume"
};


/**
 * These are the standard document types used in bibtex.
 */
static gchar* standardTypes[] = {
  "Article", "Book", "Booklet", "Conference", "Inbook", "Incollection",
    "Inproceedings", "Manual", "Mastersthesis", "misc", "Phdthesis",
    "Techreport", "Unpublished"
};


gra_paper_widget *
gra_paper_widget_new(gra_paper_t *paper) {
  gra_paper_widget *pw;
  GtkWidget *button;
  int i, n;
  
  
  /* TODO: Magic to glue to the paper struct */

  /* create the widgets */
  pw = g_malloc(sizeof(gra_paper_widget));
  pw->widget = gtk_vbox_new(FALSE, 1);
  pw->fieldBox = gtk_vbox_new(TRUE, 2);
  pw->type = gtk_combo_box_text_new_with_entry();
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
  addRow(pw,
         gtk_hbox_new(TRUE, 2),
         gtk_label_new("Field"),
         gtk_label_new("Value"));
  addFieldRow(pw);

  /* populate type dropdown */
  n = sizeof(standardTypes) / sizeof(standardTypes[0]);
  for(i=0; i<n; i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pw->type),
                                   standardTypes[i]);
  }

  /*add the add button */
  button = gtk_button_new_with_label("Add Field");
  gtk_box_pack_start(GTK_BOX(pw->widget), button, FALSE, FALSE, 0);
  gtk_widget_show(button);
  g_signal_connect(G_OBJECT(button),
                   "clicked",
                   G_CALLBACK(addFieldButtonClicked),
                   pw);

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

  addRow(pw, gtk_hbox_new(FALSE, 2), label, widget);
}


static void
addFieldRow(gra_paper_widget *pw) {
  GtkWidget *field, *value;
  int i;
  int n = sizeof(standardFields) / sizeof(standardFields[0]);
  
  /* create the combo box and entry */
  field = gtk_combo_box_text_new_with_entry();
  value = gtk_entry_new();

  for(i=0; i<n; i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(field), standardFields[i]);
  }

  addRow(pw, gtk_hbox_new(TRUE, 2), field, value);
}


static void
addRow(gra_paper_widget *pw, GtkWidget *row, GtkWidget *w1, GtkWidget *w2) {
  gtk_widget_set_size_request(w1, 80, -1);
  gtk_box_pack_start(GTK_BOX(row), w1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(row), w2, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(pw->fieldBox), row, FALSE, FALSE, 0);
  gtk_widget_show(w1);
  gtk_widget_show(w2);
  gtk_widget_show(row);
}


static void
addFieldButtonClicked(GtkWidget *widget, gpointer data) {
  gra_paper_widget *pw;

  pw = (gra_paper_widget *) data;

  addFieldRow(pw);
}
