#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "include/cli.h"
#include "include/version.h"
#include "include/utils.h"

#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"

typedef struct {
    char name[256];
    char path[256];
} Project;

int runProject() {
    return 1;
}

int migrate() {
    // maybe store the path to the database (*.db) in lavandula.yml
    // since the CLI tool technically wont be able to access it, kinda
    system("sqlite3 todo.db < output.sql");

    return 0;
}

int help() {
    printf("Lavandula CLI\n");
    printf("Usage:\n");
    printf("  lavu new <project_name>   Create a new Lavandula project\n");
    printf("  lavu run                  Run the Lavandula project\n");
    printf("  lavu migrate              Create a new database migration\n");
    printf("  lavu help                 Show this help message\n");
    
    return 0;
}

int version() {
    printf("Lavandula version %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    return 0;
}

int unknownCommand(char *option) {
    printf(RED "Error: unknown option '%s'\n" RESET, option);
    printf("Use 'lavu help' to see available commands.\n");

    return 1;
}

int createDir(const char *path) {
    if (mkdir(path, 0755) != 0) {
        perror(path);
        return 0;
    }
    return 1;
}

int createFileWithContent(const char *filepath, const char *content) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror(filepath);
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    printf(GREEN "-> Created %s\n" RESET, filepath);
    return 1;
}

int createYamlFile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/lavandula.yml", project->path);

    char content[256];
    snprintf(content, sizeof(content), "name: %s\nversion: 1.0.0", project->name);

    return createFileWithContent(filepath, content);
}

int createAppFile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/app/app.c", project->path);
    const char *content =
        "#include \"../../src/lavandula.h\"\n"
        "#include \"routes.c\"\n\n"
        "int main() {\n"
        "    AppBuilder builder = createBuilder();\n"
        "    App app = build(builder);\n\n"
        "    registerRoutes(app);\n\n"
        "    runApp(&app);\n"
        "    cleanupApp(&app);\n\n"
        "    return 0;\n"
        "}\n";

    return createFileWithContent(filepath, content);
}

int createHomeFile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/app/controllers/home.c", project->path);
    const char *content =
        "#include \"../../../src/lavandula.h\"\n\n"
        "HttpResponse home(HttpRequest _) {\n"
        "    return ok(\"Hello, World!\");\n"
        "}\n";

    return createFileWithContent(filepath, content);
}

int createRoutesFile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/app/routes.c", project->path);
    const char *content =
        "#include \"../../src/lavandula.h\"\n"
        "#include \"controllers/home.c\"\n\n"
        "void registerRoutes(App app) {\n"
        "    root(&app, home);\n"
        "}\n";

    return createFileWithContent(filepath, content);
}

int createMakefile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/makefile", project->path);
    const char *content =
        "SRCS = app/app.c app/routes.c $(wildcard app/controllers/*.c)\n"
        "CFLAGS = -Wall -Wextra\n\n"
        "all:\n"
        "\tgcc $(SRCS) $(CFLAGS) -o a\n";

    return createFileWithContent(filepath, content);
}

int createTestsFile(Project *project) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/tests/tests.c", project->path);
    const char *content =
        "#include \"../src/lavandula.h\"\n\n"
        "void aTest() {\n"
        "   int x = 10;\n"
        "   expect(x, toBe(10));\n"
        "}\n\n"
        "void runTests() {\n"
        "   runTest(aTest);\n"
        "   // ..\n"
        "}\n\n"
        "// call 'runTests' in main.c";

    return createFileWithContent(filepath, content);
}

int newProject(char *name) {
    if (!name || strlen(name) == 0) {
        printf(RED "Error: Project name cannot be empty.\n" RESET);
        return 1;
    }

    Project project;
    snprintf(project.name, sizeof(project.name), "%s", name);
    snprintf(project.path, sizeof(project.path), "%s", name);

    printf(YELLOW "\nSetting up Lavandula project '%s'...\n\n" RESET, project.name);

    if (!createDir(project.path)) return 1;

    char appDir[256], controllersDir[256], testsDir[256];
    snprintf(appDir, sizeof(appDir), "%s/app", project.path);
    snprintf(controllersDir, sizeof(controllersDir), "%s/app/controllers", project.path);
    snprintf(testsDir, sizeof(testsDir), "%s/tests", project.path);

    if (!createDir(appDir)) return 1;
    if (!createDir(controllersDir)) return 1;
    if (!createDir(testsDir)) return 1;

    if (!createYamlFile(&project)) return 1;
    if (!createAppFile(&project)) return 1;
    if (!createHomeFile(&project)) return 1;
    if (!createRoutesFile(&project)) return 1;
    if (!createMakefile(&project)) return 1;
    if (!createTestsFile(&project)) return 1;

    printf(GREEN "\n🎉 Lavandula project '%s' setup finished successfully!\n" RESET, project.name);
    printf(YELLOW "\nNext steps:\n" RESET);
    printf("  1. cd %s\n", project.path);
    printf("  2. lavu run\n\n");

    return 0;
}