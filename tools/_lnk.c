#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    printf("%%=%u !=20=%u !=25=%u 0023=%u 002F=%u 003F=%u 003D=%u 0026=%u 003A=%u 007E=%u 002C=%u\n",
           unicodoid_ucd_link_term(0x25), unicodoid_ucd_link_term(0x20),
           unicodoid_ucd_link_term(0x21), unicodoid_ucd_link_term(0x23),
           unicodoid_ucd_link_term(0x2F), unicodoid_ucd_link_term(0x3F),
           unicodoid_ucd_link_term(0x3D), unicodoid_ucd_link_term(0x26),
           unicodoid_ucd_link_term(0x3A), unicodoid_ucd_link_term(0x7E),
           unicodoid_ucd_link_term(0x2C));
    return 0;
}
