set(srcs
    "LED/led.c"
)

set(include_dirs
    "LED"
)

set(requires
    "esp_driver_gpio"
    "freertos"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${requires}
)

component_compile_options(-ffast-math -O3 -Wno-error=format -Wno-format)