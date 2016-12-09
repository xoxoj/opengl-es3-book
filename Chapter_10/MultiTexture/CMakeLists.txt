

include_directories(${OPENCV_INCLUDE_DIR})
link_directories(${OPENCV_LIB_DIR})

include_directories(${VISAGE_INCLUDE_DIR})
link_directories(${VISAGE_LIB_DIR})

add_executable( MultiTexture ImageShow.cpp )
target_link_libraries( MultiTexture Common libVisageVision64 
		debug opencv_core2411d optimized opencv_core2411
		debug opencv_highgui2411d optimized opencv_highgui2411
		debug opencv_imgproc2411d optimized opencv_imgproc2411 )

configure_file(basemap.tga ${CMAKE_CURRENT_BINARY_DIR}/basemap.tga COPYONLY)
configure_file(lightmap.tga ${CMAKE_CURRENT_BINARY_DIR}/lightmap.tga COPYONLY)

