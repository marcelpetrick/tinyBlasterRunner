idf_component_register(
    SRCS
        "test_main.c"
        "../../main/game_math.c"
    INCLUDE_DIRS "." "../../main"
    REQUIRES unity
)
