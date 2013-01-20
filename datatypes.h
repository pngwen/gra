/*
    Data type definitions for the paper database.
   
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
#ifndef DATATYPES_H
#define DATATYPES_H

/** @struct gra_db_t
 *  @brief The database type for all data interactions.
 *  @var gra_database_t::db
 *  Member 'db' contains the pointer to the sqlite connection.
 *    
 *  @var gra_database_t::changed
 *  Initially set to false.  If any function alters the database,
 *  changed is set to true.
 *
 *  @var gra_database_t::version Version of te schema
 *  @var gra_database_t::created Timestamp of the creation time of db.
 *  @var gra_database_t::lastUpdate Timestamp of last update of db.
 */
typedef struct gra_db_t {
  sqlite3 *db;
  gboolean changed;
  /* meta information about the db */
  double version;
  int created;
  int lastUpdate;
} gra_db_t;


/** @struct gra_paper_t
 *  @brief This is the root structure for storing papers.
 *  @var gra_paper_t::id Unique ID for the paper
 *  @var gra_paper_t::fileName Original filename of the paper
 *  @var gra_paper_t::pageCount Number of pages in the paper
 *  @var gra_paper_t::read True if paper has been read, False otherwise
 *  @var gra_paper_t::type Type of the paper (ARTICLE, BOOK, etc.)
 *  @var gra_paper_t::author Author of the paper.
 *  @var gra_paper_t::title Title of the paper.
 *  @var gra_paper_t::year The year of the paper's publication
 *  @var gra_paper_t::fields This paper's additional fields
 *  This uses GTree as an associative table.  The key is the field
 *  name and the value is the field struct itself.
 *  @var gra_paper_t::refs The papers referenced by this paper.
 *  @var gra_paper_t::indb True if paper is in DB, False otherwise
 *  @var gra_paper_t::changed True if changed, false if not.
 */
typedef struct gra_paper_t {
  int id;
  gchar *fileName;
  int pageCount;
  gboolean read;
  gchar *type;
  gchar *author;
  gchar *title;
  unsigned int year;
  GTree *fields;
  GList *refs;
  gboolean indb;
  gboolean changed;
} gra_paper_t;


/** @struct gra_field_t
 *  @brief Used to store extra fields for the paper.
 *  @var gra_field_t::id ID of the field row
 *  @var gra_field_t::paperId ID of the paper this belongs to.
 *  @var gra_field_t::name The name of the field
 *  @var gra_field_t::value The value of the field
 *  @var gra_field_t::indb True if the field is in DB, False otherwise.
 *  @var gra_field_t_t::changed True if changed, false if not.
 */
typedef struct gra_field_t {
  int id;
  int paperId;
  gchar *name;
  gchar *value;
  gboolean indb;
  gboolean changed;
} gra_field_t;


/** @struct gra_reference_t
 *  @brief A key structure which glues together a paper and its
 *         referenced papers.
 *  @var gra_reference_t::id ID of the reference row
 *  @var gra_reference_t::paperId ID of the paper.
 *  @var gra_reference_t::refPaperId ID of the cited paper
 *  @var gra_reference_t::indb True if the field is in DB, False Otherwise.
 *  @var gra_reference_t::changed True if changed, false otherwise.
 */
typedef struct gra_reference_t {
  int id;
  int paperId;
  int refPaperId;
  gboolean indb;
  gboolean changed;
} gra_reference_t;


/** @struct gra_note_t
 *  @brief Stores the notes for a given paper.  
 *  @var gra_note_t::id ID of the note row.
 *  @var gra_note_t::paperId ID of the paper this note belongs to.
 *  @var gra_note_t::page The page this note block belongs to.
 *  @var gra_note_t::leftNote The notes for the left hand margin.
 *  @var gra_note_t::rightNote The notes for the right hand margin.
 *  @var gra_note_t::indb True if the note is in DB, False otherwise.
 *  @var gra_note_t::changed True if changed, false if not.
 */
typedef struct gra_note_t {
  int id;
  int paperId;
  int page;
  gchar *leftNote;
  gchar *rightNote;
  gboolean indb;
  gboolean changed;
} gra_note_t;

#endif
