#ifndef shader_h__
#define shader_h__

#include <bd/util/texture.h>

#include <glm/glm.fwd>

#include <string>
#include <map>
#include <vector>

namespace bd {

enum class ShaderType
{
    Vertex,
    Fragment
};

class Shader
{
public:
    Shader(ShaderType t);

    //////////////////////////////////////////////////////////////////////////
    /// \brief Compile the shader in given file.
    /// \param A string containing the path to the file containing the shader.
    /// \return The non-zero gl identifier of the compiled shader, 0 on error.
    //////////////////////////////////////////////////////////////////////////
    unsigned int loadFromFile(const std::string &filepath);

   ///////////////////////////////////////////////////////////////////////////////
   /// \brief Compile the shader in given string.
   ///  \param A string containing the path to the file containing the shader.
   ///  \return The non-zero gl identifier of the compiled shader, 0 on error.
   ///////////////////////////////////////////////////////////////////////////////
    unsigned int loadFromString(const std::string &shaderString);


    //////////////////////////////////////////////////////////////////////////
    /// \brief Get the type of this Shader.
    //////////////////////////////////////////////////////////////////////////
    ShaderType type() const;


    //////////////////////////////////////////////////////////////////////////
    /// \brief Get a string representation of this shader's type
    /// returns either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
    //////////////////////////////////////////////////////////////////////////
    const char* typeString() const;


    //////////////////////////////////////////////////////////////////////////
    /// \brief Get the GL id of this Shader.
    /// \note id() returns zero if this shader has not been compiled yet.
    //////////////////////////////////////////////////////////////////////////
    unsigned int id() const { return m_id; }


    //////////////////////////////////////////////////////////////////////////
    /// \brief True if shader has been compiled, false otherwise.
    //////////////////////////////////////////////////////////////////////////
    bool isBuilt() const { return id() != 0; }

private:
    unsigned int compileShader(const char *shader);

    ShaderType m_type;  ///< bd type of this Shader
    unsigned int m_id;  ///< OpenGL id of this shader.
};


///////////////////////////////////////////////////////////////////////////////
// Class ShaderProgram
///////////////////////////////////////////////////////////////////////////////
class ShaderProgram
{
public:

    ////////////////////////////////////////////////////////////////////////////////
    // Constructors/Destructor
    ////////////////////////////////////////////////////////////////////////////////
    ShaderProgram();

    ShaderProgram(Shader *vert, Shader *frag);

    virtual ~ShaderProgram();

    //////////////////////////////////////////////////////////////////////////
    /// \brief Add the given Shader object as a stage in this program.
    /// \returns I don't know.
    //////////////////////////////////////////////////////////////////////////
    unsigned int addStage(Shader *);


    //////////////////////////////////////////////////////////////////////////
    /// \brief Add the Shader in the file at \c shaderPath as a stage in
    /// this program.
    /// \returns I can't remember.
    //////////////////////////////////////////////////////////////////////////
    unsigned int addStage(const std::string &shaderPath, bd::ShaderType);



    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Link frag and vert shaders if already provided (via constructor).
    /// \return The non-zero gl identifier for the program, 0 on error.
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int linkProgram();



    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Link provided frag and vertex shaders.
    /// \return The non-zero gl identifier for the program, 0 on error.
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int linkProgram(Shader *vert, Shader *frag);

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Open, build and link shaders in files at provided paths.
    /// \return The non-zero gl identifier for the program, 0 on error.
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int linkProgram(const std::string &vertPath, const std::string &fragPath);


    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the shader uniform specified by \c param to \c val.
    ///////////////////////////////////////////////////////////////////////////////
    void setUniform(const char *param, const glm::mat4 &val);
    void setUniform(const char *param, const glm::vec4 &val);
    void setUniform(const char *param, const glm::vec3 &val);
    void setUniform(const char *param, float val);
//    void setUniform(const std::string &param, const Texture &tex);


    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Get the id of the spec'd param.
    /// \return The non-zero gl identifier, or 0 if param has never been set.
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getUniformLocation(const char *param);


    void bind();
    void unbind();


    /// \brief Get the GL id of this ShaderProgram.
    unsigned int programId() const;


private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private Members
    ///////////////////////////////////////////////////////////////////////////////

    /// \brief True if both shaders have been built, false otherwise.
    bool checkBuilt();

    /// \brief Creates a new program with the GL, attaches shader stages.
    void createNewProgram();

    /// \brief Map uniform name to location.
    using ParamTable = std::map<const char*, unsigned int >;
    
//    using TextureTable = std::map<unsigned int, const Texture* >;

    std::vector<Shader *> m_stages;
    unsigned int m_programId; ///< The opengl shader program id
    ParamTable m_params;      ///< Uniform locations

//    TextureTable m_textures;  ///< Texture sampler locatons

};

} // namespace bd

#endif /* shader_h__ */
