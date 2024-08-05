load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "javelin-steno",
    srcs = glob([
        "**/*.cc",
        "**/*.h",
    ]),
    defines = [
        "RUN_TESTS=1",
        "JAVELIN_BOARD_CONFIG=<stddef.h>",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)
