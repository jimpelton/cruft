#include <GL/glew.h>

#include <bd/graphics/shader.h>
#include <bd/log/gl_log.h>
#include <bd/util/gl_strings.h>

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <vector>

#include <sstream>
#include <fstream>


namespace bd {

namespace {
const std::array<GLenum, 2> shTypes
{
    GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
};
}

///////////////////////////////////////////////////////////////////////////////
Shader::Shader(ShaderType t)
    : m_type(t), m_id(0)
{
}


///////////////////////////////////////////////////////////////////////////////
unsigned int Shader::loadFromFile(const std::string &filepath)
{
    //GLuint shaderId = 0;
    std::ifstream file(filepath.c_str());
    if (!file.is_open()) {
        gl_log_err("Couldn't open %s", filepath.c_str());
        return 0;
    }
    std::stringstream shaderCode;
    shaderCode << file.rdbuf();

    std::string code = shaderCode.str();
    const char *ptrCode = code.c_str();
    file.close();

    return compileShader(ptrCode);
}


///////////////////////////////////////////////////////////////////////////////
unsigned int Shader::loadFromString(const std::string &shaderString)
{
    return compileShader(shaderString.c_str());
}


///////////////////////////////////////////////////////////////////////////////
unsigned int Shader::compileShader(const char *shader)
{

    GLenum gl_type = shTypes.at(static_cast<int>(m_type));
    GLuint shaderId = gl_check(glCreateShader(gl_type));

    gl_log("Created shader, type=%s, id=%d", bd::gl_to_string(gl_type), shaderId);
    gl_check(glShaderSource(shaderId, 1, &shader, nullptr));

    gl_check(glCompileShader(shaderId));

    // Check for errors.
    GLint result = GL_FALSE;
    int infoLogLength;

    gl_check(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
    gl_check(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));

    if (infoLogLength > 1) {
        std::vector<char> msg(infoLogLength + 1);
        gl_check(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &msg[0]));
        gl_log("%s", &msg[0]);
    }


    return m_id = shaderId;
}

ShaderType Shader::type() const
{
    return m_type;
}

const char* Shader::typeString() const
{
    return bd::gl_to_string(shTypes[static_cast<unsigned>(m_type)]);
}


//////////////////////////////////////////////////////////////////////////
//   ShaderProgram Impl
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram()
{
}


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(Shader *vert, Shader *frag)
{
    if (vert) addStage(vert);
    if (frag) addStage(frag);
}


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::~ShaderProgram()
{
    //TODO: cleanup opengl shaders ... glDelete?
    gl_log_err("I might be a memory leak. Who cares! This be academia, foo!");
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::addStage(Shader *stage)
{
    m_stages.push_back(stage);
    gl_log("Added shader %s", gl_to_string(shTypes[static_cast<unsigned>(stage->type())]));
    return stage->id();
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::addStage(const std::string &path, ShaderType ty)
{
    Shader *sh = new Shader(ty);
    sh->loadFromFile(path);
    return addStage(sh);
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::linkProgram()
{
    if (!checkBuilt())
        return 0;

    if (m_stages.empty()){
        gl_log_err("When linking program, there were no shaders to link! Cannot "
            "create shader program.");
        return 0;
    }

    if (m_programId) {
        gl_log("Relinking shader program id=%d", m_programId);
    }
    else {
        createNewProgram();
        if (m_programId == 0) return 0;
        gl_log("Linking shader program id=%d", m_programId);
    }

    gl_check(glLinkProgram(m_programId));

    // Check the program
    int InfoLogLength = 0;
    GLint result = GL_FALSE;

    gl_check(glGetProgramiv(m_programId, GL_LINK_STATUS, &result));
    gl_check(glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &InfoLogLength));

    if (InfoLogLength > 1) {
        std::vector<char> programErrorMessage(InfoLogLength + 1);
        gl_check(glGetProgramInfoLog(m_programId, InfoLogLength, nullptr, &programErrorMessage[0]));
        gl_log("%s", &programErrorMessage[0]);
    }

    return m_programId;
}

///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::linkProgram(Shader *vert, Shader *frag)
{
    addStage(vert);
    addStage(frag);
    return linkProgram();
}

unsigned int ShaderProgram::linkProgram(const std::string &vertPath,
    const std::string &fragPath)
{
    Shader *vert = new Shader(ShaderType::Vertex);
    vert->loadFromFile(vertPath);

    Shader *frag = new Shader(ShaderType::Fragment);
    frag->loadFromFile(fragPath);

    return linkProgram(vert, frag);
}

///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char* param, const glm::mat4& val)
{
    unsigned int loc = getUniformLocation(param);
    gl_check(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, const glm::vec4 &val)
{
    unsigned int loc = getUniformLocation(param);
    gl_check(glUniform4fv(loc, 1, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, const glm::vec3 &val)
{
    unsigned int loc = getUniformLocation(param);
    gl_check(glUniform3fv(loc, 1, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, float val )
{
    unsigned int loc = getUniformLocation(param);
    gl_check(glUniform1f(loc, val));
}


///////////////////////////////////////////////////////////////////////////////
//void ShaderProgram::setUniform(const std::string &param, const Texture &tex)
//{
//    unsigned int loc = getUniformLocation(param);
//    m_textures[loc] = &tex;
//}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::getUniformLocation(const char *param)
{
    unsigned int rval = 0;
    ParamTable::iterator found = m_params.find(param);
    if (found != m_params.end()) {
        rval = (*found).second;
    } else {
        rval = gl_check(glGetUniformLocation(m_programId, param));
        m_params[param] = rval;
    }

    return rval;
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bind()
{
//    static const std::array<GLenum, 3> targets
//    {
//        GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D
//    };

    gl_check(glUseProgram(m_programId));

//    int i = 0;
    // pair is (glsl sampler location, Texture)
//    for (auto &pair : m_textures) {
//        gl_check(glActiveTexture(GL_TEXTURE0 + i));
//        const Texture &tex = *(pair.second);
//        GLenum target = targets.at(static_cast<int>(tex.type()));
//        gl_check(glBindTexture(target, tex.id()));
//        gl_check(glUniform1f(pair.first, tex.id()));
//
//        i += 1;
//    }

    //gl_check(glActiveTexture(GL_TEXTURE0));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::unbind()
{
    gl_check(glUseProgram(0));
}


unsigned int ShaderProgram::programId() const
{
    return m_programId;
}


void ShaderProgram::createNewProgram()
{
    GLuint programId = gl_check(glCreateProgram());
    if (programId == 0){
        gl_log_err("Unable to create shader program with glCreateProgram(). Returned id was 0.");
        return;
    }

    gl_log("Created program id: %d", programId);

    for(const Shader *stage : m_stages){
        gl_check(glAttachShader(programId, stage->id()));
    }

    m_programId = programId;
}

///////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::checkBuilt()
{
    bool rval = true;
    auto it = m_stages.begin();
    while (rval && it != m_stages.end()){
        gl_log("Check shader: %s, id=%d", (*it)->typeString(), (*it)->id());
        rval = (*it)->isBuilt();
        ++it;
    }

    if (!rval){
        gl_log_err("While linking shader program %d, I found that at least one shader "
            "(id=%d) was not built, cannot link program.", m_programId, (*it)->id());
    }

    return rval;
}

} //namespace bd

