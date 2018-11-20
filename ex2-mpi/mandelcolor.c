/*
 * mandelcolor.c
 * Copyright (C) 2018 nicolasfara <nicolas.farabegoli@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main()
{
  int steps = 100;
  int i;
  float n;
  unsigned char r, g, b;

  unsigned char sr = 0x22, sg = 0xce, sb = 0x5a;
  unsigned char er = 0xca, eg = 0xac, eb = 0x0c;

  printf("const int colors[][3] = {");

  for( i = 0; i < steps; i++ )
  {
    n = i / (float) (steps-1);
    r = sr * (1.0f-n) + (float)er * n;
    g = sg * (1.0f-n) + (float)eg * n;
    b = sb * (1.0f-n) + (float)eb * n;
    printf("{%d, %d, %d},\n", r, g, b);
    // draw with r,g,b
  }
  printf(" };");
}
