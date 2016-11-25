// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// MultiTexture.c
//
//    This is an example that draws a quad with a basemap and
//    lightmap to demonstrate multitexturing.
//
#include <stdlib.h>
#include "esUtil.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <FaceData.h>
#include <VisageFeaturesDetector.h>
#include <iostream>
#include <math.h>

using namespace VisageSDK;
namespace VisageSDK
{
	void __declspec(dllimport) initializeLicenseManager( const char *licenseKeyFileFolder );
}
int featureDetect( IplImage *image, float* fLoc ) {
	int fNum = 0;
	char vlcFile[] = "D:/Visage/Visage Technologies 8.1/visageSDK/Samples/data";
	VisageSDK::initializeLicenseManager( vlcFile );

	static const int MAX_FACE_NUM = 10;
	FaceData data[MAX_FACE_NUM];
	VisageFeaturesDetector* detector_ = new VisageFeaturesDetector( );
	std::string strpath = "D:/Visage/Visage Technologies 8.1/visageSDK/Samples/data/bdtsdata";
	bool init = detector_->Initialize( strpath.c_str( ) );
	if (!init) {
		printf( "visage face features detector init failed.\n" );
		delete detector_; detector_ = 0; return fNum;
	}
	int n_faces = detector_->detectFacialFeatures( (VsImage*)image, data, MAX_FACE_NUM );

	if (n_faces > 0) {
		fNum = 14;
		VisageSDK::FeaturePoint *fp; VisageSDK::FDP *fdp = data[0].featurePoints2D;
		if (data[0].eyeClosure[0]) {
			fp = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 5 ));
			fLoc[0] = fp->pos[0];
			fLoc[1] = 1 - fp->pos[1];
		}
		else{
			VisageSDK::FeaturePoint *leftCorner; VisageSDK::FeaturePoint *rightCorner;
			leftCorner = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 7 ));
			rightCorner = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 11 ));
			if (leftCorner->defined) {
				fLoc[0] = (leftCorner->pos[0] + rightCorner->pos[0]) / 2.0f;
				fLoc[1] = 1 - ((leftCorner->pos[1] + rightCorner->pos[1]) / 2.0f);
			}
			else {
				fLoc[0] = 0.0f;
				fLoc[1] = 0.0f;
			}
		}
		if (data[0].eyeClosure[1]) {
			fp = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 6 ));
			fLoc[2] = fp->pos[0];
			fLoc[3] = 1-fp->pos[1];
		}
		else {
			VisageSDK::FeaturePoint *leftCorner; VisageSDK::FeaturePoint *rightCorner;
			leftCorner = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 8 ));
			rightCorner = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( 3, 12 ));
			if (leftCorner->defined) {
				fLoc[2] = (leftCorner->pos[0] + rightCorner->pos[0]) / 2.0f;
				fLoc[3] = 1 - ((leftCorner->pos[1] + rightCorner->pos[1]) / 2.0f);
			}
			else {
				fLoc[2] = 0.0f;
				fLoc[3] = 0.0f;
			}
		}
		static int indices[] = {
			13,12,	13,14,	2,1,	13,13,	13,11,
			9,3,	8,1,	8,2,	13,3,	13,4,
			13,5,	13,6
		};
		int num = sizeof( indices ) / sizeof( int );
		for (int i = 0; i < num; ++i,++i) {
			fp = const_cast<VisageSDK::FeaturePoint*>(&fdp->getFP( indices[i], indices[i+1] ));
			if (fp->defined) { fLoc[i+4] = fp->pos[0]; fLoc[i+5] = 1 - fp->pos[1]; }
			else { fLoc[i+4] = 0.0f; fLoc[i+5] = 0.0f; }
		}
		for (int i = 0; i < num+4; ++i, ++i) {
			printf( "x = %.6f, y = %.6f.\n", fLoc[i], fLoc[i + 1] );
		}
	}
	delete detector_; detector_ = 0; return fNum;
}

#define NO_IMPL			1
#define GPUIMAGE_IMPL	2
#define FACEU_IMPL		3

#define FSHADER_IMPL FACEU_IMPL

enum BEAUTY{ FISRT=1,SECOND,THIRD,FORTH,FIFTH};

extern "C" {
	typedef struct
	{
		// Handle to a program object
		GLuint programObject;

		// Sampler locations
		GLint baseMapLoc;
		GLint backMapLoc;
		GLint fluidMapLoc;
		GLint fluidUvOffsetLoc;

		// Texture handle
		GLuint baseMapTexId;
		GLuint backMapTexId;
		GLuint fluidMapTexId;

		GLint texelWidthOffsetLoc;
		GLint texelHeightOffsetLoc;
		GLfloat texelWidthOffset;
		GLfloat texelHeightOffset;

		GLint iternumLoc;
		GLint aa_coefLoc;
		GLint mixcoefLoc;

		IplImage *image;

		float*	fLoc;
		int		fNum;
		GLint	location4Loc;
		GLint	location5Loc;
		GLint	scaleLoc;
		GLint	aspectRatioLoc;
		GLfloat scale;
		GLfloat aspectRatio;
		GLint	pointsLoc;

	} UserData;

	///
	// Load texture from disk
	//
	GLuint LoadTexture( void *ioContext, char *fileName )
	{
		int width,
			height;

		char *buffer = esLoadTGA( ioContext, fileName, &width, &height );
		GLuint texId;

		if (buffer == NULL)
		{
			esLogMessage( "Error loading (%s) image.\n", fileName );
			return 0;
		}

		glGenTextures( 1, &texId );
		glBindTexture( GL_TEXTURE_2D, texId );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		free( buffer );

		return texId;
	}



	///
	// Load texture from disk
	//
	GLuint LoadTextureImage( void *ioContext, IplImage* tmp )
	{
		int width,
			height;

		cv::Mat image = cv::cvarrToMat( tmp, false );
		//cv::imshow( "image", image ); cv::waitKey( 0 );
		cvtColor( image, image, CV_BGR2RGB );
		uchar* buffer = image.data;
		width = image.cols; height = image.rows;
		//int rowLen = image.step / image.elemSize( );
		glPixelStorei( GL_PACK_ALIGNMENT, (image.step & 3) ? 1 : 4 );
		glPixelStorei( GL_PACK_ROW_LENGTH, image.step / image.elemSize( ) );
		
		GLuint texId;

		if (buffer == NULL)
		{
			esLogMessage( "Error loading (%s) image.\n" );
			return 0;
		}

		glGenTextures( 1, &texId );
		glBindTexture( GL_TEXTURE_2D, texId );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glBindTexture( GL_TEXTURE_2D, 0 );

		return texId;
	}

	///
	// Load texture from disk
	//
	GLuint LoadTextureImageString( void *ioContext, const char* filename )
	{
		int width, height;

		IplImage *image;
		image = cvLoadImage( filename, CV_LOAD_IMAGE_UNCHANGED );
		if (!image) { printf( "Load image:%s failure.\n", filename ); getchar( ); return 0; }

		cv::Mat mat = cv::cvarrToMat( image, false );
		//cv::imshow( "background image", mat ); cv::waitKey( 0 );
		if (mat.channels( ) == 3) {
			cvtColor( mat, mat, CV_BGR2RGB );
		}
		//std::cout << mat.colRange( 10,13 ).rowRange( 10,13 ) << std::endl;
		uchar* buffer = mat.data;
		width = mat.cols; height = mat.rows;
		int rowLen = mat.step / mat.elemSize( );
		int align = mat.step & 3;
		glPixelStorei( GL_PACK_ALIGNMENT, (mat.step & 3) ? 1 : 4 );
		glPixelStorei( GL_PACK_ROW_LENGTH, mat.step / mat.elemSize( ) );

		GLuint texId;

		if (buffer == NULL)
		{
			esLogMessage( "Error loading (%s) image.\n" );
			return 0;
		}

		glGenTextures( 1, &texId );
		glBindTexture( GL_TEXTURE_2D, texId );
		if (mat.channels( ) == 4) {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
		}
		else if (mat.channels( ) == 3) {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer );
		}
		else if (mat.channels( ) == 1) {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer );
		}
		else {
			printf( "Unknowned image type.\n" );
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glBindTexture( GL_TEXTURE_2D, 0 );

		cvReleaseImage( &image );

		return texId;
	}

	///
	// Initialize the shader and program object
	//
	int Init( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
#if FSHADER_IMPL == GPUIMAGE_IMPL
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 a_position;   \n"
			"layout(location = 1) in vec2 a_texCoord;   \n"
			"const int GAUSSIAN_SAMPLES = 9;			\n"
			"out vec2 v_texCoord;                       \n"
			"out vec2 blurCoordinates[GAUSSIAN_SAMPLES];	\n"
			"uniform float texelWidthOffset;		\n"
			"uniform float texelHeightOffset;		\n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = a_position;			\n"
			"	v_texCoord = a_texCoord;			\n"
			"	int multiplier = 0;					\n"
			"	vec2 blurStep;						\n"
			"	vec2 singleStepOffset = vec2( texelWidthOffset, texelHeightOffset );	\n"
			"	for (int i = 0; i < GAUSSIAN_SAMPLES; i++)		\n"
			"	{									\n"
			"		multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));	\n"
			"		blurStep = float( multiplier ) * singleStepOffset;	\n"
			"		blurCoordinates[i] = a_texCoord + blurStep;			\n"
			"	}									\n"
			"}										\n";

		char fShaderStr[] =
			"#version 300 es                                     \n"
			"precision mediump float;                            \n"
			"layout(location = 0) out vec4 outColor;             \n"
			"const int GAUSSIAN_SAMPLES = 9;					\n"
			"in vec2 v_texCoord;                                 \n"
			"in vec2 blurCoordinates[GAUSSIAN_SAMPLES];			\n"
			"uniform sampler2D inputImageTexture;				\n"
			"mediump float distanceNormalizationFactor = 4.0;	\n"
			"void main( )										\n"
			"{													\n"
			"	lowp vec4  centralColor;						\n"
			"	lowp float gaussianWeightTotal;					\n"
			"	lowp vec4  sum;									\n"
			"	lowp vec4  sampleColor;							\n"
			"	lowp float distanceFromCentralColor;			\n"
			"	lowp float gaussianWeight;						\n"
			"	centralColor = texture( inputImageTexture, blurCoordinates[4] );	\n"
			"	gaussianWeightTotal = 0.18;						\n"
			"	sum = centralColor * 0.18;						\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[0] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.05 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;			\n"
			"	sum += sampleColor * gaussianWeight;			\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[1] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.09 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[2] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.12 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[3] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.15 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[5] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.15 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[6] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.12 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[7] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.09 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	sampleColor = texture( inputImageTexture, blurCoordinates[8] );		\n"
			"	distanceFromCentralColor = min( distance( centralColor, sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
			"	gaussianWeight = 0.05 * (1.0 - distanceFromCentralColor);			\n"
			"	gaussianWeightTotal += gaussianWeight;								\n"
			"	sum += sampleColor * gaussianWeight;								\n"
			"																		\n"
			"	outColor = sum / gaussianWeightTotal;								\n"
			"}																		\n";
#elif FSHADER_IMPL == FACEU_IMPL
		char vShaderStr[] =
			"#version 300 es                            \n"
			"#define x_a 480.0							\n"
			"#define y_a 640.0							\n"
			"layout(location = 0) in vec4 a_position;   \n"
			"layout(location = 1) in vec2 a_texCoord;   \n"
			"out vec2 textureCoordinate;                \n"
			"const int GAUSS_SAMPLES = 20;				\n"
			"out vec2 blurCoord[GAUSS_SAMPLES];			\n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = a_position;			\n"
			"	textureCoordinate = a_texCoord;		\n"
			"	float mul_x = 2.0 / x_a;						\n"
			"	float mul_y = 2.0 / y_a;						\n"
			"													\n"
			"	blurCoord[0] = textureCoordinate + vec2( 0.0 * mul_x, -10.0 * mul_y );	\n"
			"	blurCoord[1] = textureCoordinate + vec2( 5.0 * mul_x, -8.0 * mul_y );	\n"
			"	blurCoord[2] = textureCoordinate + vec2( 8.0 * mul_x, -5.0 * mul_y );	\n"
			"	blurCoord[3] = textureCoordinate + vec2( 10.0 * mul_x, 0.0 * mul_y );	\n"
			"	blurCoord[4] = textureCoordinate + vec2( 8.0 * mul_x, 5.0 * mul_y );	\n"
			"	blurCoord[5] = textureCoordinate + vec2( 5.0 * mul_x, 8.0 * mul_y );	\n"
			"	blurCoord[6] = textureCoordinate + vec2( 0.0 * mul_x, 10.0 * mul_y );	\n"
			"	blurCoord[7] = textureCoordinate + vec2( -5.0 * mul_x, 8.0 * mul_y );	\n"
			"	blurCoord[8] = textureCoordinate + vec2( -8.0 * mul_x, 5.0 * mul_y );	\n"
			"	blurCoord[9] = textureCoordinate + vec2( -10.0 * mul_x, 0.0 * mul_y );	\n"
			"	blurCoord[10] = textureCoordinate + vec2( -8.0 * mul_x, -5.0 * mul_y );	\n"
			"	blurCoord[11] = textureCoordinate + vec2( -5.0 * mul_x, -8.0 * mul_y );	\n"
			"																			\n"
			"	mul_x = 1.2 / x_a;														\n"
			"	mul_y = 1.2 / y_a;														\n"
			"																			\n"
			"	blurCoord[12] = textureCoordinate + vec2( 0.0 * mul_x, -6.0 * mul_y );	\n"
			"	blurCoord[13] = textureCoordinate + vec2( -4.0 * mul_x, -4.0 * mul_y );	\n"
			"	blurCoord[14] = textureCoordinate + vec2( -6.0 * mul_x, 0.0 * mul_y );	\n"
			"	blurCoord[15] = textureCoordinate + vec2( -4.0 * mul_x, 4.0 * mul_y );	\n"
			"	blurCoord[16] = textureCoordinate + vec2( 0.0 * mul_x, 6.0 * mul_y );	\n"
			"	blurCoord[17] = textureCoordinate + vec2( 4.0 * mul_x, 4.0 * mul_y );	\n"
			"	blurCoord[18] = textureCoordinate + vec2( 6.0 * mul_x, 0.0 * mul_y );	\n"
			"	blurCoord[19] = textureCoordinate + vec2( 4.0 * mul_x, -4.0 * mul_y );	\n"
			"}										\n";

		char fShaderStr[] =
			"#version 300 es                                     \n"
			"#define saturateMatrix mat3(1.1102,-0.0598,-0.061,-0.0774,1.0826,-0.1186,-0.0228,-0.0228,1.1772)	\n"
			"precision lowp float;								\n"
			"precision lowp int;								\n"		
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"uniform int iternum;								\n"
			"uniform float aa_coef;								\n"
			"uniform float mix_coef;							\n"		
			"mediump float distanceNormalizationFactor = 4.0;	\n"
			"in highp vec2 textureCoordinate;							\n"
			"const int GAUSS_SAMPLES = 20;						\n"
			"in highp vec2 blurCoord[GAUSS_SAMPLES];					\n"
			"													\n"
			"void main( ) {										\n"
			"	vec3 centralColor;								\n"
			"																			\n"
			"	float central;															\n"
			"	float gaussianWeightTotal;												\n"
			"	float sum;																\n"
			"	float sampleColor;														\n"
				"	float distanceFromCentralColor;									\n"
				"	float gaussianWeight;											\n"
				"																	\n"
				"	central = texture( inputImageTexture, textureCoordinate ).g;	\n"
				"	gaussianWeightTotal = 0.2;										\n"
				"	sum = central * 0.2;											\n"
				"																	\n"
				"	for (int i = 0; i < 12; i++) {									\n"
				"		sampleColor = texture( inputImageTexture, blurCoord[i] ).g;	\n"
				"		distanceFromCentralColor = min( abs( central - sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
				"		gaussianWeight = 0.05 * (1.0 - distanceFromCentralColor);	\n"
				"		gaussianWeightTotal += gaussianWeight;						\n"
				"		sum += sampleColor * gaussianWeight;						\n"
				"	}																\n"
				"	for (int i = 12; i < GAUSS_SAMPLES; i++) {						\n"
				"		sampleColor = texture( inputImageTexture, blurCoord[i] ).g;	\n"
				"		distanceFromCentralColor = min( abs( central - sampleColor ) * distanceNormalizationFactor, 1.0 );	\n"
				"		gaussianWeight = 0.1 * (1.0 - distanceFromCentralColor);	\n"
				"		gaussianWeightTotal += gaussianWeight;						\n"
				"		sum += sampleColor * gaussianWeight;								\n"
				"	}																\n"
				"																	\n"
				"	sum = sum / gaussianWeightTotal;								\n"
				"	centralColor = texture( inputImageTexture, textureCoordinate ).rgb;	\n"
				"	sampleColor = centralColor.g - sum + 0.5;							\n"
				"	for (int i = 0; i < iternum; ++i) {									\n"
				"		if (sampleColor <= 0.5) {									\n"
				"			sampleColor = sampleColor * sampleColor * 2.0;			\n"
				"		}															\n"
				"		else {														\n"
				"			sampleColor = 1.0 - ((1.0 - sampleColor)*(1.0 - sampleColor) * 2.0);	\n"
				"		}															\n"
				"	}																\n"
				"																	\n"
				"	float aa = 1.0 + pow( centralColor.g, 0.3 )*aa_coef;			\n"
				"	vec3 smoothColor = centralColor*aa - vec3( sampleColor )*(aa - 1.0);			\n"
				"	smoothColor = clamp( smoothColor, vec3( 0.0 ), vec3( 1.0 ) );					\n"
				"	smoothColor = mix( centralColor, smoothColor, pow( centralColor.g, 0.33 ) );	\n"
				"	smoothColor = mix( centralColor, smoothColor, pow( centralColor.g, mix_coef ) );\n"
				"	outColor = vec4( pow( smoothColor, vec3( 0.96 ) ), 1.0 );	\n"
				"	vec3 satcolor = outColor.rgb * saturateMatrix;				\n"
				"	outColor.rgb = mix( outColor.rgb, satcolor, 0.23 );			\n"
				"}																\n";
#else
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 a_position;   \n"
			"layout(location = 1) in vec2 a_texCoord;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = a_position;			\n"
			"	textureCoordinate = a_texCoord;		\n"
			"}										\n";

		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision mediump float;                           \n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"in vec2 textureCoordinate;							\n"
			"													\n"
			"void main( ) {										\n"
			"	outColor = texture( inputImageTexture, textureCoordinate  );	\n"
			"}													\n";
#endif


		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );

#if FSHADER_IMPL == GPUIMAGE_IMPL
		userData->texelWidthOffsetLoc = glGetUniformLocation( userData->programObject, "texelWidthOffset" );
		userData->texelHeightOffsetLoc = glGetUniformLocation( userData->programObject, "texelHeightOffset" );
#elif FSHADER_IMPL == FACEU_IMPL
		userData->iternumLoc = glGetUniformLocation( userData->programObject, "iternum" );
		userData->aa_coefLoc = glGetUniformLocation( userData->programObject, "aa_coef" );
		userData->mixcoefLoc = glGetUniformLocation( userData->programObject, "mix_coef" );
#endif

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData,userData->image);
		userData->texelHeightOffset = 2.0 / userData->image->height;
		userData->texelWidthOffset = 2.0 / userData->image->width;

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	/*
	* Init bigeye and slimface
	*/
	int InitBigEye( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate;		\n"
			"}										\n";
#if 0
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"								\n"
			"uniform lowp vec2 location4;	\n"
			"uniform lowp vec2 location5;	\n"
			"								\n"
			"const highp vec2 p_eyea = location4;		\n"
			"const highp vec2 p_eyeb = location5;		\n"
			"								\n"
			"#define x_a 0.60				\n"
			"#define y_a 0.45				\n"
			"								\n"
			"void main( ){					\n"
			"	vec2 newCoord = vec2( textureCoordinate.x*x_a, textureCoordinate.y*y_a );	\n"
			"	vec2 eyea = vec2( p_eyea.x * x_a, p_eyea.y * y_a );			\n"
			"	vec2 eyeb = vec2( p_eyeb.x * x_a, p_eyeb.y * y_a );			\n"
			"	float weight = 0.0;								\n"
			"	float face_width = distance( eyea, eyeb );		\n"
			"													\n"
			"	// eye1											\n"
			"	float eyeRadius = face_width*0.27;				\n"
			"	float dis_eye1 = distance( newCoord, eyea );	\n"
			"	if (dis_eye1 <= eyeRadius){						\n"
			"		weight = pow( dis_eye1 / eyeRadius, 0.05 );	\n"
			"		newCoord = eyea + (newCoord - eyea)*weight;	\n"
			"	}												\n"
			"	// eye2											\n"
			"	float dis_eye2 = distance( newCoord, eyeb );	\n"
			"	if (dis_eye2 <= eyeRadius){						\n"
			"		weight = pow( dis_eye2 / eyeRadius, 0.05 );	\n"
			"		newCoord = eyeb + (newCoord - eyeb)*weight;	\n"
			"	}												\n"
			"																\n"
			"	newCoord = vec2( newCoord.x / x_a, newCoord.y / y_a );		\n"
			"	outColor = texture( inputImageTexture, newCoord );		\n"
			"}																\n";
#else
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"								\n"
			"uniform highp vec2 p_eyea;	\n"
			"uniform highp vec2 p_eyeb;	\n"
			"								\n"
			"uniform highp float aspectRatio;	\n"
			"uniform highp float scale; 		\n"
			"								\n"
			"void main( ){					\n"
			"	vec2 textureCoordinateToUse = vec2( textureCoordinate.x, (textureCoordinate.y * aspectRatio + 0.5 - 0.5 * aspectRatio) );	\n"
			"	vec2 eyea = vec2( p_eyea.x, (p_eyea.y * aspectRatio + 0.5 - 0.5 * aspectRatio) );			\n"
			"	vec2 eyeb = vec2( p_eyeb.x, (p_eyeb.y * aspectRatio + 0.5 - 0.5 * aspectRatio) );			\n"
			"	float face_width = distance( eyea, eyeb );		\n"
			"													\n"
			"	// eye1											\n"
			"	float eyeRadius = face_width*0.27;				\n"
			"	float dis_eye1 = distance( textureCoordinateToUse, eyea );	\n"
			"	float dis_eye2 = distance( textureCoordinateToUse, eyeb );	\n"
			"	textureCoordinateToUse = textureCoordinate;					\n"
			"	if (dis_eye1 <= eyeRadius){						\n"
			"		textureCoordinateToUse -= p_eyea;				\n" 
			"		highp float percent = 1.0 - ((eyeRadius - dis_eye1) / eyeRadius) * scale;	\n" 
			"		percent = percent * percent;					\n"
			"		textureCoordinateToUse = textureCoordinateToUse * percent;		\n" 
			"		textureCoordinateToUse += p_eyea;				\n"
			"	}												\n"
			"	// eye2											\n"
			"	if (dis_eye2 <= eyeRadius){						\n"
			"		textureCoordinateToUse -= p_eyeb;				\n"
			"		highp float percent = 1.0 - ((eyeRadius - dis_eye2) / eyeRadius) * scale;	\n"
			"		percent = percent * percent;					\n"
			"		textureCoordinateToUse = textureCoordinateToUse * percent;		\n"
			"		textureCoordinateToUse += p_eyeb;				\n"
			"	}												\n"
			"																\n"
			"	outColor = texture( inputImageTexture, textureCoordinateToUse );		\n"
			"}																\n";
#endif



		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );
		userData->location4Loc = glGetUniformLocation( userData->programObject, "p_eyea" );
		userData->location5Loc = glGetUniformLocation( userData->programObject, "p_eyeb" );
		userData->aspectRatioLoc = glGetUniformLocation( userData->programObject, "aspectRatio" );
		userData->scaleLoc = glGetUniformLocation( userData->programObject, "scale" );


		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	int InitSlimFace( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate;		\n"
			"}										\n";
#if 1
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"								\n"
			"uniform highp vec2 points[8];		\n"
			"const highp float x_a = 0.72;		\n"
			"const highp float y_a = 1.28;		\n"
			"vec2 faceStretch( vec2 textureCoord, vec2 originPosition, vec2 targetPosition, float radius, float curve )	\n"
			"{																		\n"
			"	vec2 direction = targetPosition - originPosition;					\n"
			"	float lengthA = length( direction );								\n"
			"	float lengthB = min( lengthA, radius );								\n"
			"	direction *= lengthB / lengthA;										\n"
			"	float infect = distance( textureCoord, originPosition ) / radius;	\n"
			"	infect = clamp( 1.0 - infect, 0.0, 1.0 );							\n"
			"	infect = pow( infect, curve );										\n"
			"	return direction * infect;											\n"
			"}													\n"
			"													\n"
			"void main( ) {										\n"
			"	highp vec2 p_eyea = points[0];			\n"
			"	highp vec2 p_eyeb = points[1];			\n"
			"	highp vec2 p_faceleft = points[2];		\n"
			"	highp vec2 p_chinleft = points[3];		\n"
			"	highp vec2 p_chin = points[4];			\n"
			"	highp vec2 p_chinright = points[5];		\n"
			"	highp vec2 p_faceright = points[6];		\n"
			"	highp vec2 p_nose = points[7];			\n"
			"	vec2 newCoord = vec2( textureCoordinate.x*x_a, textureCoordinate.y*y_a );		\n"
			"															\n"
			"	vec2 eyea = vec2( p_eyea.x * x_a, p_eyea.y * y_a );		\n"
			"	vec2 eyeb = vec2( p_eyeb.x * x_a, p_eyeb.y * y_a );		\n"
			"															\n"
			"	vec2 faceleft = vec2( p_faceleft.x * x_a, p_faceleft.y * y_a );		\n"
			"	vec2 faceright = vec2( p_faceright.x * x_a, p_faceright.y * y_a );	\n"
			"																		\n"
			"	vec2 chinleft = vec2( p_chinleft.x * x_a, p_chinleft.y * y_a );		\n"
			"	vec2 chinright = vec2( p_chinright.x * x_a, p_chinright.y * y_a );	\n"
			"																		\n"
			"	vec2 nose = vec2( p_nose.x * x_a, p_nose.y * y_a );					\n"
			"	vec2 chin = vec2( p_chin.x * x_a, p_chin.y * y_a );					\n"
			"																		\n"
			"	vec2 chinCenter = nose + (chin - nose) * 0.8;						\n"
			"																		\n"
			"	float face_width = distance( eyea, eyeb );							\n"
			"																		\n"
			"	float radius = face_width*1.0;										\n"
			"	vec2 leftF = faceleft;												\n"
			"	vec2 targetleftF = nose + (leftF - nose) * 0.9;					\n"
			"	vec2 leftFplus = vec2( 0.0 );										\n"
			"	leftFplus = faceStretch( newCoord, leftF, targetleftF, radius, 1.0 );	\n"
			"	newCoord = newCoord - leftFplus;										\n"
			"																			\n"
			"	vec2 rightF = faceright;												\n"
			"	vec2 targetrightF = nose + (rightF - nose) * 0.9;						\n"
			"	vec2 rightFplus = vec2( 0.0 );											\n"
			"	rightFplus = faceStretch( newCoord, rightF, targetrightF, radius, 1.0 );\n"
			"	newCoord = newCoord - rightFplus;										\n"
			"																			\n"
			"	radius = face_width*1.2;												\n"
			"	vec2 leftC = chinleft;													\n"
			"	vec2 targetleftC = chinCenter + (leftC - chinCenter) * 0.98;			\n"
			"	vec2 leftCplus = vec2( 0.0 );											\n"
			"	leftCplus = faceStretch( newCoord, leftC, targetleftC, radius, 1.0 );	\n"
			"	newCoord = newCoord - leftCplus;										\n"
			"																			\n"
			"	vec2 rightC = chinright;												\n"
			"	vec2 targetrightC = chinCenter + (rightC - chinCenter) * 0.98;			\n"
			"	vec2 rightCplus = vec2( 0.0 );											\n"
			"	rightCplus = faceStretch( newCoord, rightC, targetrightC, radius, 1.0 );\n"
			"	newCoord = newCoord - rightCplus;										\n"
			"																			\n"
			"	newCoord = vec2( newCoord.x / x_a, newCoord.y / y_a );					\n"
			"	outColor = texture( inputImageTexture, newCoord );					\n"
			"}																			\n";
#endif

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );
		userData->pointsLoc = glGetUniformLocation( userData->programObject, "points" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitSnakeFace( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate;		\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"								\n"
			"uniform highp vec2 points[14];\n"
			"\n"
			"vec2 faceStretch( vec2 textureCoord, vec2 originPosition, vec2 targetPosition, float radius, float amp, float curve )	\n"
			"{	\n"
			"	vec2 direction = targetPosition - originPosition;	\n"
			"	float lengthA = length( direction );	\n"
			"	float lengthB = min( lengthA, radius );	\n"
			"	direction *= amp * lengthB / lengthA;	\n"
			"	float infect = distance( textureCoord, originPosition ) / radius;	\n"
			"	infect = clamp( 1.0 - infect, 0.0, 1.0 );	\n"
			"	infect = pow( infect, curve );	\n"
			"	return direction * infect;	\n"
			"}	\n"
			"	\n"
			"void main( ) {	\n"
			"	outColor = texture( inputImageTexture, textureCoordinate );	\n"
			"	\n"
			"	highp vec2 location0 = points[0]; // p_eyeRight	\n"
			"	highp vec2 location1 = points[1]; // p_eyeLeft	\n"
			"	if (location0.x < 0.01 && location1.x < 0.01)	\n"
			"	{	\n"
			"		outColor = texture( inputImageTexture, textureCoordinate );	\n"
			"		return;	\n"
			"	}	\n"
			"	\n"
			"	highp vec2 location4 = points[7]; // p_noseTip	\n"
			"	highp vec2 location8 = points[8]; // p_mouthTop	\n"
			"	highp vec2 location9 = points[9]; // p_mouthBottom	\n"
			"	highp vec2 location10 = points[4];// p_chin	\n"
			"	highp vec2 location11 = points[10];// p_faceRight1	\n"
			"	highp vec2 location12 = points[11];// p_faceLeft1	\n"
			"	highp vec2 location13 = points[12];// p_faceRight2	\n"
			"	highp vec2 location14 = points[13];// p_faceLeft2	\n"
			"	highp vec2 location17 = points[6];// p_faceRight4	\n"
			"	highp vec2 location18 = points[2];// p_faceLeft4	\n"
			"	highp vec2 location21 = points[5];// p_faceRight6	\n"
			"	highp vec2 location22 = points[3];// p_faceLeft6	\n"
			"	\n"
			"	float x_axis = 0.72;	\n"
			"	float y_axis = 1.28;	\n"
			"	\n"
			"	highp vec2 textureCoord = vec2( textureCoordinate.x * x_axis, textureCoordinate.y * y_axis );	\n"
			"	\n"
			"	//==========	\n"
			"	float faceWidth = distance( vec2( location0.x * x_axis, location0.y * y_axis ), vec2( location1.x * x_axis, location1.y * y_axis ) );	\n"
			"	vec2 centerPoint = vec2( location9.x * x_axis, location9.y * y_axis );	\n"
			"	\n"
			"	vec2 targetLeft = vec2( location12.x * x_axis, location12.y * y_axis );	\n"
			"	vec2 targetRight = vec2( location11.x * x_axis, location11.y * y_axis );	\n"
			"	\n"
			"	float radius = 0.0;	\n"
			"	float amp = 1.0;	\n"
			"	\n"
			"	// 13,12	\n"
			"	radius = faceWidth*1.5;	\n"
			"	vec2 faceLeft5 = vec2( location18.x*x_axis, location18.y*y_axis );	\n"
			"	targetLeft = centerPoint + (faceLeft5 - centerPoint) * 0.85;	\n"
			"	vec2 vectorfaceLeft5 = faceStretch( textureCoord, faceLeft5, targetLeft, radius, amp, 2.0 );	\n"
			"	// 13,11	\n"
			"	vec2 faceRight5 = vec2( location17.x*x_axis, location17.y*y_axis );	\n"
			"	targetRight = centerPoint + (faceRight5 - centerPoint) * 0.85;	\n"
			"	vec2 vectorfaceRight5 = faceStretch( textureCoord, faceRight5, targetRight, radius, amp, 2.0 );	\n"
			"	// 13,6		\n"
			"	radius = faceWidth*1.1;	\n"
			"	vec2 faceLeft54 = vec2( location14.x*x_axis, location14.y*y_axis );	\n"
			"	targetLeft = centerPoint + (faceLeft54 - centerPoint) * 0.93;	\n"
			"	vec2 vectorfaceLeft54 = faceStretch( textureCoord, faceLeft54, targetLeft, radius, amp, 1.0 );	\n"
			"	// 13,5		\n"
			"	vec2 faceRight54 = vec2( location13.x*x_axis, location13.y*y_axis );	\n"
			"	targetRight = centerPoint + (faceRight54 - centerPoint) * 0.93;	\n"
			"	vec2 vectorfaceRight54 = faceStretch( textureCoord, faceRight54, targetRight, radius, amp, 1.0 );	\n"
			"	// 2,1		\n"
			"	radius = faceWidth*1.0;	\n"
			"	vec2 chin = vec2( location10.x * x_axis, location10.y * y_axis );	\n"
			"	vec2 targetchin = centerPoint + (chin - centerPoint) * 1.03;	\n"
			"	vec2 chinplus = faceStretch( textureCoord, chin, targetchin, radius, amp, 2.0 );	\n"
			"	// 13,14			\n"
			"	radius = faceWidth*1.3;	\n"
			"	vec2 faceLeftplus7 = vec2( location22.x*x_axis, location22.y*y_axis );	\n"
			"	targetLeft = centerPoint + (faceLeftplus7 - centerPoint) * 0.9;	\n"
			"	vec2 vectorfaceLeftplus7 = faceStretch( textureCoord, faceLeftplus7, targetLeft, radius, amp, 2.0 );	\n"
			"	// 13,13			\n"
			"	vec2 faceRightplus7 = vec2( location21.x*x_axis, location21.y*y_axis );	\n"
			"	targetRight = centerPoint + (faceRightplus7 - centerPoint) * 0.9;	\n"
			"	vec2 vectorfaceRightplus7 = faceStretch( textureCoord, faceRightplus7, targetRight, radius, amp, 2.0 );	\n"
			"	// bigger left eye ==========	\n"
			"	radius = faceWidth*0.13;	\n"
			"	float a = 1.2;	\n"
			"	vec2 diffusePosition = vec2( location0.x*x_axis, location0.y*y_axis );	\n"
			"	float infect3 = distance( textureCoord, diffusePosition ) / radius;	\n"
			"	infect3 = clamp( infect3, 0.0, 1.0 );	\n"
			"	infect3 = a * infect3 * (infect3 - 1.0) + 1.0;	\n"
			"	// bigger right eye ==========	\n"
			"	vec2 diffusePosition2 = vec2( location1.x*x_axis, location1.y*y_axis );	\n"
			"	float infect4 = distance( textureCoord, diffusePosition2 ) / radius;	\n"
			"	infect4 = clamp( infect4, 0.0, 1.0 );	\n"
			"	infect4 = a * infect4 * (infect4 - 1.0) + 1.0;	\n"
			"	//====================================	\n"
			"	highp vec2 textureCoord_new = textureCoord;	\n"
			"	\n"
			//"	vec2 sum = vectorfaceLeft5 + vectorfaceRight5 + chinplus + vectorfaceLeft54 + vectorfaceRight54;	\n"
			//"	sum = sum + vectorfaceLeftplus7 + vectorfaceRightplus7;	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceLeft5;	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceRight5;	\n"
			"	textureCoord_new = textureCoord_new - chinplus;	\n"
			"	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceLeft54;	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceRight54;	\n"
			"	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceLeftplus7;	\n"
			"	textureCoord_new = textureCoord_new - vectorfaceRightplus7;	\n"
			"	\n"
			"	textureCoord_new = diffusePosition + (textureCoord_new - diffusePosition)*infect3;	\n"
			"	textureCoord_new = diffusePosition2 + (textureCoord_new - diffusePosition2)*infect4;	\n"
			"	\n"
			"	highp vec2 coordUse = vec2( textureCoord_new.x / x_axis, textureCoord_new.y / y_axis );	\n"
			"	outColor = texture( inputImageTexture, coordUse );	\n"
			"	}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );
		userData->pointsLoc = glGetUniformLocation( userData->programObject, "points" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitNightVision( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate;		\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			//"uniform float intensity;		\n"
			"								\n"
			"vec3 applyNV( vec3 color )	\n"
			"{	\n"
			"	float intensity = 1.0;		\n"
			"	float luma = dot( color, vec3( 0.299, 0.587, 0.114 ) );	\n"
			"	luma = 1.0 * luma;	\n"
			"	color *= clamp( 1.0 - intensity, 0.0, 1.0 );	\n"
			"	luma *= intensity;	\n"
			"		\n"
			"	float h = 2.0 * max( luma - 0.75, 0.0 );	\n"
			"		\n"
			"	luma = clamp( luma, 0.0, 1.0 );	\n"
			"		\n"
			"	vec3 result = luma * vec3( 0.1, 0.95, 0.2 ) + vec3( h, h, h );	\n"
			"		\n"
			"	return color + result;	\n"
			"}		\n"
			"void main( ) {	\n"
			"	outColor = texture( inputImageTexture, textureCoordinate );	\n"
			"	outColor.rgb = applyNV(outColor.rgb);	\n"
			"	outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitGuidedFilter( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"const int range = 7;	\n"
			"out vec2 textureCoordinate[15];                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	vec2 dPixel = vec2(1.0/480.0, 1.0/640.0);	\n"
			"	for (int d = -range; d <= range; d += 1) {	\n"
			"		textureCoordinate[range + d] = inputTextureCoordinate + float( d ) * dPixel;	\n"
			"	}	\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate[15];					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"#define sampleCurrent(uv) texture(inputImageTexture, uv).rgb	\n"
			"#define ADDSAMPLE(v) color = sampleCurrent(v); sum += color * color;	\n"
			"void main( ) {	\n"
				"vec3 color;	\n"
				"vec2 dPixel = vec2(2.0/480.0, 2.0/640.0);	\n"
				"vec3 sum = vec3( 0.0 );	\n"
				"ADDSAMPLE( textureCoordinate[0] - dPixel )	\n"
				"ADDSAMPLE( textureCoordinate[0] )	\n"
				"ADDSAMPLE( textureCoordinate[1] )	\n"
				"ADDSAMPLE( textureCoordinate[2] )	\n"
				"ADDSAMPLE( textureCoordinate[3] )	\n"
				"ADDSAMPLE( textureCoordinate[4] )	\n"
				"ADDSAMPLE( textureCoordinate[5] )	\n"
				"ADDSAMPLE( textureCoordinate[6] )	\n"
				"ADDSAMPLE( textureCoordinate[7] )	\n"
				"ADDSAMPLE( textureCoordinate[8] )	\n"
				"ADDSAMPLE( textureCoordinate[9] )	\n"
				"ADDSAMPLE( textureCoordinate[10] )	\n"
				"ADDSAMPLE( textureCoordinate[11] )	\n"
				"ADDSAMPLE( textureCoordinate[12] )	\n"
				"ADDSAMPLE( textureCoordinate[13] )	\n"
				"ADDSAMPLE( textureCoordinate[14] )	\n"
				"ADDSAMPLE( textureCoordinate[14] + dPixel )	\n"
				"	\n"
				"outColor.rgb = sum / 17.0;	\n"
				"outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}
	
	//
	int InitHighPassFilter( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"const int range = 1;	\n"
			"out vec2 textureCoordinate[3];                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	vec2 dPixel = vec2(1.0/480.0, 1.0/640.0);	\n"
			"	for (int d = -range; d <= range; d += 1) {	\n"
			"		textureCoordinate[range + d] = inputTextureCoordinate + float( d ) * dPixel;	\n"
			"	}	\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate[3];					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"#define sampleCurrent(uv) texture(inputImageTexture, uv).rgb	\n"
			"#define ADDSAMPLE(v) color = sampleCurrent(v); sum += color;	\n"
			"const float contrast = 0.8;	\n"
			"void main( ) {	\n"
				"vec3 color;	\n"
				"vec3 sum = vec3( 0.0 );	\n"
				"ADDSAMPLE( textureCoordinate[0] )	\n"
				"ADDSAMPLE( textureCoordinate[1] )	\n"
				"ADDSAMPLE( textureCoordinate[2] )	\n"
				"	\n"
				"outColor.rgb = sum / 3.0;	\n"
				"outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitLUTOpacity( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"const int range = 1;	\n"
			"out vec2 textureCoordinate[3];                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	vec2 dPixel = vec2(1.0/480.0, 1.0/640.0);	\n"
			"	for (int d = -range; d <= range; d += 1) {	\n"
			"		textureCoordinate[range + d] = inputTextureCoordinate + float( d ) * dPixel;	\n"
			"	}	\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate[3];					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"#define sampleCurrent(uv) texture(inputImageTexture, uv).rgb	\n"
			"#define ADDSAMPLE(v) color = sampleCurrent(v); sum += color;	\n"
			"const float contrast = 0.8;	\n"
			"void main( ) {	\n"
			"vec3 color;	\n"
			"vec3 sum = vec3( 0.0 );	\n"
			"ADDSAMPLE( textureCoordinate[0] )	\n"
			"ADDSAMPLE( textureCoordinate[1] )	\n"
			"ADDSAMPLE( textureCoordinate[2] )	\n"
			"	\n"
			"outColor.rgb = sum / 3.0;	\n"
			"outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );

		if (userData->baseMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitScreenEffect( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate.xy;	\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"uniform sampler2D backgroundImage;				\n"
			"void main( ) {	\n"
			"	vec4 foreColor;                                   \n"
			"	vec4 backColor;                                  \n"
			"                                                    \n"
			"	foreColor = texture( inputImageTexture, textureCoordinate );     \n"
			"	backColor = texture( backgroundImage, textureCoordinate );   \n"
			//"	backColor.rgb = clamp(foreColor.rgb * backColor.rgb,0.0,1.0);	\n"	// 
			"	outColor.rgb = mix(foreColor.rgb, backColor.rgb, backColor.a);	\n"
			"	outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );
		userData->backMapLoc = glGetUniformLocation( userData->programObject, "backgroundImage" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );
		userData->backMapTexId = LoadTextureImageString( esContext->platformData, "D:\\8_resources\\opengl\\fon.png" );

		if (userData->baseMapTexId == 0 || userData->backMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}

	//
	int InitGlowScreenEffect( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 position;		\n"
			"layout(location = 1) in vec2 inputTextureCoordinate;   \n"
			"out vec2 textureCoordinate;                \n"
			"void main( )							\n"
			"{										\n"
			"	gl_Position = position;				\n"
			"	textureCoordinate = inputTextureCoordinate.xy;	\n"
			"}										\n";
		char fShaderStr[] =
			"#version 300 es                                    \n"
			"precision highp float;								\n"
			"in highp vec2 textureCoordinate;					\n"
			"layout(location = 0) out vec4 outColor;            \n"
			"uniform sampler2D inputImageTexture;				\n"
			"uniform sampler2D backgroundImage;				\n"
			"uniform sampler2D fluidImage;					\n"
			"uniform float uvOffset;						\n"
			"void main( ) {	\n"
			"	vec4 foreColor;                                   \n"
			"	vec4 backColor;                                  \n"
			"                                                    \n"
			"	foreColor = texture( inputImageTexture, textureCoordinate );     \n"
			"	backColor = texture( backgroundImage, textureCoordinate );   \n"
			"	vec2 f_uv = vec2(textureCoordinate.x-uvOffset*sin(0.8), textureCoordinate.y+uvOffset );	\n"
			"	vec4 fluidColor = texture(fluidImage, f_uv );	\n"
			"	float alpha = dot(vec3(0.299,0.587,0.114), foreColor.rgb);	\n"	// 
			"	foreColor.rgb = mix(foreColor.rgb, max(foreColor.rgb , fluidColor.rgb), alpha); \n"
			"	outColor.rgb = mix(foreColor.rgb, backColor.rgb, 0.5);	\n"
			"	outColor.a = 1.0;	\n"
			"}	\n";

		// Load the shaders and get a linked program object
		userData->programObject = esLoadProgram( vShaderStr, fShaderStr );

		// Get the sampler location
		userData->baseMapLoc = glGetUniformLocation( userData->programObject, "inputImageTexture" );
		userData->backMapLoc = glGetUniformLocation( userData->programObject, "backgroundImage" );
		userData->fluidMapLoc = glGetUniformLocation( userData->programObject, "fluidImage" );
		userData->fluidUvOffsetLoc = glGetUniformLocation( userData->programObject, "uvOffset" );

		// Load the textures
		userData->baseMapTexId = LoadTextureImage( esContext->platformData, userData->image );
		userData->backMapTexId = LoadTextureImageString( esContext->platformData, "D:\\8_resources\\opengl\\glow\\glowmakeup_multiply.png" );
		userData->fluidMapTexId = LoadTextureImageString( esContext->platformData, "D:\\8_resources\\opengl\\glow\\light2.png" );
		
		if (userData->baseMapTexId == 0 || userData->backMapTexId == 0)
		{
			return FALSE;
		}

		glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
		return TRUE;
	}
	///
	// Draw a triangle using the shader pair created in Init()
	//
	void Draw( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);
		GLfloat vVertices[] = { -1.0f,  1.0f, 0.0f,  // Position 0, left-top
			0.0f,  0.0f,        // TexCoord 0 
			-1.0f, -1.0f, 0.0f,  // Position 1, left-bottom
			0.0f,  1.0f,        // TexCoord 1
			1.0f, -1.0f, 0.0f,  // Position 2, right-bottom
			1.0f,  1.0f,        // TexCoord 2
			1.0f,  1.0f, 0.0f,  // Position 3, right-top
			1.0f,  0.0f         // TexCoord 3
		};
		GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

		// Set the viewport
		glViewport( 0, 0, esContext->width, esContext->height );

		// Clear the color buffer
		glClear( GL_COLOR_BUFFER_BIT );

		// Use the program object
		glUseProgram( userData->programObject );

		// Load the vertex position
		glVertexAttribPointer( 0, 3, GL_FLOAT,
			GL_FALSE, 5 * sizeof( GLfloat ), vVertices );
		// Load the texture coordinate
		glVertexAttribPointer( 1, 2, GL_FLOAT,
			GL_FALSE, 5 * sizeof( GLfloat ), &vVertices[3] );

		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		/*
		//
#if FSHADER_IMPL == GPUIMAGE_IMPL
		glUniform1f( userData->texelWidthOffsetLoc, userData->texelWidthOffset );
		glUniform1f( userData->texelHeightOffsetLoc, userData->texelHeightOffset );
#elif FSHADER_IMPL == FACEU_IMPL
		BEAUTY beautyType = BEAUTY::FISRT;
		switch (beautyType)
		{
		case BEAUTY::FISRT:
			glUniform1i( userData->iternumLoc, 2 );
			glUniform1f( userData->aa_coefLoc, 0.19 );
			glUniform1f( userData->mixcoefLoc, 0.54 );
			break;
		case BEAUTY::SECOND:
			glUniform1i( userData->iternumLoc, 2 );
			glUniform1f( userData->aa_coefLoc, 0.25 );
			glUniform1f( userData->mixcoefLoc, 0.54 );
			break;
		case BEAUTY::THIRD:
			glUniform1i( userData->iternumLoc, 3 );
			glUniform1f( userData->aa_coefLoc, 0.17 );
			glUniform1f( userData->mixcoefLoc, 0.39 );
			break;
		case BEAUTY::FORTH:
			glUniform1i( userData->iternumLoc, 4 );
			glUniform1f( userData->aa_coefLoc, 0.13 );
			glUniform1f( userData->mixcoefLoc, 0.54 );
			break;
		case BEAUTY::FIFTH:
			glUniform1i( userData->iternumLoc, 4 );
			glUniform1f( userData->aa_coefLoc, 0.19 );
			glUniform1f( userData->mixcoefLoc, 0.69 );
			break;
		}
#endif
		*/
		//glUniform2fv( userData->pointsLoc, userData->fNum, userData->fLoc );

		// Bind the base map
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, userData->baseMapTexId );

		// Set the base map sampler to texture unit 0
		glUniform1i( userData->baseMapLoc, 0 );

		// Bind the back map
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, userData->backMapTexId );
		glUniform1i( userData->backMapLoc, 1 );

		// Bind the fluid map
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, userData->fluidMapTexId );
		glUniform1i( userData->fluidMapLoc, 2 );

		static float time = -3.1415926 / 4.0;
		time += 0.0005;
		if (time > 3.1415926 / 4.0) {
			time = -3.1415926 / 4.0;
		}
		float offset = sin(time);
		glUniform1f( userData->fluidUvOffsetLoc, offset );

		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
	}

	///
	// Cleanup
	//
	void ShutDown( ESContext *esContext )
	{
		UserData *userData = static_cast<UserData*>(esContext->userData);

		// Delete texture object
		glDeleteTextures( 1, &userData->baseMapTexId );

		// Delete program object
		glDeleteProgram( userData->programObject );


		cvReleaseImage( &(userData->image) );
		if (!userData->fLoc) { delete[] userData->fLoc; }
	}

	int esMain( ESContext *esContext )
	{
		esContext->userData = malloc( sizeof( UserData ) );
		memset( esContext->userData, 0, sizeof( UserData ) );
		char imagefile[] = "D:\\8_resources\\opengl\\glow\\glowmakeup_additive.png";
		UserData* userData = static_cast<UserData*>(esContext->userData);
		userData->image = cvLoadImage( imagefile, CV_LOAD_IMAGE_COLOR );
		if (!userData->image) { printf( "Load image:%s failure.\n", imagefile ); getchar( ); return GL_FALSE; }
		userData->texelHeightOffset = 2.0 / userData->image->height;
		userData->texelWidthOffset = 2.0 / userData->image->width;

		if (!userData->fLoc) { userData->fLoc = new float[2 * 14]; memset( userData->fLoc, 0, 28*sizeof( float ) ); }
		//userData->fNum = featureDetect( userData->image, userData->fLoc );
		userData->fNum = 0;
		if (userData->fNum>0){
			userData->scale = 0.1f;
			userData->aspectRatio = userData->image->height * 1.0f / userData->image->width;
		}

		esCreateWindow( esContext, "MultiTexture", userData->image->width, userData->image->height, ES_WINDOW_RGB );
		//SetWindowPos( esContext->eglNativeWindow, HWND_TOP, 0, 0, image.cols, image.rows, SWP_SHOWWINDOW );

#if 0
		if (userData->fNum > 0) {
			userData->fNum = 8;
		}
		if (!InitSlimFace( esContext ))
		{
			return GL_FALSE;
		}
#else
// 		if (!InitSnakeFace( esContext ))
// 		{
// 			return GL_FALSE;
// 		}
		if (!InitGlowScreenEffect( esContext ))
		{
			return GL_FALSE;
		}
#endif

		esRegisterDrawFunc( esContext, Draw );
		esRegisterShutdownFunc( esContext, ShutDown );

		return GL_TRUE;
	}
}



