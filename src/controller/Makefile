TTYUSB = 
BOOTLOADER = my-install

# the string to extract for checking
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:\|NRF:\|SUCCESS:'

# set if you want the code to automatically run after building.
RUN = 0

TEST_SRC := ./tests/controller_output_test.c
#TEST_SRC := ./tests/controller_test.c
#TEST_SRC := ./tests/nrf_test.c

#SRC := nrf-util.c nrf-public.c nrf-driver.c nes_controller.c
SUPPORT_OBJS := nes_controller.o

LIB := libproj.a

# so we have division.
# LIBS += libgcc.a

include $(CS140E_2022_PATH)/libpi/mk/Makefile.template
