cmake_minimum_required( VERSION 2.6 )
project( ES3_Book )

# External 
include_directories( External/Include )

# Common
include_directories( Common/Include )

# Opencv
set(OPENCV_DIR "D:/OpenCV/opencv/build" CACHE STRING "opencv path" FORCE)
set(OPENCV_INCLUDE_DIR ${OPENCV_DIR}/include
					   ${OPENCV_DIR}/include/opencv
					   ${OPENCV_DIR}/include/opencv2
					   CACHE STRING "opencv include path." FORCE)
set(OPENCV_LIB_DIR ${OPENCV_DIR}/x64/vc14/lib CACHE STRING "opencv libs path." FORCE)
set(OPENCV_DLL_DIR ${OPENCV_DIR}/x64/vc14/bin CACHE STRING "opencv dlls path." FORCE)

# Visage
set(VISAGE_DIR "D:/Visage/Visage Technologies 8.1/visageSDK" CACHE STRING "opencv path" FORCE)
set(VISAGE_INCLUDE_DIR ${VISAGE_DIR}/include
					   CACHE STRING "opencv include path." FORCE)
set(VISAGE_LIB_DIR ${VISAGE_DIR}/lib CACHE STRING "opencv libs path." FORCE)
set(VISAGE_DLL_DIR ${VISAGE_DIR}/bin CACHE STRING "opencv dlls path." FORCE)

find_library( OPENGLES3_LIBRARY 
				GLESv2
				"OpenGL ES v3.0 library")
find_library( EGL_LIBRARY
				EGL
				"EGL 1.4 library" )
 
SUBDIRS( Common
         Chapter_2/Hello_Triangle
         Chapter_6/Example_6_3 
         Chapter_6/Example_6_6
         Chapter_6/MapBuffers
         Chapter_6/VertexArrayObjects
         Chapter_6/VertexBufferObjects
         Chapter_7/Instancing
         Chapter_8/Simple_VertexShader
         Chapter_9/Simple_Texture2D 
         Chapter_9/Simple_TextureCubemap
         Chapter_9/MipMap2D
         Chapter_9/TextureWrap
         Chapter_10/MultiTexture
         Chapter_11/MRTs
         Chapter_14/Noise3D
         Chapter_14/ParticleSystem
         Chapter_14/ParticleSystemTransformFeedback 
         Chapter_14/Shadows 
         Chapter_14/TerrainRendering )	
		