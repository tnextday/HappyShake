
#-------------------------------------------------
# ORION - OpenGL ES 2.0 Shading Language Compiler
# SAMSUNG INDIA SOFTWARE OPERATIONS PVT. LTD.
# Compiler Version	: v04.00.03
# Release Date		: 30.06.2008
# FIMG VERSION      : FIMGv1.5
# Optimizer Options :  -O
#-------------------------------------------------

vs_3_0

fimg_version	0x01020000

dcl_f4_gl_Position	o0.x
dcl_f4_a_vertexPos	v0.x
dcl_f2_a_texCoord	v1.x
dcl_m4_mvp	c2.x
dcl_f2_v_texC	o1.x

def c1, 1.0, 1.0, 1.0, 1.0
def c0, 0.0, 0.0, 0.0, 0.0

label start
label main_
mul r0.xyzw, c2.xyzw, v0.xxxx	# a_vertexPos=v0.xxxx
mad r0.xyzw, c3.xyzw, v0.yyyy, r0.xyzw	# a_vertexPos=v0.yyyy
mad r0.xyzw, c4.xyzw, v0.zzzz, r0.xyzw	# a_vertexPos=v0.zzzz
mad o0.xyzw, c5.xyzw, v0.wwww, r0.xyzw	# a_vertexPos=v0.wwww
label main_end
mov o1.xy, v1.xy
ret
# 6 instructions, 7 C regs, 1 R regs 
