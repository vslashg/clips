load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "clips",
    srcs = ["clips.cpp"],
    hdrs = ["clips.h"],
    deps = [
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_binary(
    name = "example",
    srcs = ["example.cc"],
    deps = [
        ":clips",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_binary(
    name = "search",
    srcs = ["search.cc"],
    malloc = "@com_google_tcmalloc//tcmalloc",
    deps = [
        ":clips",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)
