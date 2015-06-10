#include "ScreenSpaceLocalReflection.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/Icosahedron.h>
#include <globjects/VertexAttributeBinding.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

ScreenSpaceLocalReflection::ScreenSpaceLocalReflection(gloperate::ResourceManager & resourceManager)
	: Painter(resourceManager)
	, m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
	, m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
	, m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
	, m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
}

ScreenSpaceLocalReflection::~ScreenSpaceLocalReflection() = default;

void ScreenSpaceLocalReflection::setupProjection()
{
	static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

	m_projectionCapability->setZNear(zNear);
	m_projectionCapability->setZFar(zFar);
	m_projectionCapability->setFovy(radians(fovy));

	m_grid->setNearFar(zNear, zFar);
}

void ScreenSpaceLocalReflection::onInitialize()
{
	// create program

	globjects::init();

#ifdef __APPLE__
	Shader::clearGlobalReplacements();
	Shader::globalReplace("#version 140", "#version 150");

	debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

	m_grid = new gloperate::AdaptiveGrid{};
	m_grid->setColor({ 0.6f, 0.6f, 0.6f });

	//m_icosahedron = new gloperate::Icosahedron{ 3 };

	m_vertices = new Buffer;
	m_vertices->setData(std::vector < float > {		// position.x, position.y, position.z, normal.x, normal.y, normal.z
//        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
//        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,

        -0.4f, -0.4f, -0.4f,  0.0f,  0.0f, -1.0f,	// front
         0.4f, -0.4f, -0.4f,  0.0f,  0.0f, -1.0f,
         0.4f,  0.4f, -0.4f,  0.0f,  0.0f, -1.0f,
        -0.4f,  0.4f, -0.4f,  0.0f,  0.0f, -1.0f,
         0.4f, -0.4f, -0.4f,  1.0f,  0.0f,  0.0f,	// right
         0.4f, -0.4f,  0.4f,  1.0f,  0.0f,  0.0f,
         0.4f,  0.4f,  0.4f,  1.0f,  0.0f,  0.0f,
         0.4f,  0.4f, -0.4f,  1.0f,  0.0f,  0.0f,
         0.4f, -0.4f,  0.4f,  0.0f,  0.0f,  1.0f,	// back
        -0.4f, -0.4f,  0.4f,  0.0f,  0.0f,  1.0f,
        -0.4f,  0.4f,  0.4f,  0.0f,  0.0f,  1.0f,
         0.4f,  0.4f,  0.4f,  0.0f,  0.0f,  1.0f,
        -0.4f, -0.4f,  0.4f, -1.0f,  0.0f,  0.0f,	// left
        -0.4f, -0.4f, -0.4f, -1.0f,  0.0f,  0.0f,
        -0.4f,  0.4f, -0.4f, -1.0f,  0.0f,  0.0f,
        -0.4f,  0.4f,  0.4f, -1.0f,  0.0f,  0.0f,
        -0.4f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,	// bottom
         0.4f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,
         0.4f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,
        -0.4f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,
        -0.4f,  0.4f, -0.4f,  0.0f,  1.0f,  0.0f,	// top
         0.4f,  0.4f, -0.4f,  0.0f,  1.0f,  0.0f,
         0.4f,  0.4f,  0.4f,  0.0f,  1.0f,  0.0f,
        -0.4f,  0.4f,  0.4f,  0.0f,  1.0f,  0.0f,
	}, gl::GL_STATIC_DRAW);

	m_indices = new Buffer;
	m_indices->setData(std::vector < unsigned int > {
//         0,
//         1,
//         2,
//         3,
//         4,
//         5,
//         6,
//         7,
//         8,
//         9,
//        10,
//        11,
//        12,
//        13

         0,  1,  2,
         2,  3,  0,
         4,  5,  6,
         6,  7,  4,
         8,  9, 10,
        10, 11,  8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
	}, gl::GL_STATIC_DRAW);

    m_size = 12 * 3 * sizeof(int);
//    m_size = 14 * sizeof(int);

	m_vao = new VertexArray;
	m_vao->bind();

	m_indices->bind(gl::GL_ELEMENT_ARRAY_BUFFER);

	auto binding = m_vao->binding(0);
	binding->setAttribute(0);	
	binding->setBuffer(m_vertices, 0, 6 * sizeof(float));
	binding->setFormat(3, gl::GL_FLOAT);
	m_vao->enable(0);

	binding = m_vao->binding(1);
	binding->setAttribute(1);
	binding->setBuffer(m_vertices, 3 * sizeof(float), 6 * sizeof(float));
	binding->setFormat(3, gl::GL_FLOAT);
	m_vao->enable(1);

	m_program = new Program{};
	m_program->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/screenspacelocalreflection/shader.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/screenspacelocalreflection/shader.frag")
		);

	m_transformLocation = m_program->getUniformLocation("transform");
    m_translateLocation = m_program->getUniformLocation("translate");

	glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

	setupProjection();

	m_vao->unbind();	
}

void ScreenSpaceLocalReflection::onPaint()
{
	if (m_viewportCapability->hasChanged())
	{
		glViewport(
			m_viewportCapability->x(),
			m_viewportCapability->y(),
			m_viewportCapability->width(),
			m_viewportCapability->height());

		m_viewportCapability->setChanged(false);
	}

	auto fbo = m_targetFramebufferCapability->framebuffer();

	if (!fbo)
		fbo = globjects::Framebuffer::defaultFBO();

	fbo->bind(GL_FRAMEBUFFER);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
	const auto eye = m_cameraCapability->eye();

	m_grid->update(eye, transform);
	m_grid->draw();

	m_program->use();
	m_program->setUniform(m_transformLocation, transform);

	//m_icosahedron->draw();

    auto translateMatrix = glm::mat4(1.0f, 0.0f, 0.0f,-1.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f);

    m_program->setUniform(m_translateLocation, translateMatrix);


	m_vao->bind();	
    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);
	m_vao->unbind();


    translateMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 1.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);

    m_program->setUniform(m_translateLocation, translateMatrix);

    m_vao->bind();
    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);
    m_vao->unbind();

	m_program->release();

	Framebuffer::unbind(GL_FRAMEBUFFER);
}
