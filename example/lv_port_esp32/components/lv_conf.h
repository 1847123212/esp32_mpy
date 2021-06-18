/**
 * @file lv_conf.h
 * Configuration file for v8.1.0-dev
 */

/*
 * COPY THIS FILE AS `lv_conf.h` NEXT TO the `lvgl` FOLDER
 */
#ifndef LV_CONF_H
#define LV_CONF_H
/*clang-format off*/

#include <stdint.h>

/*====================
   Graphical settings
 *====================*/

/* Maximal horizontal and vertical resolution to support by the library.*/
#ifndef LV_HOR_RES_MAX
#  ifdef CONFIG_LV_HOR_RES_MAX
#    define LV_HOR_RES_MAX CONFIG_LV_HOR_RES_MAX
#  else
#    define  LV_HOR_RES_MAX          (320)
#  endif
#endif
#ifndef LV_VER_RES_MAX
#  ifdef CONFIG_LV_VER_RES_MAX
#    define LV_VER_RES_MAX CONFIG_LV_VER_RES_MAX
#  else
#    define  LV_VER_RES_MAX          (240)
#  endif
#endif
#endif