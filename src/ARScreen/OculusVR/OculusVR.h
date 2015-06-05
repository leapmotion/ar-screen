#pragma once

#include <Eigen/Dense>
#include "Leap/GL/GLHeaders.h"
#include "OVR.h"
#include "OVR_Kernel.h"

#if defined(OVR_OS_LINUX)
#include "X11/Xlibint.h"
#endif

using namespace Leap::GL;

/// Used to configure slave GL rendering (i.e. for devices created externally).
typedef struct ovrGLConfigData_s
{
  /// General device settings.
  ovrRenderAPIConfigHeader Header;

#if defined(OVR_OS_WIN32)
  /// The optional window handle. If unset, rendering will use the current window.
  HWND Window;
  /// The optional device context. If unset, rendering will use a new context.
  HDC  DC;
#elif defined(OVR_OS_LINUX)
  /// The optional display. If unset, rendering will use the current display.
  _XDisplay* Disp;
  /// The optional window. If unset, rendering will use the current window.
  Window     Win;
#endif
} ovrGLConfigData;

/// Contains OpenGL-specific rendering information.
union ovrGLConfig
{
  /// General device settings.
  ovrRenderAPIConfig Config;
  /// OpenGL-specific settings.
  ovrGLConfigData    OGL;
};

/// Used to pass GL eye texture data to ovrHmd_EndFrame.
typedef struct ovrGLTextureData_s
{
  /// General device settings.
  ovrTextureHeader Header;
  /// The OpenGL name for this texture.
  GLuint           TexId;
} ovrGLTextureData;

/// Contains OpenGL-specific texture information.
typedef union ovrGLTexture_s
{
  /// General device settings.
  ovrTexture       Texture;
  /// OpenGL-specific settings.
  ovrGLTextureData OGL;
} ovrGLTexture;


// Provides an interface for retrieving tracking data and correcting distortion for an Oculus VR headset.
// Instructions for use:
// 1) Create an instance of OculusVR in your app
// 2) Call Init() and verify it returns true
// 3) Inside your app's main render loop:
//   a) Call BeginFrame()
//   b) Retrieve the eye viewport information with EyeViewport and call glViewport
//   c) Retrieve the eye transform using EyeProjection, EyeTranslation, and EyeRotation
//   d) Set up the OpenGL projection and modelview matrices appropriately
//   e) Render your geometry
//   f) Call EndFrame()
class OculusVR {

public:
  bool isDebug();

  int GetHMDWidth();

  int GetHMDHeight();

  void InitGlew();

  bool InitHMD();
  bool Init();
  void Destroy();

  void BeginFrame();
  void EndFrame();

  void DismissHealthWarning();

  const ovrRecti& EyeViewport(int eye) const {
    return m_EyeRenderViewport[eye];
  }

  Eigen::Matrix4f EyeView(int eye) const {
    return Eigen::Matrix4f(&m_EyeView[eye].Transposed().M[0][0]);
  }

  Eigen::Matrix4f EyeProjection(int eye) const {
    return Eigen::Matrix4f(&m_EyeProjection[eye].Transposed().M[0][0]);
  }

  Eigen::Vector3f EyePosition(int eye) const {
    return Eigen::Vector3f(m_EyePosition[eye].x, m_EyePosition[eye].y, m_EyePosition[eye].z);
  }

  Eigen::Matrix4f EyeRotation(int eye) const {
    return Eigen::Matrix4f(&m_EyeRotation[eye].Transposed().M[0][0]);
  }

  ovrHmd& GetHMD() {
    return m_HMD;
  }

  ovrVector2i GetWindowsPos() {
    return m_HMD->WindowsPos;
  }

  void GetFramebufferStatus(GLenum status);

#if defined(OVR_OS_WIN32)
  typedef HWND WindowHandle;
#elif defined(OVR_OS_MAC)
  typedef void* WindowHandle; // NSWindow
#elif defined(OVR_OS_LINUX)
  typedef Window WindowHandle;
#endif

  void SetWindow(const WindowHandle& window) {
    m_Window = window;
  }

private:

  void Shutdown();

  ovrHmd m_HMD = nullptr;
  bool m_Debug;

  int m_width;
  int m_height;

  GLuint m_FrameBuffer;
  GLuint m_Texture;
  GLuint m_RenderBuffer;

  ovrRecti m_EyeRenderViewport[2];
  ovrGLTexture m_EyeTexture[2];
  ovrPosef m_EyeRenderPose[2];
  ovrEyeRenderDesc m_EyeRenderDesc[2];

  OVR::Matrix4f m_EyeProjection[2];
  OVR::Matrix4f m_EyeView[2];

  OVR::Vector3f m_EyePosition[2];
  OVR::Matrix4f m_EyeRotation[2];

  WindowHandle m_Window;
};
