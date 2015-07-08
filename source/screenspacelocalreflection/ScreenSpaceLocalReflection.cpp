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

#define _USE_MATH_DEFINES 
#include <math.h>


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

	m_textureColor = new Texture();
	m_textureColor->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

	m_fbo = new Framebuffer();
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_textureColor, 0);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	GLsizei n = 1;
	m_fbo->setDrawBuffers(n, drawBuffers);

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

		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,	// front
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,	// right
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,	// back
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,	// left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,	// bottom
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,	// top
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	}, gl::GL_STATIC_DRAW);

	m_indices = new Buffer;
	m_indices->setData(std::vector < unsigned int > {
//         0,	// triangle strip
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
//    m_size = 14 * sizeof(int);	// triangle strip

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

	m_cubeProgram = new Program{};
	m_cubeProgram->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/screenspacelocalreflection/shader.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/screenspacelocalreflection/shader.frag")
		);

	m_transformLocation = m_cubeProgram->getUniformLocation("transform");
	m_translateLocation = m_cubeProgram->getUniformLocation("translate");
	m_rotateLocation = m_cubeProgram->getUniformLocation("rotate");
	m_scaleLocation = m_cubeProgram->getUniformLocation("scale");

	m_quadProgram = new Program{};
	m_quadProgram->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/screenspacelocalreflection/quad.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/screenspacelocalreflection/quad.frag")
		);

	m_textureColorLocation = m_quadProgram->getUniformLocation("textureColor");

	m_saQuad = new gloperate::ScreenAlignedQuad(m_quadProgram);

	glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

	setupProjection();

	m_vao->unbind();
	m_fbo->unbind();
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

		m_textureColor->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_viewportCapability->setChanged(false);
	}

	m_fbo->bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
	const auto eye = m_cameraCapability->eye();

	m_grid->update(eye, transform);
	m_grid->draw();

	m_cubeProgram->use();
	m_cubeProgram->setUniform(m_textureColorLocation, m_textureColor);
	m_cubeProgram->setUniform(m_transformLocation, transform);

    auto scale = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
                           0.0f, 0.5f, 0.0f, 0.0f,
                           0.0f, 0.0f, 0.5f, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);

    auto rotate = glm::mat4(cos(-M_PI/4), 0.0f, -sin(-M_PI/4),  0.0f,
							0.0f,		  1.0f,  0.0f,		    0.0f,
                            sin(-M_PI/4), 0.0f,  cos(-M_PI/4),  0.0f,
                            0.0f,		  0.0f,  0.0f,		    1.0f);
	
	auto translate = glm::mat4(1.0f, 0.0f, 0.0f, -1.0f,
							   0.0f, 1.0f, 0.0f,  0.0f,
							   0.0f, 0.0f, 1.0f,  0.0f,
						       0.0f, 0.0f, 0.0f,  1.0f);

	m_cubeProgram->setUniform(m_translateLocation, translate);
	m_cubeProgram->setUniform(m_rotateLocation, rotate);
	m_cubeProgram->setUniform(m_scaleLocation, scale);

	m_vao->bind();	
    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);
	m_vao->unbind();


    rotate = glm::mat4(cos(M_PI/4), 0.0f, -sin(M_PI/4), 0.0f,
                       0.0f,		1.0f,  0.0f,		0.0f,
                       sin(M_PI/4), 0.0f,  cos(M_PI/4), 0.0f,
                       0.0f,		0.0f,  0.0f,		1.0f);

	translate = glm::mat4(1.0f, 0.0f, 0.0f, 1.0f,
						  0.0f, 1.0f, 0.0f, 0.0f,
						  0.0f, 0.0f, 1.0f, 0.0f,
						  0.0f, 0.0f, 0.0f, 1.0f);

	m_cubeProgram->setUniform(m_translateLocation, translate);
	m_cubeProgram->setUniform(m_rotateLocation, rotate);
	m_cubeProgram->setUniform(m_scaleLocation, scale);

    m_vao->bind();
    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);
    m_vao->unbind();

	m_cubeProgram->release();
	m_fbo->unbind();

	auto fbo = m_targetFramebufferCapability->framebuffer();

	if (!fbo)
		fbo = Framebuffer::defaultFBO();

	fbo->bind();

	m_quadProgram->use();

	m_quadProgram->setUniform(m_textureColorLocation, m_textureColor);
	m_saQuad->draw();

	m_quadProgram->release();

	fbo->unbind();
	//Framebuffer::unbind(GL_FRAMEBUFFER);
}