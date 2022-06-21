build_module(gh_6506_lib gh_6506_wakeup_writing_to_wal_fiber.c)

add_custom_target(box-luatest-test-lib
                  DEPENDS gh_6506_lib)
