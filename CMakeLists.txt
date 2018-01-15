
# raspistill/raspivid/raspiyuv

SET(COMPILE_DEFINITIONS -Werror)

include_directories(bcm_host/include)
include_directories(.)
include_directories(sm)
include_directories(/opt/vc/include)

link_directories(/opt/vc/lib)


set (GL_SCENE_SOURCES
   gl_scenes/models.c
   gl_scenes/mirror.c
   gl_scenes/yuv.c
   gl_scenes/sobel.c
   gl_scenes/square.c
   gl_scenes/teapot.c
   gl_scenes/vcsm_square.c)

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCLI.c
   RaspiPreview.c)

add_executable(raspividyuv  ${COMMON_SOURCES} grabimage.c sockserver.c picworker.c)

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

target_link_libraries(raspividyuv   ${MMAL_LIBS} vcos vcsm bcm_host m pthread)

#install(TARGETS raspistill raspiyuv raspivid raspividyuv RUNTIME DESTINATION bin)
