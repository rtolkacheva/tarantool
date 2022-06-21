build_module(loaderslib loaderslib.c)
build_module(on_shutdownlib on_shutdownlib.c)

add_custom_target(app-test-lib
                  DEPENDS loaderslib
                          on_shutdownlib)
