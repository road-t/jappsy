#!/usr/local/bin/bash

echo "BASH VERSION: {$BASH_VERSION}"

# Config
WORKDIR=${PWD}
PROGNAME=$(basename $0)
GCCVER="4.9"
SDKDIR="${HOME}/Library/Android/sdk"
NDKDIR="${SDKDIR}/ndk-bundle"
ABIS=("armeabi" "armeabi-v7a" "arm64-v8a" "x86" "x86_64" "mips" "mips64")
LINKICUDATA=0
DISABLEJIT=0
FORCEJIT=0

# Declarations
declare -A APIS
APIS=(
    ["armeabi"]="13"
    ["armeabi-v7a"]="13"
    ["arm64-v8a"]="21"
    ["x86"]="13"
    ["x86_64"]="21"
    ["mips"]="13"
    ["mips64"]="21"
)

declare -A ARCHS
ARCHS=(
    ["armeabi"]="arch-arm"
    ["armeabi-v7a"]="arch-arm"
    ["arm64-v8a"]="arch-arm64"
    ["x86"]="arch-x86"
    ["x86_64"]="arch-x86_64"
    ["mips"]="arch-mips"
    ["mips64"]="arch-mips64"
)

declare -A HOSTS
HOSTS=(
    ["armeabi"]="arm-eabi"
    ["armeabi-v7a"]="arm-eabiv7a"
    ["arm64-v8a"]="arm-eabiv8a"
    ["x86"]="x86"
    ["x86_64"]="x86_64"
    ["mips"]="mips"
    ["mips64"]="mips64"
)

declare -A PREFIXES
PREFIXES=(
    ["armeabi"]="arm-linux-androideabi"
    ["armeabi-v7a"]="arm-linux-androideabi"
    ["arm64-v8a"]="aarch64-linux-android"
    ["x86"]="i686-linux-android"
    ["x86_64"]="x86_64-linux-android"
    ["mips"]="mipsel-linux-android"
    ["mips64"]="mips64el-linux-android"
)

declare -A TOOLCHAINS
TOOLCHAINS=(
    ["armeabi"]="arm-linux-androideabi-$GCCVER"
    ["armeabi-v7a"]="arm-linux-androideabi-$GCCVER"
    ["arm64-v8a"]="aarch64-linux-android-$GCCVER"
    ["x86"]="x86-$GCCVER"
    ["x86_64"]="x86_64-$GCCVER"
    ["mips"]="mipsel-linux-android-$GCCVER"
    ["mips64"]="mips64el-linux-android-$GCCVER"
)

declare -A MARCHCFLAGS
MARCHCFLAGS=(
    ["armeabi"]=""
    ["armeabi-v7a"]="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"
    ["arm64-v8a"]=""
    ["x86"]=""
    ["x86_64"]=""
    ["mips"]=""
    ["mips64"]=""
)

declare -A MARCHLDFLAGS
MARCHLDFLAGS=(
    ["armeabi"]=""
    ["armeabi-v7a"]="-march=armv7-a -Wl,--fix-cortex-a8"
    ["arm64-v8a"]=""
    ["x86"]=""
    ["x86_64"]=""
    ["mips"]=""
    ["mips64"]=""
)

export ANDROID_SDK_ROOT="${SDKDIR}"
export ANDROID_NDK_ROOT="${NDKDIR}"

function onError {
	echo "${WORKDIR}/${PROGNAME}: ${1:-"Unknown Error"}" 1>&2
	exit 1
}

function clearFlags {
	export CFLAGS=
	export CPPFLAGS=
	export CXXFLAGS=
	export LDFLAGS=
}

function checkGLIB {
	echo "Checking GLIB"
	if [ ! -d "glib" ]; then
		echo "Downloading GLIB sources..."
		wget http://ftp.gnome.org/pub/GNOME/sources/glib/2.48/glib-2.48.0.tar.xz
		tar xvzf glib-2.48.0.tar.xz
		mkdir glib
		mv glib-2.48.0 glib/trunk
		rm -rf glib-2.48.0.tar.xz
		#git clone git://git.gnome.org/glib ${WORKDIR}/glib/trunk
	fi
	if [ ! -d "libiconv" ]; then
		echo "Downloading libiconv sources..."
		wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
		tar xvzf libiconv-1.14.tar.gz
		mkdir libiconv
		mv libiconv-1.14 libiconv/trunk
		rm -rf libiconv-1.14.tar.gz
		#git clone git://git.savannah.gnu.org/libiconv.git ${WORKDIR}/libiconv/trunk
	fi
	if [ ! -d "libffi" ]; then
		echo "Downloading libffi sources..."
		wget ftp://sourceware.org/pub/libffi/libffi-3.2.1.tar.gz
		tar xvzf libffi-3.2.1.tar.gz
		mkdir libffi
		mv libffi-3.2.1 libffi/trunk
		rm -rf libffi-3.2.1.tar.gz
		#git clone git://github.com/atgreen/libffi.git ${WORKDIR}/libffi/trunk
	fi
	if [ ! -d "gettext" ]; then
		echo "Downloading gettext sources..."
		wget http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.7.tar.gz
		tar xvzf gettext-0.19.7.tar.gz
		mkdir gettext
		mv gettext-0.19.7 gettext/trunk
		rm -rf gettext-0.19.7.tar.gz
		#git clone git://git.savannah.gnu.org/gettext.git ${WORKDIR}/gettext/trunk
	fi
	export GLIBDIR=${WORKDIR}/glib
	export ICONVDIR=${WORKDIR}/libiconv
	export FFIDIR=${WORKDIR}/libffi
	export GETTEXTDIR=${WORKDIR}/gettext
}

function checkICUforMAC {
	echo "Checking ICU for MacOSX"
	if [ ! -d "icu" ]; then
		echo "Downloading ICU sources..."
		svn export http://source.icu-project.org/repos/icu/icu/trunk/ ${WORKDIR}/icu/trunk
		if !(( $? == 0 )); then
			rm -rf icu
			onError "Unable to download ICU!"
		fi
	fi
	cd icu || onError "ICU not found!"
	export ICUDIR=${WORKDIR}/icu
	export ICU_SOURCES=$ICUDIR/trunk
	[ -d "build" ] || mkdir build
	cd build
	if [ ! -d "icu_host" ]; then
		mkdir icu_host
		cd icu_host
		echo "Building ICU for MacOSX"
		export CFLAGS="-Os -DU_USING_ICU_NAMESPACE=1 \
			-fno-short-enums \
			-DUEXPORT2=__attribute__\(\(visibility\(\"default\"\)\)\) \
			-DU_HAVE_NL_LANGINFO_CODESET=0 \
			-D__STDC_INT64__ -DU_TIMEZONE=0 \
			-DUCONFIG_NO_LEGACY_CONVERSION=1 \
			-DUCONFIG_NO_BREAK_ITERATION=1 \
			-DUCONFIG_NO_COLLATION=0 \
			-DUCONFIG_NO_FORMATTING=0 \
			-DUCONFIG_NO_TRANSLITERATION=0 \
			-DUCONFIG_NO_REGULAR_EXPRESSIONS=1"
		export CPPFLAGS=$CFLAGS
		sh $ICU_SOURCES/source/runConfigureICU MacOSX \
			--prefix=$PWD/icu_build \
			--enable-extras=yes \
			--enable-strict=no \
			--enable-static \
			--enable-shared=no \
			--enable-tests=yes \
			--enable-samples=no \
			--enable-dyload=no \
		&& make -j8 \
		&& make install
		if !(( $? == 0 )); then
			cd ..
			rm -rf icu_host
			cd ${WORKDIR}
			onError "Unable to build ICU for MacOSX!"
		fi
	fi
	cd ${WORKDIR}
}

function setExports {
	export ABI=${1}
	export PLATFORM="android-${APIS["$ABI"]}"
	export PREBUILT="$ANDROID_NDK_ROOT/standalone/$PLATFORM/$ABI"
	export ARCH="${ARCHS["$ABI"]}"
	export TOOLCHAIN=${TOOLCHAINS["$ABI"]}
	export INSTALLDIR="$PREBUILT/third_party"
	export PREFIX="${PREFIXES["$ABI"]}"
	export HOST="${HOSTS["$ABI"]}"
}

function createToolchain {
	echo "Checking standalone toolchain for ABI: $ABI TOOLCHAIN: $TOOLCHAIN PLATFORM: $PLATFORM"
	if [ ! -d "$PREBUILT" ]; then
		sh $ANDROID_NDK_ROOT/build/tools/make-standalone-toolchain.sh \
			--toolchain=$TOOLCHAIN \
			--platform=$PLATFORM \
			--install-dir=$PREBUILT \
			--stl=gnustl
		if !(( $? == 0 )); then
			rm -rf $INSTALLDIR
			onError "Unable to generate standalone toolchain $TOOLCHAIN platform $PLATFORM"
		fi
	fi
}

function buildZlib {
	echo "Checking ZLIB for ABI: $ABI PLATFORM: $PLATFORM"
	if [ ! -f "$INSTALLDIR/lib/libz.a" ]; then
		echo "Building ZLIB"

	fi
	cd ${WORKDIR}
}

#		mkdir -p $HOST_ICU
#		cd $HOST_ICU
#			cd ${WORKDIR}
#		export PATH=$RESTOREPATH

export PATH="/Applications/CMake.app/Contents/bin":"$PATH"

clearFlags
#checkAndroidCmake
#checkGLIB
#checkICUforMAC
#checkWebkit

for ABI in "${ABIS[@]}"
do
	setExports $ABI
	createToolchain

		export API_LEVEL="${APIS["$ABI"]}"
		export PLATFORM="${ANDROID_NDK_ROOT}/platforms/android-${API_LEVEL}/${ARCH}"

		export CROSS_COMPILE="${PREBUILT}/bin/${PREFIX}"
		export CC="${CROSS_COMPILE}-gcc --sysroot=${PLATFORM}"
		export CXX="${CROSS_COMPILE}-g++ --sysroot=${PLATFORM}"
		export AR="${CROSS_COMPILE}-ar"
		export AS="${CROSS_COMPILE}-as"
		export LD="${CROSS_COMPILE}-ld"
		export RANLIB="${CROSS_COMPILE}-ranlib"
		export NM="${CROSS_COMPILE}-nm"
		export STRIP="${CROSS_COMPILE}-strip"
		export MAKE="/usr/bin/make"

		export INCLUDES="-DANDROID \
			-I${PLATFORM}/usr/include/ \
			-I${INSTALLDIR}/include \
			-I${ANDROID_NDK_ROOT}/sources/android/support/include \
			-I${INSTALLDIR}/include \
			-DANDROID_API=${API_LEVEL}"

		export CFLAGS="-fPIC $INCLUDES ${MARCHCFLAGS["$ABI"]}"
		export CPPFLAGS="-mandroid $INCLUDES ${MARCHCFLAGS["$ABI"]}"
		export CXXFLAGS=$CPPFLAGS
		export LIBS=""
		export LDFLAGS="-Wl,-rpath-link=${PLATFORM}/usr/lib/ \
			-L${INSTALLDIR}/lib64 \
			-L${INSTALLDIR}/lib \
			-L${PLATFORM}/usr/lib64 \
			-L${PLATFORM}/usr/lib \
			${MARCHLDFLAGS["$ABI"]}"

		export CHOST="Android"
		export CROSS_PREFIX="${CROSS_COMPILE}-"

		./configure --static -prefix=${INSTALLDIR} \
		&& make \
		&& make install \
		&& make clean

		rm -rf configure.log
		rm -rf Makefile
		rm -rf zlib.pc
		rm -rf ztest9893.c

		if !(( $? == 0 )); then
			onError "Unable to build ICU for ABI: ${ABI} PLATFORM: android-${ANDROIDVER}!"
		fi
	#buildGLIB
	#buildICU
	#buildJavaScriptCore
done

#buildAAR
