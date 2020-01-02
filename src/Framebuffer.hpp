// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef FRAME_BUFFER_HPP
#define FRAME_BUFFER_HPP

#include <smk/OpenGL.hpp>
#include <smk/Texture.hpp>
#include <iostream>

class Framebuffer {
 public:
  Framebuffer(int width, int height);
  Framebuffer(Framebuffer&&);
  Framebuffer(const Framebuffer&) = delete;
  ~Framebuffer();
  void operator=(Framebuffer&&);
  void operator=(const Framebuffer&) = delete;

  void Bind();
  void Unbind();

  void GenerateMipmap();

  smk::Texture color_texture;

 private:
  void Init();
  void Reset();

  int width_;
  int height_;
  GLuint frame_buffer_ = 0;
  GLuint render_buffer_ = 0;
};

#endif /* end of include guard: FRAME_BUFFER_HPP */
