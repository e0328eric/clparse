#include <stdio.h>

#define DRAPEAU_IMPL
#include <drapeau.h>

int main(int argc, char** argv)
{
    drapeauStart("foo", "asdasd");
    bool* fooo = drapeauBool(NO_LONG, 'F', false, "asdasdas", NO_SUBCMD);
    drapeauParse(argc, argv);
    drapeauClose();

    if (drapeauGetErr() != NULL)
    {
        printf("%s\n", drapeauGetErr());
        return 1;
    }

    if (drapeauIsHelp())
    {
        drapeauPrintHelp(stdout);
        return 0;
    }

    printf("%s\n", *fooo ? "true" : "false");

    return 0;
}
