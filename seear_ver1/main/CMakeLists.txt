set(COMPONENT_SRCS "main.c" "I2SSampler.c" "connect.c" "fetch.c" )
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

#spiffs_create_partition_image(storage ../spiffs_dir FLASH_IN_PROJECT)