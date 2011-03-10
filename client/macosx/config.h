#ifndef CONFIG_H_
#define CONFIG_H_

/* Special version of config.h for our Xcode Mac build - Xcode doesn't do the
 * whole "configure" thing, so we might want to set a few #defines ourselves
 */

#ifdef __APPLE__

#define HAVE_CEGUI_07 1

#endif

#endif
