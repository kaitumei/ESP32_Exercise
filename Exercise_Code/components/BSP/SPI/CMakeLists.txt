set(srcs
    "srcs/spi.c"
    "srcs/lcd.c"
)

set(include_dirs
    "includes"
)

set(priv_requires 
    "esp_driver_gpio"
    "esp_driver_spi"
    "IIC"
)

set(requires
    "freertos"
)



idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    PRIV_REQUIRES ${priv_requires}
    REQUIRES ${requires}
)

# component_compile_options(-ffast-math -O3 -Wno-error=format -Wno-format)