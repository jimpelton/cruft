#ifndef texture_h__
#define texture_h__

#include <bd/graphics/shader.h>

#include <string>
#include <cstdlib>
#include <bd/io/datatypes.h>

namespace bd
{
///////////////////////////////////////////////////////////////////////////////
/// \brief  Wraps up GL texture creation and binds it when asked.
/// \note If Texture is wrapping a 3D texture, then the texture unit
///   \c GL_TEXTURE0+static_cast<unsigned>(Sampler::Volume) is activated.
///
/// \note If a 1D texture is wrapped then the texture unit bound is
///   \c GL_TEXTURE0+static_cast<unsigned>(Sampler::Transfer) is activated.
///////////////////////////////////////////////////////////////////////////////
class Texture
{
public:

  enum class Target: unsigned int
  {
    Tex1D,
    Tex2D,
    Tex3D
  };


  enum class Format: unsigned int
  {
    RED,
    RG,
    RGB,
    RGBA
  };


  Texture(Target textureType);
  virtual ~Texture();


  /// \brief Binds this texture to the current active texture.
  void bind() const;


  /// \brief Activates texture unit \c unit and binds this texture.
  void bind(unsigned int unit) const;


  unsigned int genGLTex1d(float const* img, Format ity, Format ety, size_t w);


  unsigned int genGLTex2d(float* img, Format ity, Format ety, size_t w, size_t h);


  unsigned int genGLTex3d(Format internal, Format external, size_t w, size_t h, size_t d,
      DataType ty, void* pixelData);


  unsigned int
  id() const
  {
    return m_id;
  }


  Target
  target() const
  {
    return m_type;
  }

  std::string to_string() const;

private:
  ///////////////////////////////////////////////////////////////////////////////
  // Data members
  ///////////////////////////////////////////////////////////////////////////////

  unsigned int m_id; ///< OpenGL id of the texture data.
  //  unsigned int m_samplerUniform;  ///< OpenGL id of the texture sampler to use.
  //  unsigned int m_unit; ///< Texture sampling unit.
  Target m_type; ///< the gl target to bind to.
  //    Sampler m_sampler; ///
};


std::ostream& operator<<(std::ostream& os, const bd::Texture& t);
} // namespace bd


#endif // ! texture_h__


