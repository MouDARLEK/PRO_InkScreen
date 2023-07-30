#ifndef _RESOURCES_H_
#define _RESOURCES_H_

// #if (SCREEN == 266)
#include "imageData\image_266_296x152_BW.c"
#include "imageData\image_266_296x152_BWR.c"
#include "imageData\BK.c"
#include "imageData\BK_RED.c"
// #define BW_monoBuffer        (uint8_t *) & image_266_296x152_BW_mono
// #define BW_monoBuffer        (uint8_t *) & gImage_you
// #define BW_red1Buffer        (uint8_t *) & gImage_you1


#define IMG_BW_BUFFER        (uint8_t *) & gImage_BK
#define IMG_RED_BUFFER        (uint8_t *) & gImage_BK_RED


#define BW_0x00Buffer        (uint8_t *) & image_266_296x152_BW_0x00
#define BWR_blackBuffer      (uint8_t *) & image_266_296x152_BWR_blackBuffer
#define BWR_redBuffer        (uint8_t *) & image_266_296x152_BWR_redBuffer
// #endif



#endif