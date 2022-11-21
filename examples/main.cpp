#ifdef LOCAL
#define _GLIBCXX_DEBUG
#endif

#include <iostream>
#include "database.h"

int main(int argc, char *argv[]) {
    std::string s = "suka";
    if (argc <= 1) {
        std::cout << "Usage: ./example path/to/file";
        return 0;
    }

#ifdef DEBUG 
    database db = database(argv[1], true);
#else
    database<file_in_memory_interface> db = database<file_in_memory_interface>(argv[1], true);
#endif

    db.query({CREATE_SCHEMA, query_payload(
            create_schema_query(
                    "BOM-BOM",
                    {
                            {"pole",      DB_INT_32},
                            {"bool_pole", DB_BOOL},
                    }
            )
    )});
    for (auto now: db.get_schemas()) {
        
    }

    char lol[100];
    debug(db.file->read(lol, 100, 0));
    debug(feof(db.file->fd));
    debug(lol);
    return 0;
}
