/*
 * Ifliegen (SSC034A/B) MI modules: the SDK's sdk/linux/module.lds, with
 * which link_modules.mk relinks each mi_<x>.o with --gc-sections, roots
 * init_module and cleanup_module. link_modules.mk then adds one
 * KEEP(*(___ksymtab+<sym>)) per symbol in the first pass's Module.symvers;
 * keeping every ___ksymtab+* and ___ksymtab_gpl+* input section here keeps
 * the same set in one pass.
 */
SECTIONS {
    /DISCARD/ : {
        *(.discard)
        *(.discard.*)
        *(.eh_frame)
    }
    .ARM.exidx : {
        *(.ARM.exidx)
        *(.ARM.exidx.*)
    }
    .ARM.extab : {
        *(.ARM.extab)
        *(.ARM.extab.*)
    }
    .bss : {
        *(.bss .bss.[0-9a-zA-Z_]*)
        *(.bss..L*)
    }
    .data : {
        *(.data .data.[0-9a-zA-Z_]*)
        *(.data..L*)
    }
    .rodata : {
        *(.rodata.._start)
        *(.rodata .rodata.[0-9a-zA-Z_]*)
        *(.rodata..L*)
        *(.rodata.._end)
    }
    .text : {
        *(.text.._start)
        *(.text .text.[0-9a-zA-Z_]*)
        *(.text.._end)
    }
    .text.unlikely : {
        *(.text.unlikely .text.unlikely.[0-9a-zA-Z_]*)
    }
    .modinfo : {
        KEEP(*(.modinfo))
    }
    __param : {
        KEEP(*(__param))
    }
    .pv_table : {
        KEEP(*(.pv_table))
    }
}
SECTIONS {
    __ksymtab : {
        KEEP(*(SORT(___ksymtab+*)))
    }
    __ksymtab_gpl : {
        KEEP(*(SORT(___ksymtab_gpl+*)))
    }
}
