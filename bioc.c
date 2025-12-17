#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* =====================================================
      BioC vAlpha4-Lite — Type-Aware Transpiler
      Arrays only — NO dicts
   ===================================================== */

/* ---------- Helpers ---------- */

void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace(s[len - 1])) s[--len] = 0;
}

int starts_with(const char *s, const char *p) {
    return strncmp(s, p, strlen(p)) == 0;
}

void strip_inline_comment(char *s) {
    char *h = strstr(s, "#");
    if (h) *h = '\0';
    trim(s);
}

int is_string_literal(const char *s) {
    return s[0] == '"' || s[0] == '\'';
}

/* =====================================================
      Variable Type Table (for echo() type detection)
   ===================================================== */

typedef struct {
    char name[128];
    char type[64];
} Var;

Var vars[2048];
int var_count = 0;

void add_var(const char *name, const char *type) {
    strcpy(vars[var_count].name, name);
    strcpy(vars[var_count].type, type);
    var_count++;
}

const char *get_type(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].type;
    }
    return NULL;
}

/* =====================================================
                 TRANSPILER START
   ===================================================== */

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: bioc <file.bc>\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("error: cannot open file\n");
        return 1;
    }

    FILE *out = fopen("out.c", "w");

    /* ---------- Auto Headers ---------- */
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <string.h>\n");
    fprintf(out, "#include <stdbool.h>\n");
    fprintf(out, "#include \"bioc_stdlib.h\"\n\n");

    fprintf(out, "/* --- BioC transpiled code --- */\n\n");

    char line[2048];
    int main_started = 0;

    /* =====================================================
           PASS 1 — Write functions BEFORE main()
       ===================================================== */

    rewind(in);
    while (fgets(line, sizeof(line), in)) {
        strip_inline_comment(line);
        if (strlen(line) == 0) continue;

        if (strstr(line, ") -> ")) {
            char rt1[64], name[128], args[512], rt2[64];
            sscanf(line, "%63s %127[^ (](%511[^)]) -> %63s",
                   rt1, name, args, rt2);

            fprintf(out, "%s %s(%s) {\n", rt2, name, args);

            while (fgets(line, sizeof(line), in)) {
                if (strstr(line, "}")) break;
                fprintf(out, "%s", line);
            }

            fprintf(out, "}\n\n");
        }
    }

    /* =====================================================
           PASS 2 — MAIN CODE
       ===================================================== */

    rewind(in);

    while (fgets(line, sizeof(line), in)) {

        strip_inline_comment(line);
        if (strlen(line) == 0) continue;

        /* skip functions */
        if (strstr(line, ") -> ")) {
            while (fgets(line, sizeof(line), in))
                if (strstr(line, "}")) break;
            continue;
        }

        /* add custom header */
        if (starts_with(line, "add ")) {
            char h[256];
            sscanf(line, "add %255[^;];", h);
            trim(h);

            if (strcmp(h, "stdio.h") != 0)
                fprintf(out, "#include \"%s\"\n", h);
            continue;
        }

        /* out[file] */
        if (starts_with(line, "out[")) {
            if (!main_started) {
                fprintf(out, "int main() {\n");
                main_started = 1;
            }

            char file[256], fmt[1024];
            sscanf(line, "out[%255[^]]]: %1023[^;];", file, fmt);
            trim(file); trim(fmt);

            fprintf(out,
                "{ FILE *_fp = fopen(\"%s\", \"w\"); fprintf(_fp, %s); fclose(_fp); }\n",
                file, fmt
            );
            continue;
        }

        /* START main() */
        if (!main_started &&
            !starts_with(line, "add ") &&
            !strstr(line, ") -> ")) {
            fprintf(out, "int main() {\n");
            main_started = 1;
        }

        /* inline C */
        if (starts_with(line, "inline_c {")) {
            fprintf(out, "/* inline C start */\n");
            while (fgets(line, sizeof(line), in)) {
                if (strstr(line, "}")) break;
                fprintf(out, "%s", line);
            }
            fprintf(out, "/* inline C end */\n");
            continue;
        }

        /* exit */
        if (starts_with(line, "exit ")) {
            char num[64];
            sscanf(line, "exit %63[^;];", num);
            fprintf(out, "return %s;\n", num);
            continue;
        }

        /* var name: type = expr */
        if (starts_with(line, "var ")) {
            char name[128], type[128], expr[1024];
            sscanf(line, "var %127[^:]: %127[^=]= %1023[^;];",
                   name, type, expr);

            trim(name); trim(type); trim(expr);

            char ctype[64];

            if      (strcmp(type, "int") == 0)        strcpy(ctype, "int");
            else if (strcmp(type, "float") == 0)      strcpy(ctype, "float");
            else if (strcmp(type, "string") == 0)     strcpy(ctype, "char*");
            else if (strcmp(type, "string*") == 0)    strcpy(ctype, "char**");
            else if (strcmp(type, "bool") == 0)       strcpy(ctype, "bool");
            else if (strcmp(type, "array") == 0)      strcpy(ctype, "array*");
            else strcpy(ctype, type);

            fprintf(out, "%s %s = %s;\n", ctype, name, expr);

            add_var(name, ctype);
            continue;
        }

        /* echo(x) */
        if (starts_with(line, "echo(")) {
            char inside[1024];
            int s = strlen("echo(");
            int e = strlen(line) - 2;

            strncpy(inside, line + s, e - s);
            inside[e - s] = 0;
            trim(inside);

            /* literal string */
            if (is_string_literal(inside)) {
                fprintf(out, "printf(\"%%s\\n\", %s);\n", inside);
                continue;
            }

            /* type lookup */
            const char *t = get_type(inside);

            if (t && strcmp(t, "char*") == 0)
                fprintf(out, "printf(\"%%s\\n\", %s);\n", inside);
            else if (t && strcmp(t, "char**") == 0)
                fprintf(out, "printf(\"%%s\\n\", %s[0]);\n", inside);
            else if (t && strcmp(t, "float") == 0)
                fprintf(out, "printf(\"%%f\\n\", %s);\n", inside);
            else
                fprintf(out, "printf(\"%%d\\n\", %s);\n", inside);

            continue;
        }

        /* ternary */
        if (strstr(line, "?") && strstr(line, ":")) {
            fprintf(out, "%s\n", line);
            continue;
        }

        /* constant folding */
        {
            int a,b; char op;
            if (sscanf(line, "%d %c %d", &a, &op, &b) == 3) {
                int r = 0;
                if(op=='+') r=a+b;
                else if(op=='-') r=a-b;
                else if(op=='*') r=a*b;
                else if(op=='/') r=a/b;

                char tmp[64];
                sprintf(tmp, "%d", r);
                strcpy(line, tmp);
            }
        }

        /* fallback */
        fprintf(out, "%s\n", line);
    }

    if (main_started)
        fprintf(out, "return 0;\n}\n");

    fclose(in);
    fclose(out);
    return 0;
}

