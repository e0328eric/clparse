#include <stdio.h>

#define DRAPEAU_IMPL
#include <drapeau.h>

// blabla clean all

int main(int argc, char** argv)
{
    const char* drapeau_err;
    drapeauStart("testing", "A test program for testing drapeau");

    bool* main_help = drapeauBool("help", false, NULL, NULL);
    bool* fab = drapeauBool("fab", false, "fas is great!", NULL);

    __attribute__((unused)) bool* is_install =
        drapeauSubcmd("install", "installing a programs");

    bool* install_help = drapeauBool(
        "help", false, "help message for install subcommand", "install");
    const char** build_dir =
        drapeauStr("build", "./build", "building adiada", "install");

    drapeauParse(argc, argv);
    if ((drapeau_err = drapeauPrintErr()) != NULL)
    {
        fprintf(stderr, "%s\n", drapeau_err);
        drapeauClose();
        return 1;
    }

    if (*main_help || *install_help)
    {
        drapeauPrintHelp(stdout);
        drapeauClose();
        return 0;
    }

    if (*fab)
    {
        printf("Hello World!\n");
    }
    else if (build_dir)
    {
        printf("%s\n", *build_dir);
    }

    drapeauClose();
}
