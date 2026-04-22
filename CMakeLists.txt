cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Implementation_ROB5)

target_sources(app PRIVATE
  src/main.c
  src/drivers/moteur.c
  src/drivers/moteur.h
  src/sensors/sensors.c
src/sensors/sensors.h
)