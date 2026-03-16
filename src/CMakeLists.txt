set(target_name homevfd)
add_executable(${target_name})

target_sources(${target_name} PRIVATE
	hv_usb.c
	hv_usb_dev.c
	hv_vfd.c
	main.c
)
target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_name} PRIVATE -Wall -Wextra)

target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${target_name} PRIVATE
	pico_stdlib
	pico_unique_id
	hardware_flash
	tinyusb_device
)
pico_add_extra_outputs(${target_name})
