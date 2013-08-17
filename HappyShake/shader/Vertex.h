#ifndef _VertexVertex_h_
#define _VertexVertex_h_


// Header generated from binary by WriteAsBinHeader()..
static const int VertexVertexLength = 103;
static const unsigned int VertexVertex[VertexVertexLength]={
	0x20205356,	0xFFFF0008,	0x00000048,	0x01020000,	0x00000006,	0x00000006,	0x00000000,	0x00000000,	0x00000002,	0x00000002,
	0x00000001,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x0000002E,	0x00000000,	0x00000000,	0x02020000,
	0x237820E4,	0x00000000,	0x00E40100,	0x02035500,	0x2EF820E4,	0x00000000,	0x00E40100,	0x0204AA00,	0x2EF820E4,	0x00000000,
	0x00E40100,	0x0205FF00,	0x0EF800E4,	0x00000000,	0x00000000,	0x00010000,	0x00980154,	0x00000000,	0x00000000,	0x00000000,
	0x1E000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x3F800000,	0x3F800000,	0x3F800000,	0x3F800000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x0000000C,	0x0000000B,	0x00000009,	0x00000000,
	0x00000000,	0x00000018,	0x0000000A,	0x00000003,	0x00000000,	0x00000004,	0x00000000,	0x0000000B,	0x00000009,	0x00010004,
	0x00000000,	0x00000027,	0x00000006,	0x00000003,	0x00010004,	0x00000004,	0x00000023,	0x00000003,	0x0000000E,	0x00020001,
	0x00000008,	0x505F6C67,	0x7469736F,	0x006E6F69,	0x65765F61,	0x78657472,	0x00736F50,	0x65745F61,	0x6F6F4378,	0x6D006472,
	0x76007076,	0x7865745F,	0x00000043,};

//checksum generated by simpleCheckSum()
static const unsigned int VertexVertexCheckSum = 180;

static const char* VertexVertexText = 
	"precision highp float;\n"
	"\n"
	"attribute vec4 a_vertexPos;\n"
	"attribute vec2 a_texCoord;\n"
	"\n"
	"uniform mat4 mvp;\n"
	"varying vec2 v_texC;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_Position = mvp * a_vertexPos;\n"
	"   v_texC = a_texCoord;\n"
	"}\n"
	"";

#ifdef GL_HELPERS_INCLUDED
//glHelpers.h must be included BEFORE any of the shader header files. Also make sure you have the latest version of glHelpers.h
static ghShader VertexVertexShader(VertexVertexText, VertexVertex, VertexVertexLength, VertexVertexCheckSum);


#endif


#endif //_VertexVertex_h_