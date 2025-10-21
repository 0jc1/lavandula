#include "include/lavandula.h"

appRoute(a, ctx) {
    return ok("This is route A", TEXT_PLAIN);
}

appRoute(b, ctx) {
    return ok("This is route B", TEXT_PLAIN);
}

int main() {
    AppBuilder builder = createBuilder();
    App app = build(builder);

    get(&app, "/a", a);
    get(&app, "/b", b);

    runApp(&app);

    return 0;

    // if (argc < 2) {
    //     printf("usage: lavu <command> [options]\n");
    //     return 1;
    // }

    // char *option = argv[1];

    // if (strcmp(option, "new") == 0) {
    //     if (argc < 3) {
    //         printf("error: expected project name after 'new'\n");
    //         return 1;
    //     }

    //     char *projectName = argv[2];
    //     return newProject(projectName);
    // } else if (strcmp(option, "run") == 0) {
    //     return runProject();
    // } else if (strcmp(option, "build") == 0) {
    //     return buildProject();
    // } else if (strcmp(option, "help") == 0) {
    //     return help();
    // } else if (strcmp(option, "--version") == 0 || strcmp(option, "-v") == 0) {
    //     return version();
    // } else {
    //     return unknownCommand(option);
    // }
}