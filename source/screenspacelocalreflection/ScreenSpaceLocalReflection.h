#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>

#include <gloperate/painter/Painter.h>


namespace globjects
{
    class Program;
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
    ScreenSpaceLocalReflection(gloperate::ResourceManager & resourceManager);
	virtual ~ScreenSpaceLocalReflection();

    void setupProjection();

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    globjects::ref_ptr<globjects::Program> m_program;
    gl::GLint m_transformLocation;
    gl::GLint m_translateLocation;
	gl::GLint m_rotateLocation;
    gl::GLint m_scaleLocation;
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_indices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	int m_size;
};
