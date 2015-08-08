#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>

#include <gloperate/painter/Painter.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>


namespace globjects
{
    class Framebuffer;
    class Program;
    class Texture;
}

namespace gloperate
{
    class AdaptiveGrid;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class ScreenSpaceLocalReflection : public gloperate::Painter
{
public:
    ScreenSpaceLocalReflection(gloperate::ResourceManager & resourceManager, const std::string & relDataPath);
	virtual ~ScreenSpaceLocalReflection();

    void setupProjection();

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;
	void initPrograms();
	void initScene();
    void initFramebuffer();
	void initProperties();
	void renderScene();
	void finalize();
    void drawScene(const glm::vec3 & eye, const glm::mat4 & transform);
	float planeReflectiveness() const;
	void setPlaneReflectiveness(float reflectiveness);
	float objectsReflectiveness() const;
	void setObjectsReflectiveness(float reflectiveness);
	float maxDepthDifference() const;
	void setMaxDepthDifference(float difference);
	float selfReflectionThreshold() const;
	void setSelfReflectionThreshold(float threshold);

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    globjects::ref_ptr<globjects::Program> m_sceneProgram;
    globjects::ref_ptr<globjects::Program> m_quadProgram;
    gl::GLint m_transformLocation;
	gl::GLint m_viewLocation;
	gl::GLint m_projectionLocation;
    gl::GLint m_viewportLocation;
    gl::GLint m_quadTransformLocation;
    gl::GLint m_eyeLocation;
    gl::GLint m_translateLocation;
	gl::GLint m_rotateLocation;
    gl::GLint m_scaleLocation;
	gl::GLint m_reflectivenessLocation;
	gl::GLint m_maxDepthDifferenceLocation;
	gl::GLint m_selfReflectionThresholdLocation;
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_indices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;   
	globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_saQuad;

	int m_size;
	float m_planeReflectiveness;
	float m_objectsReflectiveness;
	float m_maxDepthDifference;
	float m_selfReflectionThreshold;

    gl::GLint m_fboColorAttachmentLocation;
	gl::GLint m_normalAttachmentLocation;
    gl::GLint m_depthLocation;
	gl::GLint m_positionAttachmentLocation;
	gl::GLint m_reflectivenessAttachmentLocation;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_fboColorAttachment;
	globjects::ref_ptr<globjects::Texture> m_fboNormalAttachment;
    globjects::ref_ptr<globjects::Texture> m_fboDepthAttachment;
	globjects::ref_ptr<globjects::Texture> m_fboPositionAttachment;
	globjects::ref_ptr<globjects::Texture> m_fboReflectivenessAttachment;
};
