#include <bd/graphics/texture.h>
#include <bd/util/gl_strings.h>
#include <bd/util/ordinal.h>
#include <bd/log/gl_log.h>

#include <GL/glew.h>

#include <array>
#include <sstream>


namespace bd
{

namespace
{
static const std::array<GLenum, 4> gl_format{
  GL_RED, GL_RG, GL_RGB, GL_RGBA
};

static const std::array<GLenum, 3> gl_target{
  GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D
};

GLenum pixelType(DataType dt)
{
  GLenum rval;
  switch(dt) {
    case DataType::Integer:
      rval = GL_INT;
      break;
    case DataType::UnsignedInteger:
      rval = GL_UNSIGNED_INT;
      break;
    case DataType::Character:
      rval = GL_BYTE;
      break;
    case DataType::UnsignedCharacter:
      rval = GL_UNSIGNED_BYTE;
      break;
    case DataType::Short:
      rval = GL_SHORT;
      break;
    case DataType::UnsignedShort:
      rval = GL_UNSIGNED_SHORT;
      break;
    case DataType::Float:
    default:
      rval = GL_FLOAT;
      break;
  }

  return rval;
}


} // namespace

Texture::Texture(Target textureType)
  : m_id{ 0 }
  , m_type{ textureType }
{
}

Texture::~Texture()
{
}


void
Texture::bind() const
{
  glBindTexture(gl_target[bd::ordinal<Target>(m_type)], m_id);
}


void
Texture::bind(unsigned int unit) const
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(gl_target[ordinal<Target>(m_type)], m_id);
  //  glUniform1i(m_samplerUniform, unit);
}


unsigned int
Texture::genGLTex1d(float* img, Format ity, Format ety, size_t w)
{
  GLuint texId{ 0 };
  gl_check(glGenTextures(1, &texId));
  gl_check(glBindTexture(GL_TEXTURE_1D, texId));

  gl_check(glTexImage1D(
      GL_TEXTURE_1D,
      0,
      gl_format[ordinal<Format>(ity)],
    w,
    0,
    gl_format[ordinal<Format>(ety)],
    GL_FLOAT,
    static_cast<void *>(img)
    ));

  gl_check(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));

  m_type = Target::Tex1D;
  m_id = texId;

  return texId;
}


unsigned int
Texture::genGLTex2d(float* img, Format ity, Format ety, size_t w, size_t h)
{
  GLuint texId{ 0 };
  gl_check(glGenTextures(1, &texId));
  gl_check(glBindTexture(GL_TEXTURE_2D, texId));

  gl_check(glTexImage2D(
      GL_TEXTURE_2D,
      0,
      gl_format[ordinal<Format>(ity)],
    w, h,
    0,
    gl_format[ordinal<Format>(ety)],
    GL_FLOAT,
    img));

  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));

  gl_check(glBindTexture(GL_TEXTURE_2D, 0));

  m_type = Target::Tex2D;
  m_id = texId;

  return texId;
}


unsigned int
Texture::genGLTex3d(Format ity, Format ety, size_t w, size_t h, size_t d,
    DataType ty, void* pixelData)
{
  const GLint border{ 0 };
  const GLint mipMapLevel{ 0 };

  GLuint texId{ 0 };
  gl_check(glGenTextures(1, &texId));
  gl_check(glBindTexture(GL_TEXTURE_3D, texId));

  gl_check(glTexImage3D(
    GL_TEXTURE_3D,
    mipMapLevel,                         // mip-map detail level
    gl_format[ordinal<Format>(ity)],     // internal format (how texture is to be stored by GL)
    w, h, d,                             // dimensions
    border,                              // border
    gl_format[ordinal<Format>(ety)],     // pixel data format (R, RG, etc.)
    pixelType(ty),                       // pixel data type
    pixelData));                         // ptr to data

  gl_check(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

  gl_check(glBindTexture(GL_TEXTURE_3D, 0));

  m_type = Target::Tex3D;
  m_id = texId;

  return texId;
}


std::string
Texture::to_string() const
{
  std::stringstream ss;
  GLenum tt;
  tt = gl_target[bd::ordinal<Target>(m_type)];

  static const std::array<GLenum, 8> thingies2{
    GL_TEXTURE_WIDTH,
    GL_TEXTURE_HEIGHT,
    GL_TEXTURE_RED_SIZE,
    GL_TEXTURE_BLUE_SIZE,
    GL_TEXTURE_GREEN_SIZE,
    GL_TEXTURE_ALPHA_SIZE,
    GL_TEXTURE_COMPRESSED,
    GL_TEXTURE_COMPRESSED_IMAGE_SIZE
  };

  ss << "{ Id: " << m_id << ", Type: " << bd::gl_to_string(tt);
  if (m_id != 0) {
    ss << "\nGL values:";
    GLint val{ 0 };
    for (size_t i = 0; i < thingies2.size(); ++i) {
      glGetTextureLevelParameteriv(m_id, 0, thingies2[i], &val);
      ss << "\n\t" << bd::gl_to_string(thingies2[i]) << ": " << val;
    }
  }
  ss << " }";
  return ss.str();
}

std::ostream&
operator<<(std::ostream& os, const bd::Texture& t)
{
  return os << t.to_string();
}




} // namespace bd


