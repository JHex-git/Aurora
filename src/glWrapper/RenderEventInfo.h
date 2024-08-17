#pragma once
// std include
#include <string>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include


namespace Aurora
{

class RenderEventInfo
{
public:
    RenderEventInfo(std::string name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, EventMessageID, -1, name.c_str());
    }

    RenderEventInfo(const char* name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, EventMessageID, -1, name);
    }

    ~RenderEventInfo()
    {
        glPopDebugGroup();
    }

    RenderEventInfo(const RenderEventInfo&) = delete;
    RenderEventInfo& operator=(const RenderEventInfo&) = delete;

    RenderEventInfo(RenderEventInfo&&) = delete;
    RenderEventInfo& operator=(RenderEventInfo&&) = delete;

    const static unsigned int EventMessageID = 1;
};

#define SCOPED_RENDER_EVENT(event_name) RenderEventInfo __render_event_info(event_name)
} // namespace Aurora