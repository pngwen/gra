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
#define DB_ERROR(error)  g_set_error(error, GRA_DATA_ERROR, 1, "SQLite Error: %s", sqlite3_errmsg(db->db))

static void create_schema(gra_db_t *db, GError **error);
static gboolean has_schema(gra_db_t *db, GError **error);
static gboolean has_schema_version(gra_db_t *db, GError **error);
static void schema_upgrade(gra_db_t *db, GError **error);
static gint fieldcmp(gconstpointer, gconstpointer);

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
  if(error && *error) return NULL;

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


gra_paper_t *
gra_db_paper_load(gra_db_t *db, int id, GError **error) {
  gra_paper_t *result;
  sqlite3_stmt *stmt=NULL;
  int rc;
  
  /* abort on previous error */
  if(error && *error) return NULL;

  rc = sqlite3_prepare_v2(db->db, "SELECT ID, FileName, PageCount, Read, Type, Author, Title, Year FROM \"Paper\" WHERE ID=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));
    goto cleanup;
  }

  /* finish off the query and run*/
  sqlite3_bind_int(stmt, 1, id);
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    g_set_error(error, GRA_DATA_ERROR, 1,
                "SQLite Error: %s", sqlite3_errmsg(db->db));
    goto cleanup;
  }

  /* build the result */
  result = g_malloc(sizeof(gra_paper_t));
  if(!result) {
    goto cleanup;
  }
  result->id = sqlite3_column_int(stmt, 0);
  result->fileName = g_strdup((gchar*)sqlite3_column_text(stmt, 1));
  result->pageCount = sqlite3_column_int(stmt, 2);
  result->read = sqlite3_column_int(stmt, 3);
  result->type = g_strdup((gchar*)sqlite3_column_text(stmt, 4));
  result->author = g_strdup((gchar*)sqlite3_column_text(stmt, 5));
  result->title = g_strdup((gchar*)sqlite3_column_text(stmt, 6));
  result->year = sqlite3_column_int(stmt, 7);
  result->fields = NULL;
  result->refs = NULL;
  result->indb = TRUE;
  result->changed = FALSE;

  /* all done! */
  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return result;
}


void
gra_db_paper_save(gra_db_t *db, gra_paper_t *p, GError **error) {
  sqlite3_stmt *stmt=NULL;
  int rc;
  
  /* abort on previous error */
  if(error && *error) return;

  /* do not save unchanged papers */
  if(!p->changed)
    return;

  if(p->indb) {
    /* prepare update */
    rc = sqlite3_prepare_v2(db->db, "UPDATE \"Paper\" SET \"Read\"=?, \"Type\"=?, \"Author\"=?, \"Title\"=?, \"Year\"=? WHERE \"ID\"=?", -1, &stmt, 0);
    if(rc == SQLITE_OK)
      rc = sqlite3_bind_int(stmt, 6, p->id);
  } else {
    /* prepare insert */
    rc = sqlite3_prepare_v2(db->db, "INSERT INTO \"Paper\" (\"Read\", \"Type\", \"Author\", \"Title\", \"Year\") VALUES(?, ?, ?, ?, ?)", -1, &stmt, 0);
  }

  /* handle statement errors */
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind the values */
  sqlite3_bind_int(stmt, 1, p->read);
  sqlite3_bind_text(stmt, 2, p->type, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, p->author, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 4, p->title, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 5, p->year);

  /* run the query */
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* handle new rows properly */
  if(!p->indb) {
    p->id = sqlite3_last_insert_rowid(db->db);
    p->indb = TRUE;
  }

  /* the database is now current */
  p->changed = FALSE;

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


void
gra_db_paper_delete(gra_db_t *db, gra_paper_t *p, GError **error) {
  int rc;
  sqlite3_stmt *stmt = NULL;

  /* abort on previous error */
  if(error && *error) return;

  rc = sqlite3_prepare_v2(db->db, "DELETE FROM \"Paper\" WHERE \"ID\"=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind and run the delete */
  sqlite3_bind_int(stmt, 1, p->id);
  rc = sqlite3_step(stmt);

  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* this is no longer in the db, mark it as such */
  p->indb = FALSE;
  p->changed = TRUE;

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


void
gra_db_paper_load_fields(gra_db_t *db, gra_paper_t *p, GError **error) {
  int rc;
  sqlite3_stmt *stmt = NULL;
  gra_field_t *field = NULL;

  /* abort on previous error */
  if(error && *error) return;

  rc = sqlite3_prepare_v2(db->db, "SELECT \"ID\", \"Name\", \"Value\" FROM \"Field\" WHERE \"PaperID\"=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }
  sqlite3_bind_int(stmt, 1, p->id);

  /* set up GTree */
  if(!p->fields)
    p->fields = g_tree_new(fieldcmp);
  if(!p->fields) {
    g_set_error(error, GRA_DATA_ERROR, 2, "Could not allocate binary tree for fields.");
    goto cleanup;
  }
    

  /* loop through results */
  while((rc=sqlite3_step(stmt)) == SQLITE_ROW) {
    /* create field structure */
    field = g_malloc(sizeof(gra_field_t));
    if(!field) {
      g_set_error(error, GRA_DATA_ERROR, 2, "Could not allocate field.");
      goto cleanup;
    }

    /* populate the field */
    field->id = sqlite3_column_int(stmt, 0);
    field->paperId = p->id;
    field->name = g_strdup((gchar*) sqlite3_column_text(stmt, 1));
    field->value = g_strdup((gchar*) sqlite3_column_text(stmt, 2));
    field->indb = TRUE;
    field->changed = FALSE;

    /* add the field to the tree */
    g_tree_insert(p->fields, field->name, field);
  }

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


void
gra_db_paper_load_refs(gra_db_t *db, gra_paper_t *p, GError **error) {
  int rc;
  sqlite3_stmt *stmt = NULL;
  gra_reference_t *ref = NULL;

  /* abort on previous error */
  if(error && *error) return;

  rc = sqlite3_prepare_v2(db->db, "SELECT \"ID\", \"RefPaperID\" FROM \"Reference\" WHERE \"PaperID\"=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }
  sqlite3_bind_int(stmt, 1, p->id);


  /* loop through results */
  while((rc=sqlite3_step(stmt)) == SQLITE_ROW) {
    /* create field structure */
    ref = g_malloc(sizeof(gra_reference_t));
    if(!ref) {
      g_set_error(error, GRA_DATA_ERROR, 2, "Could not allocate field.");
      goto cleanup;
    }

    /* populate the field */
    ref->id = sqlite3_column_int(stmt, 0);
    ref->paperId = p->id;
    ref->refPaperId = sqlite3_column_int(stmt, 1);
    ref->indb = TRUE;
    ref->changed = FALSE;

    /* add the reference to the list */
    p->refs = g_list_prepend(p->refs, ref);
  }

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


/* field functions */
void
gra_db_field_save(gra_db_t *db, gra_field_t *f, GError **error) {
  sqlite3_stmt *stmt = NULL;
  int rc;

  /* abort on previous error */
  if(error && *error) return;

  /* do not save unchanged fields */
  if(!f->changed)
    return;

  if(f->indb) {
    /* prepare update */
    rc = sqlite3_prepare_v2(db->db, "UPDATE \"Field\" SET \"PaperID\"=?, \"Name\"=?, \"Value\"=? WHERE \"ID\"=?", -1, &stmt, 0);
    if(rc != SQLITE_OK) {
      DB_ERROR(error);
      goto cleanup;
    }
    rc = sqlite3_bind_int(stmt, 4, f->id);
  } else {
    /* prepare insert */
    rc = sqlite3_prepare_v2(db->db, "INSERT INTO \"Field\" (\"PaperID\", \"Name\", \"Value\") VALUES(?, ?, ?)", -1, &stmt, 0);
  }

  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind the colums and run */
  sqlite3_bind_int(stmt, 1, f->paperId);
  sqlite3_bind_text(stmt, 2, f->name, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, f->value, -1, SQLITE_TRANSIENT);
  rc = sqlite3_step(stmt);

  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* handle new rows properly */
  if(!f->indb) {
    f->id = sqlite3_last_insert_rowid(db->db);
    f->indb = TRUE;
  }

  /* the database is now current */
  f->changed = FALSE;
  

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


void
gra_db_field_delete(gra_db_t *db, gra_field_t *f, GError **error) {
  int rc;
  sqlite3_stmt *stmt = NULL;

  /* abort on previous error */
  if(error && *error) return;

  rc = sqlite3_prepare_v2(db->db, "DELETE FROM \"Field\" WHERE \"ID\"=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind and run the delete */
  sqlite3_bind_int(stmt, 1, f->id);
  rc = sqlite3_step(stmt);

  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* this is no longer in the db, mark it as such */
  f->indb = FALSE;
  f->changed = TRUE;

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


/* reference functions */
void
gra_db_reference_save(gra_db_t *db, gra_reference_t *r, GError **error) {
  sqlite3_stmt *stmt = NULL;
  int rc;

  /* abort on previous error */
  if(error && *error) return;

  /* do not save unchanged fields */
  if(!r->changed)
    return;

  if(r->indb) {
    /* prepare update */
    rc = sqlite3_prepare_v2(db->db, "UPDATE \"Reference\" SET \"PaperID\"=?, \"RefPaperID\"=? WHERE \"ID\"=?", -1, &stmt, 0);
    if(rc != SQLITE_OK) {
      DB_ERROR(error);
      goto cleanup;
    }
    rc = sqlite3_bind_int(stmt, 3, r->id);
  } else {
    /* prepare insert */
    rc = sqlite3_prepare_v2(db->db, "INSERT INTO \"Reference\" (\"PaperID\", \"RefPaperID\") VALUES(?, ?)", -1, &stmt, 0);
  }

  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind the colums and run */
  sqlite3_bind_int(stmt, 1, r->paperId);
  sqlite3_bind_int(stmt, 2, r->refPaperId);
  rc = sqlite3_step(stmt);

  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* handle new rows properly */
  if(!r->indb) {
    r->id = sqlite3_last_insert_rowid(db->db);
    r->indb = TRUE;
  }

  /* the database is now current */
  r->changed = FALSE;
  

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
}


void
gra_db_reference_delete(gra_db_t *db, gra_reference_t *r, GError ** error) {
  int rc;
  sqlite3_stmt *stmt = NULL;

  /* abort on previous error */
  if(error && *error) return;

  rc = sqlite3_prepare_v2(db->db, "DELETE FROM \"Reference\" WHERE \"ID\"=?", -1, &stmt, 0);
  if(rc != SQLITE_OK) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* bind and run the delete */
  sqlite3_bind_int(stmt, 1, r->id);
  rc = sqlite3_step(stmt);

  if(rc != SQLITE_DONE) {
    DB_ERROR(error);
    goto cleanup;
  }

  /* this is no longer in the db, mark it as such */
  r->indb = FALSE;
  r->changed = TRUE;

  cleanup:
  if(stmt) sqlite3_finalize(stmt);
  return;
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
  if(stmt) sqlite3_finalize(stmt);
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

    goto cleanup;
  }

  while(sqlite3_step(stmt) == SQLITE_ROW) {
    name = sqlite3_column_text(stmt, 0);
    if(g_strcmp0((gchar*)name, "MetaInfo")) {
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


static gint
fieldcmp(gconstpointer a, gconstpointer b) {
  return g_strcmp0((gchar*) a, (gchar*) b);
}
