/*
    Database and object interaction for the paper database.
   
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

#ifndef DATA_H
#define DATA_H

#include <sqlite3.h>
#include <glib.h>
#include "datatypes.h"

#define GRA_DB_VERSION 1.0
#define GRA_DATA_ERROR gra_data_error_quark()

GQuark gra_data_error_quark(void);


/** Opens a database.  If the database does not exist, it is created.
 *  @param filename The name of the file we are opening.
 *  @param error GError Pointer.  Set to NULL for no error reporting.
 *  @return On success, it returns a dynamically allocated gra_db_t
 *  structure.  On failure, returns NULL.  The returned struct should
 *  only be destroyed by calling gra_db_close.
 *  @see gra_db_close
 */
gra_db_t *gra_db_open(const gchar *filename, GError **error);


/** Closes a database and destroys the connection.
 *  @param db the database to close
 *  @param error GError Pointer.  Set to NULL for no error reporting.
 *  @see gra_db_open
 */
void gra_db_close(gra_db_t *db, GError **error);


/* paper functions */
gra_paper_t *gra_db_paper_load(gra_db_t *db, int id, GError **error);
void gra_db_paper_save(gra_db_t *db, gra_paper_t *p, GError **error);
void gra_db_paper_delete(gra_db_t *db, gra_paper_t *p, GError **error);
void gra_db_paper_load_fields(gra_db_t *db, gra_paper_t *p, GError **error);
void gra_db_paper_load_refs(gra_db_t *db, gra_paper_t *p, GError **error);

/* field functions */
void gra_db_field_save(gra_db_t *db, gra_field_t *f, GError **error);
void gra_db_field_delete(gra_db_t *db, gra_field_t *f, GError **error);

/* reference functions */
void gra_db_reference_save(gra_db_t *db, gra_reference_t *r, GError **error);
void gra_db_reference_delete(gra_db_t *db, gra_reference_t *r, GError ** error);

/* note functions */
void gra_db_note_save(gra_db_t *db, gra_note_t *f, GError **error);
void gra_db_note_delete(gra_db_t *db, gra_note_t *f, GError **error);

/* search functions */
GList *gra_db_search_keyword(gra_db_t *db, const gchar *keyword, GError **error);
GList *gra_db_search_title(gra_db_t *db, const gchar *title, GError **error);
GList *gra_db_search_author(gra_db_t *db, const gchar *author, GError **error);
#endif
