#pragma once

#include "Leap/GL/Shader.h"
#include "ScopeGuard.h"

inline void ShaderBind (const Leap::GL::Shader &shader) { shader.Bind(); }
inline void ShaderUnbind (const Leap::GL::Shader &shader) { shader.Unbind(); }

// Convenience typedef for a ScopeGuard type that will Bind and Unbind a Shader object.
typedef ScopeGuard<Leap::GL::Shader,ShaderBind,ShaderUnbind> ShaderBindingScopeGuard;
