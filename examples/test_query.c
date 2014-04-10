#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "evhtp.h"

struct test {
    const char * raw_query;
    struct expected {
        char * key;
        char * val;
    } exp[10]; /* avoid flexible array member: limit expectations per raw_query */
};

static int
test_cmp(evhtp_query_t * query, evhtp_kv_t * kvobj, const char * valstr, struct expected * exp) {
    if (!query || !kvobj) {
        return -1;
    }

    if (exp->val == NULL) {
        if (kvobj->val || valstr) {
            return -1;
        }

        return 0;
    }

    if (strcmp(kvobj->val, exp->val)) {
        printf("  expected: '%s'\n", exp->val);
        printf("  actual:   '%s'\n", kvobj->val);
        return -1;
    }

    if (strcmp(valstr, exp->val)) {
        return -1;
    }

    return 0;
}

static int
query_test(const char * raw_query, struct expected exp[]) {
    evhtp_query_t   * query;
    struct expected * check;
    int               idx        = 0;
    int               num_errors = 0;

    if (!(query = evhtp_parse_query(raw_query, strlen(raw_query)))) {
        return -1;
    }

    while (1) {
        evhtp_kv_t * kvobj  = NULL;
        const char * valstr = NULL;

        check = &exp[idx++];

        if (check == NULL || check->key == NULL) {
            break;
        }

        kvobj  = evhtp_kvs_find_kv(query, check->key);
        valstr = evhtp_kv_find(query, check->key);

        if (test_cmp(query, kvobj, valstr, check) == -1) {
            num_errors += 1;
        }
    }

    return num_errors;
}

struct test tests[] = {
    { "notp&ifp=&othernotp;thenp=;key=val", {
        { "notp",      NULL  },
        { "ifp",       ""    },
        { "othernotp", NULL  },
        { "thenp",     ""    },
        { "key",       "val" },
        { NULL,        NULL  }
    }},
    { "foo=bar;baz=raz&a=1", {
        { "foo", "bar" },
        { "baz", "raz" },
        { "a",   "1"   },
        { NULL,  NULL  }
    }},
    { "end_empty_string=", {
        { "end_empty_string", "" },
        { NULL,            NULL  }
    }},
    { "end_null", {
        { "end_null", NULL },
        { NULL,       NULL }
    }},
    { "hexa=some%20;hexb=bla%0&hexc=%", {
        { "hexa", "some%20" },
        { "hexb",   "bla%0" },
        { "hexc",       "%" },
        { NULL,       NULL  }
    }}
};

static void
test(const char * raw_query, struct expected exp[]) {
    printf("%-50s %s\n", raw_query, query_test(raw_query, exp) ? "ERROR" : "OK");
}

int
main(int argc, char ** argv) {
    int i;

    for (i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i)
        test(tests[i].raw_query, tests[i].exp);

    return 0;
}
