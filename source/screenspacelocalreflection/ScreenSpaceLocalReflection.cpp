#include "ScreenSpaceLocalReflection.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

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
#include <gloperate/primitives/ScreenAlignedQuad.h>

#define _USE_MATH_DEFINES 
#include <math.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

ScreenSpaceLocalReflection::ScreenSpaceLocalReflection(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
	: Painter("ScreenSpaceLocalReflections", resourceManager, relDataPath)
	, m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
	, m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
	, m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
	, m_cameraCapability(addCapability(new gloperate::CameraCapability()))
	, m_reflectiveness(0.3)
	, m_treshold(0.002)
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

void ScreenSpaceLocalReflection::initScene()
{
    m_vertices = new Buffer;
    m_vertices->setData(std::vector < float > {		// position.x, position.y, position.z, normal.x, normal.y, normal.z
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
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
    }, gl::GL_STATIC_DRAW);

    m_size = 12 * 3 * sizeof(int);

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

    m_vao->unbind();
}

void ScreenSpaceLocalReflection::initPrograms()
{
    auto path = std::string{"data/screenspacelocalreflection/"};

    m_sceneProgram = make_ref<Program>();
    m_sceneProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, path + "scene.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, path + "scene.frag")
        );

    m_quadProgram = make_ref<Program>();
    m_quadProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, path + "quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, path + "quad.frag")
        );

    m_transformLocation = m_sceneProgram->getUniformLocation("transform");
	m_viewLocation = m_sceneProgram->getUniformLocation("view");
	m_projectionLocation = m_sceneProgram->getUniformLocation("projection");
    m_translateLocation = m_sceneProgram->getUniformLocation("translate");
    m_rotateLocation = m_sceneProgram->getUniformLocation("rotate");
    m_scaleLocation = m_sceneProgram->getUniformLocation("scale");

	m_reflectivenessLocation = m_sceneProgram->getUniformLocation("reflectiveness");

	m_quadProgram->use();

    m_quadTransformLocation = m_quadProgram->getUniformLocation("transform");
    m_viewportLocation = m_quadProgram->getUniformLocation("viewport");
    m_eyeLocation = m_quadProgram->getUniformLocation("eye");
	m_tresholdLocation = m_quadProgram->getUniformLocation("treshold");

    m_fboColorAttachmentLocation = m_quadProgram->getUniformLocation("fboTexture");
    m_quadProgram->setUniform(m_fboColorAttachmentLocation, 0);

	m_normalAttachmentLocation = m_quadProgram->getUniformLocation("normalTexture");
	m_quadProgram->setUniform(m_normalAttachmentLocation, 1);

    m_depthLocation = m_quadProgram->getUniformLocation("depthTexture");
    m_quadProgram->setUniform(m_depthLocation, 2);

	m_positionAttachmentLocation = m_quadProgram->getUniformLocation("positionTexture");
	m_quadProgram->setUniform(m_positionAttachmentLocation, 3);

	m_reflectivenessAttachmentLocation = m_quadProgram->getUniformLocation("reflectivenessTexture");
	m_quadProgram->setUniform(m_reflectivenessAttachmentLocation, 4);

	m_quadProgram->release();

	m_saQuad = make_ref<gloperate::ScreenAlignedQuad>(m_quadProgram);
}

void ScreenSpaceLocalReflection::initFramebuffer()
{
    m_fboColorAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_fboColorAttachment->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

	m_fboNormalAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_fboNormalAttachment->image2D(0, gl::GL_RGB, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fboDepthAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_fboDepthAttachment->image2D(0, gl::GL_DEPTH_COMPONENT, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

	m_fboPositionAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_fboPositionAttachment->image2D(0, gl::GL_RGB, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);

	m_fboReflectivenessAttachment = Texture::createDefault(GL_TEXTURE_2D);
	m_fboReflectivenessAttachment->image2D(0, gl::GL_RGB, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fbo = make_ref<Framebuffer>();
    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_fboColorAttachment);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT1, m_fboNormalAttachment);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT2, m_fboPositionAttachment);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT3, m_fboReflectivenessAttachment);
	m_fbo->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_fboDepthAttachment);

    m_fbo->printStatus(true);
}

void ScreenSpaceLocalReflection::initProperties()
{
	addProperty<float>("reflectiveness", this,
		&ScreenSpaceLocalReflection::reflectiveness, &ScreenSpaceLocalReflection::setReflectiveness)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.05f },
			{ "precision", 2u }
	});

	addProperty<float>("treshold", this,
		&ScreenSpaceLocalReflection::treshold, &ScreenSpaceLocalReflection::setTreshold)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 0.01f },
			{ "step", 0.001f },
			{ "precision", 3u }
	});
}

float ScreenSpaceLocalReflection::reflectiveness() const
{
	return m_reflectiveness;
}

void ScreenSpaceLocalReflection::setReflectiveness(float reflectiveness)
{
	m_reflectiveness = reflectiveness;
}

float ScreenSpaceLocalReflection::treshold() const
{
	return m_treshold;
}

void ScreenSpaceLocalReflection::setTreshold(float treshold)
{
	m_treshold = treshold;
}

void ScreenSpaceLocalReflection::onInitialize()
{
	globjects::init();

#ifdef __APPLE__
	Shader::clearGlobalReplacements();
	Shader::globalReplace("#version 140", "#version 150");

	debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

	m_grid = new gloperate::AdaptiveGrid{};
	m_grid->setColor({ 0.6f, 0.6f, 0.6f });

	initProperties();
    initPrograms();
	initFramebuffer();
    initScene();

    glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
//    glClearDepth(1.0f);

	setupProjection();
}

void ScreenSpaceLocalReflection::drawScene(const glm::vec3 & eye, const glm::mat4 & transform)
{
    m_sceneProgram->use();
    m_sceneProgram->setUniform(m_transformLocation, transform);
	m_sceneProgram->setUniform(m_viewLocation, m_cameraCapability->view());
	m_sceneProgram->setUniform(m_projectionLocation, m_projectionCapability->projection());

	auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	auto rotate = glm::rotate(glm::mat4(1.0f), (float)-M_PI_4, glm::vec3(0.0f, 1.0f, 0.0f));
	auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    m_sceneProgram->setUniform(m_translateLocation, translate);
    m_sceneProgram->setUniform(m_rotateLocation, rotate);
    m_sceneProgram->setUniform(m_scaleLocation, scale);
//	m_sceneProgram->setUniform(m_reflectivenessLocation, 0.0f);
    m_sceneProgram->setUniform(m_reflectivenessLocation, m_reflectiveness);

    m_vao->bind();
    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);

	rotate = glm::rotate(glm::mat4(1.0f), (float)M_PI_4, glm::vec3(0.0f, 1.0f, 0.0f));
	translate = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    m_sceneProgram->setUniform(m_translateLocation, translate);
    m_sceneProgram->setUniform(m_rotateLocation, rotate);

    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);

	scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 1.5f, 1.5f));
	rotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	translate = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    m_sceneProgram->setUniform(m_translateLocation, translate);
    m_sceneProgram->setUniform(m_rotateLocation, rotate);
    m_sceneProgram->setUniform(m_scaleLocation, scale);

    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);

	scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 0.001f, 3.0f));
	rotate = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.7f, 0.0f));

    m_sceneProgram->setUniform(m_translateLocation, translate);
    m_sceneProgram->setUniform(m_rotateLocation, rotate);
    m_sceneProgram->setUniform(m_scaleLocation, scale);

    m_vao->drawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT);

    m_vao->unbind();
    m_sceneProgram->release();
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

		m_fboColorAttachment->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_fboDepthAttachment->image2D(0, gl::GL_DEPTH_COMPONENT, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_fboNormalAttachment->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
        m_fboPositionAttachment->image2D(0, gl::GL_RGB, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);
		m_fboReflectivenessAttachment->image2D(0, gl::GL_RGB, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);
	
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
//    m_grid->draw();

    m_fbo->bind(GL_FRAMEBUFFER);
    m_fboColorAttachment->bindActive(GL_TEXTURE0);
	m_fboNormalAttachment->bindActive(GL_TEXTURE1);
	m_fboDepthAttachment->bindActive(GL_TEXTURE2);
	m_fboPositionAttachment->bindActive(GL_TEXTURE3);
	m_fboReflectivenessAttachment->bindActive(GL_TEXTURE4);
	m_fbo->setDrawBuffers({ gl::GL_COLOR_ATTACHMENT0, gl::GL_COLOR_ATTACHMENT1, gl::GL_COLOR_ATTACHMENT2, gl::GL_COLOR_ATTACHMENT3});

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene(eye, transform);
//    drawScene(m_cameraCapability->center(), transform);

    m_fbo->unbind();



    fbo->bind(GL_FRAMEBUFFER);

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    m_quadProgram->use();

//    m_quadProgram->setUniform(m_fboColorAttachmentLocation, m_fboColorAttachment);
//    m_quadProgram->setUniform(m_depthLocation, m_fboDepthAttachment);

//    m_quadProgram->use();
//    m_quadProgram->setUniform(m_fboColorAttachmentLocation, m_fboColorAttachment);

//    m_quadProgram->setUniform(m_fboColorAttachmentLocation, m_fboColorAttachment);
//    m_quadProgram->setUniform(m_depthLocation, m_fboDepthAttachment);
    m_quadProgram->setUniform(m_quadTransformLocation, transform);
    m_quadProgram->setUniform(m_viewportLocation, glm::vec2(m_viewportCapability->width(), m_viewportCapability->height()));
    m_quadProgram->setUniform(m_eyeLocation, eye);
	m_quadProgram->setUniform(m_tresholdLocation, m_treshold);
    m_saQuad->draw();

	Framebuffer::unbind(GL_FRAMEBUFFER);
}
