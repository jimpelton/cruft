
#include <bd/log/gl_log.h>
#include <bd/context/context.h>
#include <bd/context/glfwcontext.h>


namespace bd {

// static
RenderLoop * Context::m_loop = nullptr;

// static
Context* Context::InitializeContext(RenderLoop *cc)
{
    gl_log_restart();
    gl_debug_log_restart();

    Context *context = new GlfwContext(cc);
    if (context) {
        bool success = context->init(1280, 720);
        context->isInit(success);
    }

    return context;
}


///////////////////////////////////////////////////////////////////////////////
Context::Context(RenderLoop *cc)
    : m_isInit{ false }
{
    m_loop = cc;
}


///////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
    if (m_loop) delete m_loop;
}


///////////////////////////////////////////////////////////////////////////////
void Context::startLoop()
{

    gl_log("Context initializing renderloop.");
    m_loop->initialize(*this);

    gl_log("Starting render loop.");
    do {
        m_loop->render();
        swapBuffers();
        pollEvents();
    } while (! windowShouldClose());

    gl_log("Renderloop exited.");
}


///////////////////////////////////////////////////////////////////////////////
RenderLoop & Context::renderLoop()
{ 
    return *m_loop;
}


///////////////////////////////////////////////////////////////////////////////
void Context::isInit(bool i) 
{
    m_isInit = i;
}


///////////////////////////////////////////////////////////////////////////////
bool Context::isInit() const 
{
    return m_isInit;
}

} // namespace bd
