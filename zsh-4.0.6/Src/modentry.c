#include "zsh.mdh"

int setup_ _((Module));
int boot_ _((Module));
int cleanup_ _((Module));
int finish_ _((Module));
int modentry _((int boot, Module m));

/**/
int
modentry(int boot, Module m)
{
    switch (boot) {
    case 0:
	return setup_(m);
	break;

    case 1:
	return boot_(m);
	break;

    case 2:
	return cleanup_(m);
	break;

    case 3:
	return finish_(m);
	break;

    default:
	zerr("bad call to modentry", NULL, 0);
	return 1;
	break;
    }
}
