#ifndef HANDLEMAIN_H
#define HANDLEMAIN_H

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#include "General/system.h"

#include "storyboard/RGBvideorender.h"

/* end include object headers*/

/*Global Vars*/
extern int debug;

/* End Globals */

/* Here are the function prototypes */
void printc(char *sz,...);
extern char string[256];
/* End Global function prototypes */


#endif /* HANDLEMAIN_H */
