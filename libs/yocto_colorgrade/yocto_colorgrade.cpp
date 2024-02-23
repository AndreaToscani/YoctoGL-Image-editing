//
// Implementation for Yocto/Grade.
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "yocto_colorgrade.h"

#include <yocto/yocto_color.h>
#include <yocto/yocto_sampling.h>

// -----------------------------------------------------------------------------
// COLOR GRADING FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto {

color_image grade_image(const color_image& image, const grade_params& params) {
  // PUT YOUR CODE HERE
  int       i = 0, j = 0;
  rng_state rng = make_rng(10);
  auto  img = image;
  for (auto& pixel : img.pixels) {
    if (i >= image.width) {
      i = 0;
      j++;
    }
    
    if (j >= image.height) j = 0;
    vec3f c = xyz(pixel);



    // Tone mapping
    if (params.exposure != 0) {
      c = c * pow(2, params.exposure);
    }
    if (params.filmic == true) {
      c *= 0.6;
      c = ((pow(c, 2) * 2.51 + c * 0.03) /
           (pow(c, 2) * 2.43 + c * 0.59 + 0.14));
    }
     if(params.srgb==true) c = pow(c, 1 / 2.2);
      c = clamp(c, 0, 1);
    

    // Color Tint
      if (params.tint != vec3f{1, 1, 1}) {
        c = c * params.tint;
      }


    // Saturation
    if (params.saturation != 0.5) {
    auto g = (c.x + c.y + c.z) / 3;
    c      = g + (c - g) * (params.saturation * 2);
    }



    //Contrast
    if (params.contrast != 0.5) {
      c = gain(c, 1 - params.contrast);
    }


    //Vignette:
    if (params.vignette != 0) {
      auto  vr   = 1 - params.vignette;
      vec2f ij   = {i, j};
      vec2f size = {image.width, image.height};
      auto  r    = length(ij - size / 2) / length(size / 2);
      c          = c * (1 - smoothstep(vr, 2 * vr, r));
    }



     //Film Grain
    if (params.grain!=0) {
      c = c + (rand1f(rng) - 0.5) * params.grain;
    }



     i++;
     pixel = {c.x, c.y, c.z, pixel.w};
  }


  // Mosaic
  int k = 0, y = 0;
  vec4f c;
  if (params.mosaic != 0) 
  {
    for (k = 0; k < image.width; k++) {
      for ( y = 0; y < image.height; y++) {
        auto c  = get_pixel(img, y, k);
        auto c2 = get_pixel(img, k - k % params.mosaic, y - y % params.mosaic);
        c = c2;
       
        set_pixel(img, k, y, c);
      }
    }
    
  }
  // Grid
  if (params.grid != 0) {
  
   for (k = 0; k < image.width; k++) {
      for ( y = 0; y < image.height; y++) {
        auto c = get_pixel(img, k, y);
       
          if (0 == k % params.grid || 0 == y % params.grid)
            c = 0.5 * c;
        set_pixel(img, k, y, c);
      }
    }
   
  }
   
  
	return img;
}

}  // namespace yocto