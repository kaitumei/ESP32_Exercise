set(srcs
    "srcs/myiic.c"
    "srcs/xl9555.c"
    "srcs/at24cxx.c"
)

set(include_dirs
    "includes"
)

set(priv_requires 
    "esp_driver_gpio"
)

set(requires
    "freertos"
    "esp_driver_i2c"
)



idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    PRIV_REQUIRES ${priv_requires}
    REQUIRES ${requires}
)

# component_compile_options(-ffast-math -O3 -Wno-error=format -Wno-format)