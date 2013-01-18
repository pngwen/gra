/*
    Database interaction and object definitions for the paper database.
   
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

#include <glib.h>
#include <sqlite3.h>
#include <time.h>
#include "data.h"

static void create_schema(gra_db_t *db, GError **error);
static gboolean has_schema(gra_db_t *db, GError **error);
static gboolean has_schema_version(gra_db_t *db, GError **error);
static void schema_upgrade(gra_db_t *db, GError **error);

GQuark
gra_data_error_quark(void) {
  return g_quark_from_static_string("gra-data-error");
}

/* Open the database using sqlite3. If it does not exist, it is
   Created
 */
gra_db_t *
gra_db_open(const gchar *filename, GError **error) {
  gra_db_t *db;
  int code;

  /* fail on prior errors */
  if(error && *error) return;

  /* allocate the database and open it */
  db = (gra_db_t*) g_malloc(sizeof(gra_db_t));
  db->changed = FALSE;

  /* attempt to open the database */
  code = sqlite3_open(filename, &(db->db));
  if(code != SQLITE_OK) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));
    sqlite3_close(db->db);
    g_free(db);
    return NULL;
  }

  /* create the schema if needed */
  if(!has_schema(db, error)) {
    create_schema(db, error);
  }

  /* handle schema upgrade, if needed */
  if(!has_schema_version(db, error)) {
    schema_upgrade(db, error);
  }

  return db;
}


/* Close the database and destroy the connection. */
void
gra_db_close(gra_db_t *db, GError **error) {
  int rc;
  sqlite3_stmt *stmt=NULL;

  /* fail on prior errors */
  if(error && *error) return;
  
  /* update the meta info if it has changed */
  if(db->changed) {
    rc = sqlite3_prepare_v2(db->db, "UPDATE MetaInfo SET LastUpdate=?", -1, &stmt, 0);
    if(rc != SQLITE_OK) {
      g_set_error(error, GRA_DATA_ERROR, 1,
                  "SQLite Error: %s", sqlite3_errmsg(db->db));
      goto cleanup;
    }
    sqlite3_bind_int(stmt, 1, time(0));
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
      g_set_error(error, GRA_DATA_ERROR, 1,
                  "SQLite Error: %s", sqlite3_errmsg(db->db));
      goto cleanup;
    }
  }

  cleanup:
  if(stmt)
    sqlite3_finalize(stmt);
  sqlite3_close(db->db);
  g_free(db);
}



/*-------------------------------
 * static methods
 *-------------------------------*/

/* creates the database schema */
static void
create_schema(gra_db_t *db, GError **error) {
  const gchar *script[] = {
    "CREATE TABLE \"MetaInfo\" ("
      " \"Version\" REAL NOT NULL,"
      " \"Created\" INTEGER NOT NULL,"
      " \"LastUpdate\" INTEGER"
      " )",
    "CREATE TABLE \"Paper\" ("
      " \"ID\" INTEGER PRIMARY KEY AUTOINCREMENT,"
      " \"FileName\" TEXT NOT NULL,"
      " \"Contents\" BLOB,"
      " \"PageCount\" INTEGER,"
      " \"Read\" INTEGER NOT NULL,"
      " \"Type\" TEXT NOT NULL,"
      " \"Author\" TEXT NOT NULL,"
      " \"Title\" TEXT NOT NULL,"
      " \"Year\" INTEGER"
      " )",
    "CREATE TABLE \"Field\" ("
      " \"ID\" INTEGER PRIMARY KEY AUTOINCREMENT,"
      " \"PaperID\" INTEGER NOT NULL,"
      " \"Name\" TEXT NOT NULL,"
      " \"Value\" TEXT NOT NULL,"
      " FOREIGN KEY (\"PaperID\") REFERENCES \"Paper\"(\"ID\")"
      " )",
    "CREATE TABLE \"Reference\" ("
      " \"PaperID\" INTEGER NOT NULL,"
      " \"RefPaperID\" INTEGER NOT NULL,"
      " FOREIGN KEY (\"PaperID\") REFERENCES \"Paper\"(\"ID\"),"
      " FOREIGN KEY (\"RefPaperID\") REFERENCES \"Paper\"(\"ID\"),"
      " PRIMARY KEY (\"PaperID\", \"RefPaperID\")"
      " )",
    "CREATE TABLE \"Note\" ("
      " \"ID\" INTEGER PRIMARY KEY AUTOINCREMENT,"
      " \"PaperID\" INTEGER NOT NULL,"
      " \"Page\" INTEGER NOT NULL,"
      " \"LeftNote\" TEXT NOT NULL,"
      " \"RightNote\" TEXT NOT NULL,"
      " FOREIGN KEY (\"PaperID\") REFERENCES \"Paper\"(\"ID\")"
      ")"       
  };
  int i;
  int n = sizeof(script) / sizeof(script[0]);
  sqlite3_stmt *stmt=NULL;
  int rc;
  char *zErrMsg = 0;

  /* fail on prior errors */
  if(error && *error) return;

  /* run all items in the script */
  for(i=0; i<n; i++) {
    rc = sqlite3_prepare_v2(db->db, script[i], -1, &stmt, 0);
    if(rc != SQLITE_OK) {
      g_set_error(error, GRA_DATA_ERROR, 1,
                  "SQLite Error: %s", sqlite3_errmsg(db->db));
      return;
    }

    /* run and destroy */
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
      g_set_error(error, GRA_DATA_ERROR, 1,
                  "SQLite Error: %s", sqlite3_errmsg(db->db));

      goto cleanup;
    }
    sqlite3_finalize(stmt);
  }

  /* update the MetaInfo table */
  rc = sqlite3_prepare_v2(db->db,
                          "INSERT INTO MetaInfo (\"Version\", \"Created\")"
                          " VALUES(?, ?)",
                          -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));

    goto cleanup;
  }
  sqlite3_bind_double(stmt, 1, GRA_DB_VERSION);
  sqlite3_bind_int(stmt, 2, time(0));
  
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_DONE) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));

    goto cleanup;
  }

  /* it's been changed! */
  db->changed = TRUE;
  
  cleanup:  
  sqlite3_finalize(stmt);
}


/* Check to see if the schema is present */
static gboolean
has_schema(gra_db_t *db, GError **error) {
  int rc;
  sqlite3_stmt *stmt=NULL;
  const unsigned char *name;
  gboolean result = FALSE;

  /* fail on prior errors */
  if(error && *error) return FALSE;

  rc = sqlite3_prepare_v2(db->db, "SELECT \"NAME\" FROM \"SQLITE_MASTER\"", -1, &stmt, 0);
  if(rc !=SQLITE_OK) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));

    return;
  }

  while(sqlite3_step(stmt) == SQLITE_ROW) {
    name = sqlite3_column_text(stmt, 0);
    if(g_strcmp0(name, "MetaInfo")) {
      result = TRUE;
      break;
    }
  }
  
  cleanup:
  sqlite3_finalize(stmt);

  return result;
}


/* Check to see if we are at current schema version */
static gboolean
has_schema_version(gra_db_t *db, GError **error) {
  int rc;
  sqlite3_stmt *stmt=NULL;
  gboolean result = FALSE;

  /* fail on prior errors */
  if(error && *error) return FALSE;

  rc = sqlite3_prepare_v2(db->db, "SELECT \"Version\" FROM \"MetaInfo\"", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));

    return FALSE;
  }

  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));

    goto cleanup;
  }

  if(sqlite3_column_double(stmt, 0) == GRA_DB_VERSION) {
    result = TRUE;
  }

  cleanup:
  sqlite3_finalize(stmt);

  return result;
}


/* bring schema up to date with current schema */
static void
schema_upgrade(gra_db_t *db, GError **error) {
  /* TODO: When upgrade is available, write this function! */
  
  /* fail on prior errors */
  if(error && *error) return;

}
