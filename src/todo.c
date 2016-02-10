#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>

#include "dbg.h"

#define UNUSED(x) (void)(x)

static char * todo_file = "/todo.txt";
static char * tmp_file = "/todo.tmp.txt";
static char * filename;
static char * tmp_filename;

static char *
init_filename (char * file)
{
    char * filename;
    char * home_dir = getenv ("TODO_DIR");
    if (!home_dir) {
        home_dir = getenv ("HOME");
    }
    check (home_dir, "Failed to get HOME");
    size_t home_dir_len = strlen (home_dir);

    filename = malloc (home_dir_len + strlen(file) + 1);
    check_mem (filename);

    memcpy (filename, home_dir, home_dir_len);
    memcpy (filename + home_dir_len, file, strlen (file));
    filename[home_dir_len + strlen (file)] = '\0';

    return filename;
error:
    return NULL;
}

/**
 * Get the file with the specified mode.
 */
static FILE *
get_file (const char * mode)
{
    int fd = open (filename, O_CREAT, S_IRUSR | S_IWUSR);
    check (fd != -1, "Failed to open file [%s]", filename);
    close (fd);
    return fopen (filename, mode);
error:
    return NULL;
}

static int
list(void)
{
    FILE * f = get_file ("r");
    check (f, "Failed to list");

    int c;
    int id = 1;
    bool newline = false;

    c = fgetc (f);
    if (c == EOF) {
        fclose (f);
        return 0;
    }
    fprintf (stdout, "%d: %c", id, c);
    while ((c = fgetc (f)) != EOF) {
        if (c == '\n') {
            newline = true;
        } else {
            if (newline) {
                fprintf (stdout, "\n%d: %c", ++id, c);
            } else {
                fputc (c, stdout);
            }
            newline = false;
        }
    }

    fclose (f);
    return 0;
error:
    return -1;
}

static inline int
delete_line (FILE * in)
{
    int c;
    while ((c = fgetc (in)) != EOF) {
        if (c == '\n') {
            return 0;
        }
    }
    return 0;
}

static int
delete (int line)
{
    int fd = -1;
    FILE * in = NULL, * out = NULL;

    char * new_filename = tmp_filename;

    fd = open (new_filename, O_CREAT, S_IRUSR | S_IWUSR);
    check (fd != -1, "Failed to create new file");
    close (fd);

    out = fopen (new_filename, "w");
    check (out, "Failed to open tmp file");

    in = get_file ("r");
    check (in, "Failed to open todo file for reading");

    int c;
    int id = 1;

    c = fgetc (in);
    ungetc (c, in);
    if (c == EOF) {
        sentinel ("Empty todo file, can't delete");
    }
    if (line == id) {
        delete_line (in);
    }

    while ((c = fgetc (in)) != EOF) {
        fputc (c, out);
        if (c == '\n') {
            int next = fgetc (in);
            ungetc (next, in);
            if (next != EOF && line == ++id) {
                delete_line (in);
            }
        }
    }

    fclose (in);
    fclose (out);

    unlink (filename);
    rename (new_filename, filename);

    return 0;

error:
    if (fd != -1) close (fd);
    if (in) fclose (in);
    if (out) fclose (out);
    return -1;
}

static int
create (int argc, char * argv[])
{
    FILE * f = get_file ("a");
    check (f, "Failed to open file in append mode");

    for (int i = 1; i < argc; i++) {
        fprintf (f, "%s", argv[i]);
        if (i < argc - 1) {
            fputc (' ', f);
        }
    }
    fputc ('\n', f);

    fclose (f);
    return 0;
error:
    return -1;
}

static inline bool
streq (char * a, char * b)
{
    return strcmp (a, b) == 0;
}

static void
usage (char * name)
{
    printf ("%s: [-f <id>]\n", name);
    printf ("\tlist:   %s\n", name);
    printf ("\tdelete: %s -f <num>\n", name);
    printf ("\tcreate: %s <str>\n", name);
}

int
main (int argc, char * argv[])
{
    int rc;

    filename = init_filename (todo_file);
    check (filename, "failed to get filename");
    tmp_filename = init_filename (tmp_file);
    check (tmp_filename, "failed to get tmp filename");

    if (argc == 1) {
        rc = list ();
        check (rc == 0, "Failed to list");
    } else if (argc == 2 && streq (argv[1], "-f")) {
        rc = delete (1);
        check (rc == 0, "Failed to finish first task");
        rc = list ();
        check (rc == 0, "Failed to list");
    } else if (argc == 3 && streq (argv[1], "-f")) {
        int id = atoi (argv[2]);
        rc = delete (id);
        check (rc == 0, "Failed to delete %d", id);
        rc = list ();
        check (rc == 0, "Failed to list");
    } else if (argc >= 2 && argv[1][0] != '-') {
        rc = create (argc, argv);
        check (rc == 0, "Failed to create new item");
        rc = list ();
        check (rc == 0, "Failed to list");
    } else {
        usage (basename (argv[0]));
        return 1;
    }

    return 0;

error:
    return 2;
}
