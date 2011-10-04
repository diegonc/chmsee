CHMSEE_ROOTDIR = ..
COMPONENTSDIR = ${CHMSEE_ROOTDIR}/components

TARGET = ${COMPONENTSDIR}/libxpcomchm.so

SRCS = csChm.cpp csChmModule.cpp csChmfile.c
OBJS = csChm.o csChmModule.o csChmfile.o

INTERFACE = csIChm
IDL = ${INTERFACE}.idl
XPT = ${COMPONENTSDIR}/xpcomchm.xpt

SDK_IDL = ${LIBXUL_SDK}/idl
XPIDL = ${LIBXUL_SDK}/bin/xpidl
XPT_LINK = ${LIBXUL_SDK}/sdk/bin/xpt.py link

MOZ_DEBUG_DISABLE_DEFS	= -DNDEBUG -DTRIMMED

INCLUDES         = -I/usr/include -I. -I${LIBXUL_SDK}/include ${NSPR_INCLUDES} ${CHMLIB_INCLUDES}
DEFINES		 = -Wall -Wpointer-arith -Wcast-align -Wno-variadic-macros \
		   -O2 -fPIC -DPIC -fno-strict-aliasing -Dunix -std=gnu++0x -fshort-wchar -pthread -pipe
VISIBILITY_FLAGS = -fvisibility=hidden
LIBXUL_CXXFLAGS  = -DMOZILLA_CLIENT -include mozilla-config.h

CFLAGS          += ${DEFINES} ${VISIBILITY_FLAGS} ${INCLUDES}
CXXFLAGS        += -fno-rtti -fno-exceptions \
	           -Woverloaded-virtual -Wsynth -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor -Wno-invalid-offsetof \
	           ${VISIBILITY_FLAGS} ${DEFINES} ${INCLUDES} ${LIBXUL_CXXFLAGS}


XPCOM_FROZEN_LDOPTS = -Wl,-R${LIBXUL_SDK}/bin -L${LIBXUL_SDK}/bin -lxpcom -lmozalloc

LDFLAGS            += ${DEFINES} \
	              ${INCLUDES} \
		      ${MOZ_DEBUG_DISABLE_DEFS} \
		      -shared -Wl,-soname,${TARGET} -lpthread \
		      ${LIBXUL_SDK}/lib/libxpcomglue_s.a \
		      ${XPCOM_FROZEN_LDOPTS} \
		      ${NSPR_LIBS} \
		      ${CHMLIB_LIBS}


all: ${TARGET}

${XPT}: ${IDL}
	${XPIDL} -w -v -m header -I ${SDK_IDL} ${IDL}
	${XPIDL} -w -v -m typelib -I ${SDK_IDL} ${IDL}
	${XPT_LINK} ${XPT} ${INTERFACE}.xpt

${TARGET}: ${XPT} ${OBJS}
	${CC} ${OBJS} -o ${TARGET} ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $<

%.o: %.c++
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm ${TARGET} ${OBJS} ${XPT}
