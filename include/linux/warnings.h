/*
      6 -Wunused-but-set-parameter]
     14 -Wold-style-definition]
     16 -Wvla-larger-than=]
     26 -Wstrict-overflow]
     57 -Wundef]
    145 -Wfloat-equal]
    157 -Wframe-address]
    190 -Wjump-misses-init]
    232 -Wlogical-op]
    304 -Wold-style-declaration]
    307 -Wframe-larger-than=]
    319 -Wignored-qualifiers]
    393 -Wmaybe-uninitialized]
    623 -Wsuggest-attribute=format]
    647 -Wstack-usage=]
    671 -Wstringop-truncation]
    711 -Wcast-function-type]
    743 -Wvector-operation-performance]
    984 -Wformat-overflow=]
    997 -Wsuggest-attribute=noreturn]
   1013 -Wformat-security]
   1500 -Wempty-body]
   1972 -Wdouble-promotion]
   2784 -Wformat-nonliteral]
   4332 -Wduplicated-cond]
   5171 -Wfloat-conversion]
   6576 -Wformat-truncation=]
   6697 -Wstack-protector]
   7826 -Woverlength-strings]
   7969 -Wunused-but-set-variable]
  11801 -Wduplicated-branches]
  15989 -Wuninitialized]
  19518 -Wmissing-prototypes]
  21354 -Woverride-init]
  25315 -Wtype-limits]
  28348 -Wsuggest-attribute=const]
  35718 -Wsuggest-attribute=cold]
  35789 -Wunsuffixed-float-constants]
  37048 -Waggregate-return]
  40664 -Wimplicit-fallthrough=]
  43685 -Wunused-const-variable=]
  68675 -Wshift-overflow=]
  79124 -Wsuggest-attribute=pure]
 184929 -Wsuggest-attribute=malloc]
 285910 -Wpointer-sign]
 305056 -Wunused-macros]
 550802 -Wformat=]
 619302 -Wshadow]
 621102 -Wswitch-default]
 854764 -Wcast-align]
1075660 -Wmissing-field-initializers]
1141263 -Wlarger-than=]
1171275 -Winline]
1266195 -Wsign-compare]
2241214 -Wswitch-enum]
4828168 -Wpacked]
5915026 -Wredundant-decls]
*/

/* default-enabled warnings */
KBUILD_WARN(0, GCC_4_6, "-Wattributes")
KBUILD_WARN(0, GCC_4_6, "-Wbuiltin-macro-redefined")
KBUILD_WARN(0, GCC_4_6, "-Wcoverage-mismatch")
KBUILD_WARN(0, GCC_4_6, "-Wcpp")
KBUILD_WARN(0, GCC_4_6, "-Wdeclaration-after-statement")
KBUILD_WARN(0, GCC_4_6, "-Wdeprecated-declarations")
KBUILD_WARN(0, GCC_4_6, "-Wdeprecated")
KBUILD_WARN(0, GCC_4_6, "-Wdiv-by-zero")
KBUILD_WARN(0, GCC_4_6, "-Wendif-labels")
KBUILD_WARN(0, GCC_4_6, "-Wenum-compare")
KBUILD_ERROR(GCC_4_6, "-Wimplicit-function-declaration")
KBUILD_WARN(0, GCC_4_6, "-Wimplicit-int")
KBUILD_WARN(0, GCC_4_6, "-Wint-to-pointer-cast")
KBUILD_WARN(4, GCC_4_6, "-Wlong-long") /* harmful */
KBUILD_WARN(0, GCC_4_6, "-Wmain")
KBUILD_WARN(0, GCC_4_6, "-Woverflow")
KBUILD_WARN(0, GCC_4_6, "-Wpacked-bitfield-compat")
KBUILD_WARN(0, GCC_4_6, "-Wpointer-to-int-cast")
KBUILD_WARN(1, GCC_4_6, "-Wpragmas")
KBUILD_WARN(1, GCC_4_6, "-Wpsabi") /* rare */
KBUILD_WARN(0, GCC_4_6, "-Wreturn-type")
KBUILD_WARN(0, GCC_4_6, "-Wsync-nand")
KBUILD_WARN(0, GCC_4_6, "-Wtrigraphs") /* never */
KBUILD_WARN(0, GCC_4_6, "-Wnormalized=nfc")
KBUILD_WARN(0, GCC_4_7, "-Wfree-nonheap-object")
KBUILD_WARN(0, GCC_4_7, "-Winvalid-memory-model")
KBUILD_WARN(0, GCC_4_7, "-Wnarrowing")
KBUILD_WARN(0, GCC_4_8, "-Waggressive-loop-optimizations")
KBUILD_WARN(0, GCC_4_8, "-Wreturn-local-addr")
KBUILD_WARN(0, GCC_4_8, "-Wvarargs")
KBUILD_WARN(4, GCC_5, "-Wc90-c99-compat")
KBUILD_WARN(4, GCC_5, "-Wc99-c11-compat") /* harmful */
KBUILD_WARN(0, GCC_5, "-Wdesignated-init")
KBUILD_WARN(0, GCC_5, "-Wdiscarded-array-qualifiers")
KBUILD_WARN(0, GCC_5, "-Wdiscarded-qualifiers")
KBUILD_WARN(0, GCC_5, "-Wincompatible-pointer-types")
KBUILD_WARN(0, GCC_5, "-Wint-conversion")
KBUILD_WARN(0, GCC_5, "-Wodr")
KBUILD_WARN(0, GCC_5, "-Wshift-count-negative")
KBUILD_WARN(0, GCC_5, "-Wshift-count-overflow")
KBUILD_WARN(0, GCC_5, "-Wsizeof-array-argument")
KBUILD_WARN(0, GCC_5, "-Wswitch-bool")
KBUILD_WARN(0, GCC_6, "-Whsa")
KBUILD_WARN(0, GCC_6, "-Wignored-attributes")
KBUILD_WARN(0, GCC_6, "-Wlto-type-mismatch")
KBUILD_WARN(0, GCC_6, "-Woverride-init-side-effects")
KBUILD_WARN(0, GCC_6, "-Wscalar-storage-order")
KBUILD_WARN(1, GCC_6, "-Wshift-negative-value")
KBUILD_WARN(0, GCC_6, "-Wshift-overflow=1")
KBUILD_WARN(4, GCC_6, "-Wshift-overflow=2") /* lots */
KBUILD_WARN(0, GCC_7, "-Wbuiltin-declaration-mismatch")
KBUILD_WARN(0, GCC_7, "-Wpointer-compare")
KBUILD_WARN(0, GCC_7, "-Wstringop-overflow=2")
KBUILD_WARN(3, GCC_7, "-Wstringop-overflow=4") /* never */
KBUILD_WARN(0, GCC_7, "-Wswitch-unreachable")
KBUILD_WARN(0, GCC_8, "-Wattribute-alias")
KBUILD_WARN(0, GCC_8, "-Wif-not-aligned")
KBUILD_WARN(1, GCC_9, "-Waddress-of-packed-member") /* lots */
KBUILD_WARN(1, CLANG_8, "-Waddress-of-packed-member") /* lots */
KBUILD_WARN(0, CLANG_8, "-W#pragma-messages")
KBUILD_WARN(0, CLANG_8, "-W#warnings")
KBUILD_WARN(0, CLANG_8, "-Wabsolute-value")
KBUILD_WARN(0, CLANG_8, "-Wabstract-final-class")
KBUILD_WARN(0, CLANG_8, "-Waddress")
KBUILD_WARN(0, CLANG_8, "-Wstring-compare")
KBUILD_WARN(0, CLANG_8, "-Wtautological-pointer-compare") /* rare */
KBUILD_WARN(0, CLANG_8, "-Waddress-of-temporary")
KBUILD_WARN(0, CLANG_8, "-Wambiguous-macro")
KBUILD_WARN(0, CLANG_8, "-Wambiguous-member-template")
KBUILD_WARN(0, CLANG_8, "-Warc")
KBUILD_WARN(0, CLANG_8, "-Warc-non-pod-memaccess")
KBUILD_WARN(0, CLANG_8, "-Warc-retain-cycles")
KBUILD_WARN(0, CLANG_8, "-Warc-unsafe-retained-assign")
KBUILD_WARN(0, CLANG_8, "-Warc-repeated-use-of-weak")
KBUILD_WARN(0, CLANG_8, "-Warc-maybe-repeated-use-of-weak")
KBUILD_WARN(0, CLANG_8, "-Warray-bounds")
KBUILD_WARN(0, CLANG_8, "-Wasm")
//KBUILD_WARN(0, CLANG_8, "-Wasm-ignored-qualifier")
KBUILD_WARN(0, CLANG_8, "-Wasm-operand-widths")
KBUILD_WARN(0, CLANG_8, "-Watimport-in-framework-header")
KBUILD_WARN(0, CLANG_8, "-Wattributes")
KBUILD_WARN(0, CLANG_8, "-Wignored-attributes")
KBUILD_WARN(0, CLANG_8, "-Wunknown-attributes")
KBUILD_WARN(0, CLANG_8, "-Wauto-disable-vptr-sanitizer")
KBUILD_WARN(0, CLANG_8, "-Wavailability")
KBUILD_WARN(0, CLANG_8, "-Wbackend-plugin")
KBUILD_WARN(0, CLANG_8, "-Wblock-capture-autoreleasing")
KBUILD_WARN(0, CLANG_8, "-Wbool-conversion")
KBUILD_WARN(0, CLANG_8, "-Wpointer-bool-conversion")
KBUILD_WARN(0, CLANG_8, "-Wundefined-bool-conversion")
KBUILD_WARN(0, CLANG_8, "-Wbridge-cast")
KBUILD_WARN(0, CLANG_8, "-Wbuiltin-macro-redefined")
KBUILD_WARN(0, CLANG_8, "-Wbuiltin-requires-header")
KBUILD_WARN(0, CLANG_8, "-Wclang-cl-pch")
KBUILD_WARN(0, CLANG_8, "-Wcompare-distinct-pointer-types")
KBUILD_WARN(0, CLANG_8, "-Wconfig-macros")
KBUILD_WARN(0, CLANG_8, "-Wctu")
KBUILD_WARN(0, CLANG_8, "-Wdangling")
KBUILD_WARN(0, CLANG_8, "-Wdangling-field")
KBUILD_WARN(0, CLANG_8, "-Wdangling-initializer-list")
KBUILD_WARN(0, CLANG_8, "-Wreturn-stack-address")
KBUILD_WARN(0, CLANG_8, "-Wdealloc-in-category")
KBUILD_WARN(0, CLANG_8, "-Wdelete-incomplete")
KBUILD_WARN(0, CLANG_8, "-Wdistributed-object-modifiers")
KBUILD_WARN(0, CLANG_8, "-Wdivision-by-zero")
KBUILD_WARN(0, CLANG_8, "-Wempty-body")
KBUILD_WARN(0, CLANG_8, "-Wextra-tokens")
KBUILD_WARN(0, CLANG_8, "-Wenum-compare")
KBUILD_WARN(0, CLANG_8, "-Wenum-compare-switch")
KBUILD_WARN(0, CLANG_8, "-Wenum-too-large")
//KBUILD_WARN(0, CLANG_8, "-Wexperimental-isel")
KBUILD_WARN(0, CLANG_8, "-Wexplicit-initialize-call")
KBUILD_WARN(0, CLANG_8, "-Wflag-enum")
KBUILD_WARN(0, CLANG_8, "-Wflexible-array-extensions")
KBUILD_WARN(0, CLANG_8, "-Wfunction-multiversion")
KBUILD_WARN(3, CLANG_8, "-Wgcc-compat")
KBUILD_WARN(0, CLANG_8, "-Wimplicit-conversion-floating-point-to-bool")
KBUILD_WARN(0, CLANG_8, "-Wimplicitly-unsigned-literal")
KBUILD_WARN(0, CLANG_8, "-Wincompatible-exception-spec")
KBUILD_WARN(0, CLANG_8, "-Wincompatible-ms-struct")
KBUILD_WARN(0, CLANG_8, "-Wincompatible-pointer-types")
KBUILD_WARN(0, CLANG_8, "-Wincompatible-function-pointer-types")
KBUILD_WARN(0, CLANG_8, "-Wincompatible-pointer-types-discards-qualifiers")
KBUILD_WARN(0, CLANG_8, "-Wincomplete-framework-module-declaration")
KBUILD_WARN(0, CLANG_8, "-Wincomplete-module")
KBUILD_WARN(0, CLANG_8, "-Wincomplete-umbrella")
KBUILD_WARN(0, CLANG_8, "-Wnon-modular-include-in-module")
KBUILD_WARN(0, CLANG_8, "-Wnon-modular-include-in-framework-module")
KBUILD_WARN(0, CLANG_8, "-Winconsistent-missing-override")
KBUILD_WARN(0, CLANG_8, "-Wincrement-bool")
KBUILD_WARN(0, CLANG_8, "-Wdeprecated-increment-bool")
KBUILD_WARN(0, CLANG_8, "-Winline-asm")
KBUILD_WARN(0, CLANG_8, "-Wint-to-pointer-cast")
KBUILD_WARN(0, CLANG_8, "-Wint-to-void-pointer-cast")
KBUILD_WARN(0, CLANG_8, "-Winvalid-command-line-argument")
KBUILD_WARN(2, CLANG_8, "-Wignored-optimization-argument")
KBUILD_WARN(0, CLANG_8, "-Winvalid-iboutlet")
KBUILD_WARN(0, CLANG_8, "-Winvalid-noreturn")
KBUILD_WARN(0, CLANG_8, "-Winvalid-offsetof")
KBUILD_WARN(0, CLANG_8, "-Winvalid-pp-token")
KBUILD_WARN(0, CLANG_8, "-Winvalid-source-encoding")
KBUILD_WARN(0, CLANG_8, "-Wkeyword-compat")
KBUILD_WARN(0, CLANG_8, "-Wknr-promoted-parameter")
KBUILD_WARN(0, CLANG_8, "-Wlarge-by-value-copy")
KBUILD_WARN(0, CLANG_8, "-Wmacro-redefined")
KBUILD_WARN(0, CLANG_8, "-Wmain-return-type")
KBUILD_WARN(0, CLANG_8, "-Wmalformed-warning-check")
KBUILD_WARN(0, CLANG_8, "-Wmax-unsigned-zero")
KBUILD_WARN(0, CLANG_8, "-Wmismatched-parameter-types")
KBUILD_WARN(0, CLANG_8, "-Wmismatched-return-types")
KBUILD_WARN(1, CLANG_8, "-Wmissing-declarations") /* never */
KBUILD_WARN(1, CLANG_8, "-Wmissing-noreturn")
KBUILD_WARN(0, CLANG_8, "-Wmodule-conflict")
KBUILD_WARN(0, CLANG_8, "-Wmodule-file-extension")
KBUILD_WARN(0, CLANG_8, "-Wnewline-eof")
KBUILD_WARN(0, CLANG_8, "-Wnoderef")
KBUILD_WARN(0, CLANG_8, "-Wnsconsumed-mismatch")
KBUILD_WARN(0, CLANG_8, "-WNSObject-attribute")
KBUILD_WARN(0, CLANG_8, "-Wnsreturns-mismatch")
KBUILD_WARN(0, CLANG_8, "-Wnull-arithmetic")
KBUILD_WARN(0, CLANG_8, "-Wnull-character")
KBUILD_WARN(0, CLANG_8, "-Wnull-dereference")
KBUILD_WARN(0, CLANG_8, "-Wnullability")
KBUILD_WARN(0, CLANG_8, "-Wnullability-completeness")
KBUILD_WARN(0, CLANG_8, "-Wnullability-completeness-on-arrays")
KBUILD_WARN(0, CLANG_8, "-Wnullability-declspec")
KBUILD_WARN(0, CLANG_8, "-Wnullability-inferred-on-nested-type")
KBUILD_WARN(0, CLANG_8, "-Wold-style-cast")
KBUILD_WARN(0, CLANG_8, "-Woption-ignored")
KBUILD_WARN(0, CLANG_8, "-Wout-of-line-declaration")
KBUILD_WARN(1, CLANG_8, "-Winitializer-overrides") /* medium */
KBUILD_WARN(0, CLANG_8, "-Wunguarded-availability-new")
KBUILD_WARN(0, CLANG_8, "-Wpass-failed")
KBUILD_WARN(4, CLANG_8, "-Wpedantic")
KBUILD_WARN(1, CLANG_8, "-Wpointer-sign")  /* lots, but valuable */
KBUILD_WARN(0, CLANG_8, "-Wprofile-instr-out-of-date")
KBUILD_WARN(0, CLANG_8, "-Wprofile-instr-unprofiled")
KBUILD_WARN(0, CLANG_8, "-Wproperty-access-dot-syntax")
KBUILD_WARN(0, CLANG_8, "-Wproperty-attribute-mismatch")
KBUILD_WARN(0, CLANG_8, "-Wprotocol")
KBUILD_WARN(0, CLANG_8, "-Wregister")
KBUILD_WARN(0, CLANG_8, "-Wdeprecated-register")
KBUILD_WARN(0, CLANG_8, "-Wsentinel")
KBUILD_WARN(0, CLANG_8, "-Wunsequenced")
KBUILD_WARN(0, CLANG_8, "-Wserialized-diagnostics")
KBUILD_WARN(0, CLANG_8, "-Wstatic-float-init")
KBUILD_WARN(0, CLANG_8, "-Wstatic-local-in-inline")
KBUILD_WARN(0, CLANG_8, "-Wstring-plus-char")
KBUILD_WARN(0, CLANG_8, "-Wstrncat-size")
KBUILD_WARN(0, CLANG_8, "-Wsuspicious-memaccess")
KBUILD_WARN(0, CLANG_8, "-Wdynamic-class-memaccess")
KBUILD_WARN(0, CLANG_8, "-Wmemset-transposed-args")
KBUILD_WARN(0, CLANG_8, "-Wnontrivial-memaccess")
KBUILD_WARN(0, CLANG_8, "-Wsizeof-pointer-memaccess")
KBUILD_WARN(0, CLANG_8, "-Wsuspicious-bzero")
KBUILD_WARN(0, CLANG_8, "-Wtype-safety")
KBUILD_WARN(0, CLANG_8, "-Wunavailable-declarations")
KBUILD_WARN(0, CLANG_8, "-Wunicode")
KBUILD_WARN(0, CLANG_8, "-Wunknown-argument")
KBUILD_WARN(0, CLANG_8, "-Wunknown-sanitizers")
KBUILD_WARN(0, CLANG_8, "-Wunknown-warning-option")
KBUILD_WARN(0, CLANG_8, "-Wunsupported-abs")
KBUILD_WARN(0, CLANG_8, "-Wunsupported-cb")
KBUILD_WARN(0, CLANG_8, "-Wunsupported-gpopt")
KBUILD_WARN(0, CLANG_8, "-Wunsupported-nan")
KBUILD_WARN(0, CLANG_8, "-Wunsupported-target-opt")
KBUILD_WARN(0, CLANG_8, "-Wunused-command-line-argument")
KBUILD_WARN(0, CLANG_8, "-Wunused-getter-return-value")
KBUILD_WARN(0, CLANG_8, "-Wuser-defined-literals")
KBUILD_WARN(0, CLANG_8, "-Wvarargs")
KBUILD_WARN(0, CLANG_8, "-Wvexing-parse")
KBUILD_WARN(0, CLANG_8, "-Wvisibility")
KBUILD_WARN(0, CLANG_8, "-Wwritable-strings")
KBUILD_WARN(0, CLANG_8, "-Wdeprecated-writable-strings")

/* Enabled by -Wall */
KBUILD_WARN(0, GCC_4_6, "-Wall")
KBUILD_WARN(0, GCC_4_6, "-Waddress")
KBUILD_WARN(0, GCC_4_6, "-Warray-bounds")
KBUILD_WARN(0, GCC_4_6, "-Warray-bounds=1")
KBUILD_WARN(3, GCC_4_6, "-Warray-bounds=2") /* never */
KBUILD_WARN(1, GCC_9, "-Warray-bounds") /* false-positives in gcc-9.2 */
KBUILD_WARN(0, GCC_4_6, "-Wchar-subscripts")
KBUILD_WARN(0, GCC_4_6, "-Wcomment")
KBUILD_WARN(0, GCC_4_6, "-Wimplicit")
KBUILD_WARN(0, GCC_4_6, "-Wmissing-braces")
KBUILD_WARN(0, GCC_4_6, "-Wnonnull")
KBUILD_WARN(0, GCC_4_6, "-Wparentheses")
KBUILD_WARN(2, GCC_4_6, "-Wpointer-sign") /* lots, but valuable */
KBUILD_WARN(4, GCC_4_6, "-Wpointer-sign") /* lots, but valuable */
KBUILD_WARN(0, GCC_4_6, "-Wsequence-point")
KBUILD_WARN(0, GCC_4_6, "-Wswitch")
KBUILD_WARN(0, GCC_4_6, "-Wstrict-aliasing=3")
KBUILD_WARN(0, GCC_4_6, "-Wstrict-overflow=1")
KBUILD_WARN(3, GCC_4_6, "-Wstrict-overflow=5") /* rare */
KBUILD_WARN(0, GCC_4_6, "-Wuninitialized") /* medium */
KBUILD_WARN(0, GCC_4_6, "-Wunknown-pragmas")
KBUILD_WARN(0, GCC_4_6, "-Wvolatile-register-var")

#if defined(CONFIG_CC_DISABLE_WARN_MAYBE_UNINITIALIZED) || defined(CONFIG_GCOV_PROFILE_ALL)
KBUILD_WARN(1, GCC_4_7, "-Wmaybe-uninitialized")
#else
KBUILD_WARN(1, GCC_4_7, "-Wmaybe-uninitialized") /* medium */
#endif

KBUILD_WARN(0, GCC_4_8, "-Wsizeof-pointer-memaccess")
KBUILD_WARN(0, GCC_4_9, "-Wopenmp-simd")
KBUILD_WARN(0, GCC_5, "-Wbool-compare")
KBUILD_WARN(0, GCC_5, "-Wchkp")
KBUILD_WARN(0, GCC_5, "-Wlogical-not-parentheses")
KBUILD_WARN(0, GCC_5, "-Wmemset-transposed-args")
KBUILD_WARN(1, GCC_6, "-Wframe-address") /* rare */
KBUILD_WARN(0, GCC_6, "-Wmisleading-indentation")
KBUILD_WARN(0, GCC_6, "-Wnonnull-compare")
KBUILD_WARN(0, GCC_6, "-Wtautological-compare")
KBUILD_WARN(0, GCC_7, "-Wbool-operation")
KBUILD_WARN(0, GCC_7, "-Wdangling-else")
KBUILD_WARN(0, GCC_7, "-Wduplicate-decl-specifier")
KBUILD_WARN(0, GCC_7, "-Wint-in-bool-context")
KBUILD_WARN(0, GCC_7, "-Wmemset-elt-size")
KBUILD_WARN(0, GCC_7, "-Wrestrict")
KBUILD_WARN(1, GCC_10, "-Wrestrict")
KBUILD_WARN(0, GCC_8, "-Wmissing-attributes")
KBUILD_WARN(0, GCC_8, "-Wmultistatement-macros")
KBUILD_WARN(1, GCC_8, "-Wpacked-not-aligned")
KBUILD_WARN(0, GCC_8, "-Wsizeof-pointer-div")
KBUILD_WARN(1, GCC_8, "-Wstringop-truncation") /* medium */
KBUILD_WARN(0, CLANG_8, "-Wall")
KBUILD_WARN(0, CLANG_8, "-Wmost")
KBUILD_WARN(0, CLANG_8, "-Wcast-of-sel-type")
KBUILD_WARN(0, CLANG_8, "-Wchar-subscripts")
KBUILD_WARN(0, CLANG_8, "-Wcomment")
KBUILD_WARN(0, CLANG_8, "-Wextern-c-compat")
KBUILD_WARN(0, CLANG_8, "-Wfor-loop-analysis")
KBUILD_WARN(0, CLANG_8, "-Wimplicit")
KBUILD_ERROR(CLANG_8, "-Wimplicit-function-declaration")
KBUILD_WARN(0, CLANG_8, "-Wimplicit-int")
KBUILD_WARN(0, CLANG_8, "-Winfinite-recursion")
KBUILD_WARN(0, CLANG_8, "-Wmismatched-tags")
KBUILD_WARN(0, CLANG_8, "-Wmissing-braces")
KBUILD_WARN(0, CLANG_8, "-Wmove")
KBUILD_WARN(0, CLANG_8, "-Wpessimizing-move")
KBUILD_WARN(0, CLANG_8, "-Wredundant-move")
KBUILD_WARN(0, CLANG_8, "-Wreturn-std-move")
KBUILD_WARN(0, CLANG_8, "-Wself-move")
KBUILD_WARN(0, CLANG_8, "-Wmultichar")
KBUILD_WARN(0, CLANG_8, "-Wobjc-designated-initializers")
KBUILD_WARN(0, CLANG_8, "-Wobjc-flexible-array")
KBUILD_WARN(0, CLANG_8, "-Wobjc-missing-super-calls")
KBUILD_WARN(0, CLANG_8, "-Woverloaded-virtual")
KBUILD_WARN(0, CLANG_8, "-Wprivate-extern")
KBUILD_WARN(0, CLANG_8, "-Wreorder")
KBUILD_WARN(0, CLANG_8, "-Wreturn-type")
KBUILD_WARN(0, CLANG_8, "-Wreturn-type-c-linkage")
KBUILD_WARN(0, CLANG_8, "-Wself-assign")
KBUILD_WARN(0, CLANG_8, "-Wself-assign-field")
KBUILD_WARN(0, CLANG_8, "-Wself-assign-overloaded")
KBUILD_WARN(0, CLANG_8, "-Wsizeof-array-argument")
KBUILD_WARN(0, CLANG_8, "-Wsizeof-array-decay")
KBUILD_WARN(0, CLANG_8, "-Wstring-plus-int")
KBUILD_WARN(1, CLANG_8, "-Wtrigraphs") /* rare */
KBUILD_WARN(0, CLANG_8, "-Wuninitialized")
KBUILD_WARN(0, CLANG_8, "-Wsometimes-uninitialized")
KBUILD_WARN(0, CLANG_8, "-Wstatic-self-init")
KBUILD_WARN(0, CLANG_8, "-Wunknown-pragmas")

KBUILD_WARN(0, CLANG_8, "-Wuser-defined-warnings")
KBUILD_WARN(0, CLANG_8, "-Wvolatile-register-var")
KBUILD_WARN(0, CLANG_8, "-Wparentheses")
KBUILD_WARN(0, CLANG_8, "-Wbitwise-op-parentheses")
KBUILD_WARN(0, CLANG_8, "-Wdangling-else")
KBUILD_WARN(0, CLANG_8, "-Wlogical-not-parentheses")
KBUILD_WARN(0, CLANG_8, "-Wlogical-op-parentheses")
KBUILD_WARN(0, CLANG_8, "-Woverloaded-shift-op-parentheses")
KBUILD_WARN(0, CLANG_8, "-Wparentheses-equality")
KBUILD_WARN(0, CLANG_8, "-Wshift-op-parentheses")
KBUILD_WARN(0, CLANG_8, "-Wswitch")
KBUILD_WARN(0, CLANG_8, "-Wswitch-bool")

/* printf format strings */
KBUILD_WARN(0, GCC_4_6, "-Wformat=1")
KBUILD_WARN(0, GCC_4_6, "-Wformat-contains-nul")
KBUILD_WARN(0, GCC_4_6, "-Wformat-extra-args")
KBUILD_WARN(2, GCC_4_6, "-Wformat-zero-length")
KBUILD_WARN(1, GCC_7, "-Wformat-truncation=1")
KBUILD_WARN(4, GCC_4_6, "-Wformat=2") /* huge */
KBUILD_WARN(3, GCC_4_6, "-Wformat-nonliteral") /* medium */
KBUILD_WARN(3, GCC_4_6, "-Wformat-security") /* medium */
KBUILD_WARN(3, GCC_4_6, "-Wformat-y2k")
KBUILD_WARN(3, GCC_5, "-Wformat-signedness")
KBUILD_WARN(1, GCC_7, "-Wformat-overflow=1")
KBUILD_WARN(3, GCC_7, "-Wformat-overflow=2") /* medium */
KBUILD_WARN(4, GCC_7, "-Wformat-truncation=2")
KBUILD_WARN(1, CLANG_8, "-Wformat")
KBUILD_WARN(0, CLANG_8, "-Wformat-extra-args")
KBUILD_WARN(1, CLANG_8, "-Wformat-invalid-specifier")
KBUILD_WARN(1, CLANG_8, "-Wformat-security") /* medium */
KBUILD_WARN(0, CLANG_8, "-Wformat-y2k")
KBUILD_WARN(2, CLANG_8, "-Wformat-zero-length")
KBUILD_WARN(0, CLANG_8, "-Wnonnull")
KBUILD_WARN(3, CLANG_8, "-Wformat-non-iso") /* lots */
KBUILD_WARN(4, CLANG_8, "-Wformat-pedantic") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wformat=2")
KBUILD_WARN(3, CLANG_8, "-Wformat-nonliteral") /* medium */

/* -Wextra level, corresponds to W=1 */
KBUILD_WARN(1, GCC_4_6, "-Wextra")
KBUILD_WARN(3, GCC_4_6, "-Wsign-compare") /* huge */
KBUILD_WARN(4, GCC_4_6, "-Wsign-compare") /* huge */
KBUILD_WARN(1, GCC_4_6, "-Wclobbered") /* never */
KBUILD_WARN(1, GCC_4_6, "-Wempty-body") /* medium */
KBUILD_WARN(1, GCC_4_6, "-Wignored-qualifiers") /* rare */
KBUILD_WARN(2, GCC_4_6, "-Wmissing-field-initializers") /* huge */
KBUILD_WARN(4, GCC_4_6, "-Wmissing-field-initializers") /* huge */
KBUILD_WARN(1, GCC_4_6, "-Wmissing-parameter-type") /* never */
KBUILD_WARN(1, GCC_4_6, "-Wold-style-declaration") /* rare */
KBUILD_WARN(1, GCC_4_6, "-Woverride-init") /* medium */
KBUILD_WARN(2, GCC_4_6, "-Wtype-limits") /* medium */
KBUILD_WARN(1, GCC_7, "-Wexpansion-to-defined") /* never */
KBUILD_WARN(1, GCC_8, "-Wcast-function-type") /* never */
KBUILD_WARN(0, GCC_7, "-Wimplicit-fallthrough=5") /* rare */
KBUILD_WARN(1, CLANG_8, "-Wextra")
KBUILD_WARN(1, CLANG_8, "-Wempty-init-stmt")
//KBUILD_WARN(partly, CLANG_8, "-Wignored-qualifiers") /* rare */
KBUILD_WARN(2, CLANG_8, "-Wmissing-field-initializers") /* huge */
KBUILD_WARN(1, CLANG_8, "-Wmissing-method-return-type")
KBUILD_WARN(1, CLANG_8, "-Wnull-pointer-arithmetic") /* rare */
KBUILD_WARN(1, CLANG_8, "-Wsemicolon-before-method-body")
KBUILD_WARN(2, CLANG_8, "-Wsign-compare") /* huge */
KBUILD_WARN(4, CLANG_8, "-Wsign-compare") /* huge */

/* Wunused and friends */
//KBUILD_WARN(partly, GCC_4_6, "-Wunused")
KBUILD_WARN(0, GCC_4_6, "-Wunused-result")
/*KBUILD_WARN(2, GCC_4_6, "-Wunused-macros") / lots */
KBUILD_WARN(4, GCC_4_6, "-Wunused-macros") /* lots */
KBUILD_WARN(1, GCC_4_6, "-Wunused-but-set-variable") /* medium */
KBUILD_WARN(0, GCC_4_6, "-Wunused-function")
KBUILD_WARN(0, GCC_4_6, "-Wunused-label")
KBUILD_WARN(0, GCC_4_6, "-Wunused-value")
KBUILD_WARN(0, GCC_4_6, "-Wunused-variable")
KBUILD_WARN(0, GCC_4_7, "-Wunused-local-typedefs")
KBUILD_WARN(1, GCC_4_6, "-Wunused-but-set-parameter") /* rare */
KBUILD_WARN(4, GCC_4_6, "-Wunused-parameter") /* harmful */

KBUILD_WARN(2, GCC_6, "-Wunused-const-variable=1")
KBUILD_WARN(4, GCC_6, "-Wunused-const-variable=2") /* lots */
KBUILD_WARN(1, CLANG_8, "-Wunused")
KBUILD_WARN(0, CLANG_8, "-Wunused-function")
KBUILD_WARN(0, CLANG_8, "-Wunneeded-internal-declaration")
KBUILD_WARN(0, CLANG_8, "-Wunused-label")
KBUILD_WARN(0, CLANG_8, "-Wunused-lambda-capture")
KBUILD_WARN(0, CLANG_8, "-Wunused-local-typedef")
KBUILD_WARN(0, CLANG_8, "-Wunused-private-field")
KBUILD_WARN(0, CLANG_8, "-Wunused-property-ivar")
KBUILD_WARN(0, CLANG_8, "-Wunused-value")
KBUILD_WARN(0, CLANG_8, "-Wunevaluated-expression")
KBUILD_WARN(0, CLANG_8, "-Wpotentially-evaluated-expression")
KBUILD_WARN(0, CLANG_8, "-Wunused-comparison")
KBUILD_WARN(0, CLANG_8, "-Wunused-result")
KBUILD_WARN(0, CLANG_8, "-Wunused-variable")
KBUILD_WARN(1, CLANG_8, "-Wunused-const-variable") /* medium */
KBUILD_WARN(4, CLANG_8, "-Wunused-parameter") /* harmful */

KBUILD_WARN(0, GCC_4_6, "-Wframe-larger-than=" __stringify(CONFIG_FRAME_WARN)) /* FIXME */
KBUILD_WARN(4, GCC_4_6, "-Wlarger-than=16384") /* huge */
KBUILD_WARN(3, GCC_4_7, "-Wstack-usage=1024") /* medium */
KBUILD_WARN(3, GCC_7, "-Walloc-size-larger-than=4096")
KBUILD_WARN(0, GCC_4_6, "-Wvla")
KBUILD_WARN(3, GCC_7, "-Wvla-larger-than=1") /* rare but useless */

//KBUILD_WARN(partly, CLANG_8, "-Wdeprecated")
KBUILD_WARN(0, CLANG_8, "-Wdeprecated-attributes")
KBUILD_WARN(0, CLANG_8, "-Wdeprecated-declarations")
KBUILD_WARN(3, CLANG_8, "-Wdeprecated-dynamic-exception-spec")

KBUILD_WARN(0, CLANG_8, "-Wtautological-compare")
KBUILD_WARN(1, CLANG_8, "-Wtautological-constant-compare")
KBUILD_WARN(1, CLANG_8, "-Wtautological-constant-out-of-range-compare") /* rare */
KBUILD_WARN(3, CLANG_8, "-Wtautological-overlap-compare")
KBUILD_WARN(1, CLANG_8, "-Wtautological-pointer-compare")
KBUILD_WARN(1, CLANG_8, "-Wtautological-undefined-compare")
KBUILD_WARN(3, CLANG_8, "-Wtautological-constant-in-range-compare")
KBUILD_WARN(3, CLANG_8, "-Wtautological-type-limit-compare") /* medium */
KBUILD_WARN(3, CLANG_8, "-Wtautological-unsigned-enum-zero-compare") /* rare */
KBUILD_WARN(3, CLANG_8, "-Wtautological-unsigned-zero-compare") /* rare */

//KBUILD_WARN(partly, CLANG_8, "-Wshadow-all")
//KBUILD_WARN(partly, CLANG_8, "-Wshadow")
KBUILD_WARN(2, CLANG_8, "-Wshadow-field-in-constructor-modified")
KBUILD_WARN(0, CLANG_8, "-Wshadow-ivar")
KBUILD_WARN(2, CLANG_8, "-Wshadow-field")
KBUILD_WARN(2, CLANG_8, "-Wshadow-field-in-constructor")
KBUILD_WARN(2, CLANG_8, "-Wshadow-field-in-constructor-modified")
KBUILD_WARN(2, CLANG_8, "-Wshadow-uncaptured-local")

//KBUILD_WARN(partly, CLANG_8, "-Wpragmas")
KBUILD_WARN(0, CLANG_8, "-Wignored-pragmas")
KBUILD_WARN(0, CLANG_8, "-Wignored-pragma-intrinsic")
KBUILD_WARN(0, CLANG_8, "-Wignored-pragma-optimize")
KBUILD_WARN(0, CLANG_8, "-Wpragma-clang-attribute")
//KBUILD_WARN(partly, CLANG_8, "-Wpragma-pack")
KBUILD_WARN(3, CLANG_8, "-Wpragma-pack-suspicious-include")
KBUILD_WARN(0, CLANG_8, "-Wunknown-pragmas")

KBUILD_WARN(1, CLANG_8, "-Wgnu")
KBUILD_WARN(1, CLANG_8, "-Wgnu-alignof-expression")
KBUILD_WARN(3, CLANG_8, "-Wgnu-anonymous-struct")
KBUILD_WARN(3, CLANG_8, "-Wgnu-auto-type")
KBUILD_WARN(3, CLANG_8, "-Wgnu-binary-literal") /* medium */
KBUILD_WARN(4, CLANG_8, "-Wgnu-case-range") /* harmful */
KBUILD_WARN(3, CLANG_8, "-Wgnu-complex-integer")
KBUILD_WARN(3, CLANG_8, "-Wgnu-compound-literal-initializer")
KBUILD_WARN(4, CLANG_8, "-Wgnu-conditional-omitted-operand") /* harmful */
KBUILD_WARN(1, CLANG_8, "-Wgnu-designator") /* harmful */
KBUILD_WARN(4, CLANG_8, "-Wgnu-empty-initializer") /* harmful */
KBUILD_WARN(4, CLANG_8, "-Wgnu-empty-struct") /* useless */
KBUILD_WARN(3, CLANG_8, "-Wgnu-flexible-array-initializer") /* medium */
KBUILD_WARN(3, CLANG_8, "-Wgnu-flexible-array-union-member")
KBUILD_WARN(3, CLANG_8, "-Wgnu-folding-constant") /* huge, harmful */
KBUILD_WARN(3, CLANG_8, "-Wgnu-imaginary-constant")
KBUILD_WARN(3, CLANG_8, "-Wgnu-include-next")
KBUILD_WARN(4, CLANG_8, "-Wgnu-label-as-value") /* harmful */
KBUILD_WARN(4, CLANG_8, "-Wgnu-redeclared-enum") /* harmful */
KBUILD_WARN(4, CLANG_8, "-Wgnu-statement-expression") /* harmful */
KBUILD_WARN(1, CLANG_8, "-Wgnu-static-float-init")
KBUILD_WARN(1, CLANG_8, "-Wgnu-string-literal-operator-template")
KBUILD_WARN(3, CLANG_8, "-Wgnu-union-cast")
KBUILD_WARN(1, CLANG_8, "-Wgnu-variable-sized-type-not-at-end") /* medium */
KBUILD_WARN(3, CLANG_8, "-Wgnu-zero-line-directive")
KBUILD_WARN(4, CLANG_8, "-Wgnu-zero-variadic-macro-arguments") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wredeclared-class-member")
KBUILD_WARN(3, CLANG_8, "-Wvla-extension")
KBUILD_WARN(4, CLANG_8, "-Wzero-length-array") /* excessive */
KBUILD_WARN(4, CLANG_10, "-Wlanguage-extension-token") /* excessive */

//KBUILD_WARN(partly, CLANG_8, "-Wnon-gcc")
//KBUILD_WARN(partly, CLANG_8, "-Wconversion")
KBUILD_WARN(3, CLANG_8, "-Wbitfield-enum-conversion") /* rare */
KBUILD_WARN(0, CLANG_8, "-Wbool-conversion")
KBUILD_WARN(0, CLANG_8, "-Wundefined-bool-conversion")
KBUILD_WARN(0, CLANG_8, "-Wconstant-conversion")
KBUILD_WARN(0, CLANG_8, "-Wbitfield-constant-conversion")
KBUILD_WARN(0, CLANG_8, "-Wenum-conversion")
KBUILD_WARN(3, CLANG_8, "-Wfloat-conversion") /* lots */
KBUILD_WARN(3, CLANG_8, "-Wfloat-overflow-conversion")
KBUILD_WARN(3, CLANG_8, "-Wfloat-zero-conversion") /* rare */
//KBUILD_WARN(partly, CLANG_8, "-Wimplicit-float-conversion")
#if 0 
KBUILD_WARN(4, CLANG_8, "-Wimplicit-int-conversion") /* huge */
#endif
KBUILD_WARN(0, CLANG_8, "-Wint-conversion")
KBUILD_WARN(0, CLANG_8, "-Wliteral-conversion")
KBUILD_WARN(0, CLANG_8, "-Wnon-literal-null-conversion")
KBUILD_WARN(0, CLANG_8, "-Wnull-conversion")
KBUILD_WARN(4, CLANG_8, "-Wshorten-64-to-32") /* excessive */
KBUILD_WARN(4, CLANG_8, "-Wsign-conversion") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wstring-conversion")
KBUILD_WARN(0, CLANG_8, "-Wliteral-range")
#if 0
KBUILD_WARN(2, CLANG_8, "-Wsign-compare")
#endif

KBUILD_WARN(0, CLANG_8, "-Wmicrosoft")
KBUILD_WARN(0, CLANG_8, "-Winconsistent-dllimport")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-anon-tag")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-cast")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-charize")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-comment-paste")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-const-init")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-cpp-macro")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-default-arg-redefinition")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-end-of-file")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-enum-forward-reference")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-enum-value")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-exception-spec")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-explicit-constructor-call")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-extra-qualification")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-fixed-enum")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-flexible-array")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-goto")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-include")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-mutable-reference")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-pure-definition")
KBUILD_WARN(3, CLANG_8, "-Wmicrosoft-redeclare-static")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-sealed")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-template")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-union-member-reference")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-unqualified-friend")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-using-decl")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-void-pseudo-dtor")
KBUILD_WARN(0, CLANG_8, "-Wmicrosoft-inaccessible-base")

KBUILD_WARN(4, CLANG_8, "-Wc99-compat")
KBUILD_WARN(4, CLANG_8, "-Wc99-extensions")
//KBUILD_WARN(partly, CLANG_8, "-Wduplicate-decl-specifier")
//KBUILD_WARN(partly, CLANG_8, "-Wexpansion-to-defined")
//KBUILD_WARN(partly, CLANG_8, "-Winvalid-or-nonexistent-directory")
//KBUILD_WARN(partly, CLANG_8, "-Wnarrowing")
//KBUILD_WARN(partly, CLANG_8, "-Wunguarded-availability")
KBUILD_WARN(3, CLANG_8, "-Wpointer-arith")
//KBUILD_WARN(partly, CLANG_8, "-Wsource-uses-openmp")
KBUILD_WARN(4, CLANG_8, "-Wvariadic-macros")
//KBUILD_WARN(partly, CLANG_8, "-Wcuda-compat")
//KBUILD_WARN(partly, CLANG_8, "-Wmain")
KBUILD_WARN(1, CLANG_8, "-Wstatic-in-inline")

/* Supported but disabled by default */
KBUILD_WARN(3, GCC_4_6, "-Wabi") /* never */
KBUILD_WARN(4, GCC_4_6, "-Waggregate-return") /* harmful */
/* KBUILD_WARN(3, GCC_4_6, "-Wbad-function-cast") * excessive */
/* KBUILD_WARN(3, GCC_4_6, "-Wcast-align") * huge */
KBUILD_WARN(4, GCC_4_6, "-Wcast-align") /* huge */
KBUILD_WARN(4, GCC_8, "-Wcast-align=strict") /* modifier */
/* KBUILD_WARN(3, GCC_4_6, "-Wcast-qual") * excessive */
KBUILD_WARN(4, GCC_4_6, "-Wc++-compat") /* excessive */
KBUILD_WARN(4, GCC_4_6, "-Wconversion") /* excessive */
KBUILD_WARN(2, GCC_4_6, "-Wdisabled-optimization") /* never */
KBUILD_WARN(3, GCC_4_6, "-Wdouble-promotion") /* medium */
KBUILD_WARN(3, GCC_4_6, "-Wfloat-equal") /* rare */
KBUILD_WARN(3, GCC_4_6, "-Winit-self") /* medium */
KBUILD_WARN(4, GCC_4_6, "-Winline") /* huge */
KBUILD_WARN(3, GCC_4_6, "-Winvalid-pch") /* never */
KBUILD_WARN(3, GCC_4_6, "-Wjump-misses-init") /* rare */
KBUILD_WARN(2, GCC_4_6, "-Wlogical-op") /* rare */
KBUILD_WARN(1, GCC_4_6, "-Wmissing-declarations") /* never */
KBUILD_WARN(1, GCC_4_6, "-Wmissing-include-dirs") /* never */
KBUILD_WARN(1, GCC_4_6, "-Wmissing-prototypes") /* medium */
KBUILD_WARN(3, GCC_4_6, "-Wmultichar") /* never */
KBUILD_WARN(4, GCC_4_6, "-Wnested-externs") /* excessive */
KBUILD_WARN(1, GCC_4_6, "-Wold-style-definition") /* rare */
KBUILD_WARN(3, GCC_4_6, "-Woverlength-strings") /* medium, useless */
KBUILD_WARN(4, GCC_4_6, "-Wpacked") /* huge */
KBUILD_WARN(4, GCC_4_6, "-Wpadded") /* excessive */
KBUILD_WARN(4, GCC_4_6, "-Wpointer-arith") /* excessive */
KBUILD_WARN(4, GCC_4_6, "-Wredundant-decls") /* huge */
KBUILD_WARN(2, GCC_4_6, "-Wshadow") /* huge ? */
KBUILD_WARN(4, GCC_4_6, "-Wshadow") /* huge ? */
KBUILD_WARN(4, GCC_4_6, "-Wsign-conversion") /* excessive */
KBUILD_WARN(3, GCC_4_6, "-Wstack-protector") /* medium */
KBUILD_WARN(0, GCC_4_6, "-Wstrict-prototypes")
KBUILD_WARN(4, GCC_4_6, "-Wsuggest-attribute=const") /* lots */
KBUILD_WARN(3, GCC_4_6, "-Wsuggest-attribute=noreturn") /* medium */
KBUILD_WARN(4, GCC_4_6, "-Wsuggest-attribute=pure") /* lots */
KBUILD_WARN(4, GCC_4_6, "-Wswitch-default") /* huge */
KBUILD_WARN(4, GCC_4_6, "-Wswitch-enum") /* huge */
KBUILD_WARN(3, GCC_4_6, "-Wsystem-headers") /* modifier */
KBUILD_WARN(4, GCC_4_6, "-Wtraditional-conversion") /* harmful */
KBUILD_WARN(4, GCC_4_6, "-Wtraditional") /* harmful */
KBUILD_WARN(3, GCC_4_6, "-Wtrampolines") /* never */
KBUILD_WARN(0, GCC_4_6, "-Wundef") /* rare */
KBUILD_WARN(4, GCC_4_6, "-Wunsuffixed-float-constants") /* lots */
KBUILD_WARN(3, GCC_4_6, "-Wvariadic-macros") /* never */
KBUILD_WARN(3, GCC_4_6, "-Wwrite-strings") /* never */
KBUILD_WARN(3, GCC_4_7, "-Wvector-operation-performance") /* rare */
KBUILD_WARN(4, GCC_4_8, "-Wpedantic") /* harmful */
KBUILD_WARN(3, GCC_4_8, "-Wsuggest-attribute=format") /* medium */
KBUILD_WARN(3, GCC_4_9, "-Wdate-time") /* never */
KBUILD_WARN(3, GCC_4_9, "-Wfloat-conversion") /* medium */
KBUILD_WARN(3, GCC_5, "-Wsuggest-final-methods") /* never */
KBUILD_WARN(3, GCC_5, "-Wsuggest-final-types") /* never */
KBUILD_WARN(3, GCC_6, "-Wduplicated-cond") /* medium */
KBUILD_WARN(3, GCC_6, "-Wnull-dereference") /* never */
KBUILD_WARN(3, GCC_7, "-Walloca") /* never */
KBUILD_WARN(3, GCC_7, "-Walloca-larger-than=1") /* never */
KBUILD_WARN(3, GCC_7, "-Walloc-zero") /* never */
KBUILD_WARN(3, GCC_7, "-Wduplicated-branches") /* medium */
KBUILD_WARN(4, GCC_7, "-Wshadow=compatible-local")
KBUILD_WARN(4, GCC_7, "-Wshadow=local") /* huge */
KBUILD_WARN(4, GCC_8, "-Wsuggest-attribute=cold") /* lots */
KBUILD_WARN(4, GCC_8, "-Wsuggest-attribute=malloc") /* lots */
KBUILD_WARN(3, CLANG_8, "-Warray-bounds-pointer-arithmetic") /* rare */
KBUILD_WARN(3, CLANG_8, "-Watomic-properties")
KBUILD_WARN(3, CLANG_8, "-Wcustom-atomic-properties")
KBUILD_WARN(3, CLANG_8, "-Wimplicit-atomic-properties")
KBUILD_WARN(3, CLANG_8, "-Wauto-import")
KBUILD_WARN(4, CLANG_8, "-Wbad-function-cast") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wbinary-literal")
KBUILD_WARN(3, CLANG_8, "-Wbitfield-width")
KBUILD_WARN(4, CLANG_8, "-Wc11-extensions") /* harmful */
KBUILD_WARN(1, CLANG_8, "-Wtypedef-redefinition")
KBUILD_WARN(3, CLANG_8, "-Wcast-align") /* huge */
KBUILD_WARN(4, CLANG_8, "-Wcast-align") /* huge */
KBUILD_WARN(4, CLANG_8, "-Wcast-qual") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wconditional-uninitialized") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wconsumed")
KBUILD_WARN(3, CLANG_8, "-Wcovered-switch-default") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wcstring-format-directive")
KBUILD_WARN(4, CLANG_8, "-Wdocumentation") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wdocumentation-deprecated-sync")
KBUILD_WARN(3, CLANG_8, "-Wdocumentation-html")
KBUILD_WARN(3, CLANG_8, "-Wdocumentation-pedantic")
KBUILD_WARN(4, CLANG_8, "-Wdocumentation-unknown-command") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wdouble-promotion") /* rare */
KBUILD_WARN(3, CLANG_8, "-Wduplicate-method-arg")
KBUILD_WARN(3, CLANG_8, "-Wduplicate-method-match")
KBUILD_WARN(3, CLANG_8, "-Wexit-time-destructors")
KBUILD_WARN(4, CLANG_8, "-Wextra-semi") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wextra-semi-stmt") /* excessive */
KBUILD_WARN(3, CLANG_8, "-Wfour-char-constants")
//KBUILD_WARN(3, CLANG_8, "-Wframe-larger-than=1024")
KBUILD_WARN(3, CLANG_8, "-Wheader-hygiene")
KBUILD_WARN(3, CLANG_8, "-Wimplicit-fallthrough")
KBUILD_WARN(3, CLANG_8, "-Wimplicit-fallthrough-per-function")
KBUILD_WARN(3, CLANG_8, "-Winconsistent-missing-destructor-override")
KBUILD_WARN(3, CLANG_8, "-Wkeyword-macro")
KBUILD_WARN(3, CLANG_8, "-Wlong-long") /* harmful */
KBUILD_WARN(3, CLANG_8, "-Wloop-analysis")
KBUILD_WARN(3, CLANG_8, "-Wrange-loop-analysis")
KBUILD_WARN(3, CLANG_8, "-Wmethod-signatures")
KBUILD_WARN(3, CLANG_8, "-Wmissing-noescape")
//KBUILD_WARN(3, CLANG_8, "-Wmodule-build")
KBUILD_WARN(3, CLANG_8, "-Wnullable-to-nonnull-conversion")
KBUILD_WARN(3, CLANG_8, "-Wover-aligned")
KBUILD_WARN(3, CLANG_8, "-Woverlength-strings") /* huge, useless */
KBUILD_WARN(3, CLANG_8, "-Woverriding-method-mismatch")
KBUILD_WARN(4, CLANG_8, "-Wpacked") /* huge */
KBUILD_WARN(4, CLANG_8, "-Wpadded") /* excessive */
//KBUILD_WARN(3, CLANG_8, "-Wpass")
//KBUILD_WARN(3, CLANG_8, "-Wpass-analysis")
KBUILD_WARN(3, CLANG_8, "-Wprofile-instr-missing")
KBUILD_WARN(3, CLANG_8, "-Wquoted-include-in-framework-header")
KBUILD_WARN(4, CLANG_8, "-Wreserved-id-macro") /* excessive */
//KBUILD_WARN(3, CLANG_8, "-Wsanitize-address")
KBUILD_WARN(3, CLANG_8, "-Wsection")
KBUILD_WARN(3, CLANG_8, "-Wselector")
KBUILD_WARN(3, CLANG_8, "-Wselector-type-mismatch")
KBUILD_WARN(3, CLANG_8, "-Wsigned-enum-bitfield")
KBUILD_WARN(3, CLANG_8, "-Wspir-compat")
KBUILD_WARN(3, CLANG_8, "-Wstrict-selector-match")
KBUILD_WARN(3, CLANG_8, "-Wswitch-enum") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wthread-safety")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-analysis")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-attributes")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-precise")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-reference")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-beta")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-negative")
KBUILD_WARN(3, CLANG_8, "-Wthread-safety-verbose")
KBUILD_WARN(3, CLANG_8, "-Wundeclared-selector")
KBUILD_WARN(3, CLANG_8, "-Wunreachable-code-aggressive")
KBUILD_WARN(3, CLANG_8, "-Wunreachable-code")
KBUILD_WARN(3, CLANG_8, "-Wunreachable-code-loop-increment")
KBUILD_WARN(3, CLANG_8, "-Wunreachable-code-break")
KBUILD_WARN(3, CLANG_8, "-Wunreachable-code-return")
KBUILD_WARN(3, CLANG_8, "-Wunused-exception-parameter")
KBUILD_WARN(3, CLANG_8, "-Wunused-local-typedef")
KBUILD_WARN(4, CLANG_8, "-Wused-but-marked-unused") /* huge */
KBUILD_WARN(3, CLANG_8, "-Wvector-conversion")
KBUILD_WARN(0, CLANG_8, "-Wvla")

/* Ignored by clang, only listed for reference */
KBUILD_WARN(3, CLANG_8, "-Wabi")
KBUILD_WARN(3, CLANG_8, "-Waggregate-return")
KBUILD_WARN(3, CLANG_8, "-Wat-protocol")
KBUILD_WARN(3, CLANG_8, "-Wchar-align")
KBUILD_WARN(3, CLANG_8, "-Wctor-dtor-privacy")
KBUILD_WARN(3, CLANG_8, "-Wdisabled-optimization")
KBUILD_WARN(3, CLANG_8, "-Wdiscard-qual")
KBUILD_WARN(3, CLANG_8, "-Wfuture-compat")
KBUILD_WARN(3, CLANG_8, "-Wimport")
KBUILD_WARN(3, CLANG_8, "-Winit-self")
KBUILD_WARN(3, CLANG_8, "-Winline")
KBUILD_WARN(3, CLANG_8, "-Winvalid-pch")
KBUILD_WARN(3, CLANG_8, "-Wliblto")
KBUILD_WARN(3, CLANG_8, "-Wmissing-format-attribute")
KBUILD_WARN(3, CLANG_8, "-Wmissing-include-dirs")
KBUILD_WARN(4, CLANG_8, "-Wnested-externs")
KBUILD_WARN(3, CLANG_8, "-Wnonportable-cfstrings")
KBUILD_WARN(3, CLANG_8, "-Wold-style-definition")
KBUILD_WARN(3, CLANG_8, "-Woverflow")
KBUILD_WARN(3, CLANG_8, "-Wpointer-to-int-cast")
KBUILD_WARN(3, CLANG_8, "-Wredundant-decls")
KBUILD_WARN(3, CLANG_8, "-Wsign-promo")
KBUILD_WARN(3, CLANG_8, "-Wstack-protector")
KBUILD_WARN(3, CLANG_8, "-Wstrict-aliasing")
KBUILD_WARN(3, CLANG_8, "-Wstrict-aliasing=0")
KBUILD_WARN(3, CLANG_8, "-Wstrict-aliasing=1")
KBUILD_WARN(3, CLANG_8, "-Wstrict-aliasing=2")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=0")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=1")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=2")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=3")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=4")
KBUILD_WARN(3, CLANG_8, "-Wstrict-overflow=5")
KBUILD_WARN(3, CLANG_8, "-Wstrict-prototypes")
KBUILD_WARN(3, CLANG_8, "-Wswitch-default")
KBUILD_WARN(3, CLANG_8, "-Wsynth")
KBUILD_WARN(3, CLANG_8, "-Wunused-argument")
KBUILD_WARN(3, CLANG_8, "-Wtype-limits") /* medium */


KBUILD_WARN(1, CLANG_11, "-Wframe-address") /* TBD */
