CHMSEE_ROOTDIR = ..
COMPONENTSDIR = ${CHMSEE_ROOTDIR}/components

TARGET = ${COMPONENTSDIR}/${XPCOMCHM}

SRCS = csChm.cpp csChmModule.cpp csChmfile.c
OBJS = csChm.o csChmModule.o csChmfile.o

INTERFACE = csIChm
IDL = ${INTERFACE}.idl
XPT = ${COMPONENTSDIR}/xpcomchm.xpt

SDK_IDL = ${LIBXUL_SDK}/idl
XPIDL = ${PYTHON2} ${LIBXUL_SDK}/sdk/bin/xpidl.py
XPIDL_HEADER = ${PYTHON2} ${LIBXUL_SDK}/sdk/bin/header.py
XPIDL_TYPELIB = ${PYTHON2} ${LIBXUL_SDK}/sdk/bin/typelib.py
XPT_LINK = ${PYTHON2} ${LIBXUL_SDK}/sdk/bin/xpt.py link

MOZ_DEBUG_DISABLE_DEFS	= -DNDEBUG -DTRIMMED

INCLUDES         = -I/usr/include -I. -I${LIBXUL_SDK}/include ${NSPR_INCLUDES} ${CHMLIB_INCLUDES}
DEFINES		+= -Wall -Wpointer-arith -Wcast-align -Wno-variadic-macros \
		   -O2 -fPIC -DPIC -fno-strict-aliasing -Dunix -fshort-wchar -pipe
VISIBILITY_FLAGS = -fvisibility=hidden
LIBXUL_CXXFLAGS  = -DMOZILLA_CLIENT -include mozilla-config.h

CFLAGS          += ${DEFINES} ${VISIBILITY_FLAGS} ${INCLUDES}
CXXFLAGS        += -fno-rtti -fno-exceptions -std=gnu++0x \
	           -Woverloaded-virtual -Wsynth -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor -Wno-invalid-offsetof \
	           ${VISIBILITY_FLAGS} ${DEFINES} ${INCLUDES} ${LIBXUL_CXXFLAGS}

LDFLAGS            += ${DEFINES} \
	              ${INCLUDES} \
		      ${MOZ_DEBUG_DISABLE_DEFS} \
		      ${LIBXUL_SDK}/lib/libxpcomglue_s.a \
		      -shared -lxpcom -lmozalloc \
		      ${NSPR_LIBS} \
		      ${CHMLIB_LIBS}


all: ${TARGET}

${XPT}: ${IDL}
	${XPIDL_HEADER} -o ${INTERFACE}.h -I ${SDK_IDL} --cachedir=. ${IDL}
	${XPIDL_TYPELIB} -o ${INTERFACE}.xpt -I ${SDK_IDL} --cachedir=. ${IDL}
	${XPT_LINK} ${XPT} ${INTERFACE}.xpt

${TARGET}: ${XPT} ${OBJS}
	${CXX} ${OBJS} -o ${TARGET} ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $<

%.o: %.c++
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm ${TARGET} ${OBJS} ${XPT}
