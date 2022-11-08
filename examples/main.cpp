#ifdef LOCAL
#define _GLIBCXX_DEBUG
#endif

#include <iostream>
#include "llp.h"
#include "database.h"

int main(int argc, char *argv[]) {
    std::string s = "suka";
    if (argc <= 1) {
        std::cout << "Usage: ./example path/to/file";
        return 0;
    }
    database db = database(argv[1], true);

    db.query({CREATE_SCHEMA, query_payload(
            create_schema_query(
                    "BOM-BOM",
                    {
                            {"pole", DB_INT_32}
                    }
            )
    )});


    char lol[100];
    debug(db.file->read(lol, 100, 0));
    debug(feof(db.file->file));
    debug(lol);
    return 0;
}
