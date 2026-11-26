/*
 * Forced first include for SigmaStar's disp, panel and rgn sources built into
 * open_mhal.ko. Their EXPORT_SYMBOLs become no-ops: the vendor mhal object
 * already exports every one of those names from its own __ksymtab, and the
 * linker points those entries at the source definitions (scripts/weaken.sh),
 * so a second export would only make modpost fail on a duplicate.
 */
#include <linux/export.h>

#undef EXPORT_SYMBOL
#undef EXPORT_SYMBOL_GPL
#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)
