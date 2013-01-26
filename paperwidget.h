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
#ifndef PAPERWIDGET_H
#define PAPERWIDGET_H

#include <gtk/gtk.h>
#include <glib.h>
#include "datatypes.h"


/**  TODO: Write documentation block. 
 * 
 */
typedef struct gra_paper_widget {
  gra_paper_t *paper;
  GtkWidget   *widget;
  GtkWidget   *fieldBox;
  GtkWidget   *type;
  GtkWidget   *title;
  GtkWidget   *author;
  GtkWidget   *year;
  GList       *fieldNames;
  GList       *fieldValues;
} gra_paper_widget;

/** Create a new paper widget.  If paper is NULL, a new paper
 *  is allocated.
 */
gra_paper_widget *gra_paper_widget_new(gra_paper_t *paper);
#endif
