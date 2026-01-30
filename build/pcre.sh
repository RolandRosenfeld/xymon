	echo "Checking for PCRE ..."

	PCREINC=""
	PCRELIB=""
	PCREDEF=""
	PCRELIBNAME="pcre"
	for DIR in /opt/pcre* /usr/local/pcre* /usr/local /usr/pkg /opt/csw /opt/sfw
	do
		if test -f $DIR/include/pcre.h
		then
			PCREINC=$DIR/include
		fi
		if test -f $DIR/include/pcre/pcre.h
		then
			PCREINC=$DIR/include/pcre
		fi

		if test -f $DIR/lib/libpcre.so
		then
			PCRELIB=$DIR/lib
		fi
		if test -f $DIR/lib/libpcre.a
		then
			PCRELIB=$DIR/lib
		fi
		if test -f $DIR/lib64/libpcre.so
		then
			PCRELIB=$DIR/lib64
		fi
		if test -f $DIR/lib64/libpcre.a
		then
			PCRELIB=$DIR/lib64
		fi
	done

	if test "$USERPCREINC" != ""; then
		PCREINC="$USERPCREINC"
	fi
	if test "$USERPCRELIB" != ""; then
		PCRELIB="$USERPCRELIB"
	fi

	# Lets see if it can build
	PCREOK="YES"
	cd build
	if test "$PCREINC" != ""; then INCOPT="-I$PCREINC"; fi
	if test "$PCRELIB" != ""; then LIBOPT="-L$PCRELIB"; fi

	if pkg-config --exists libpcre2-8 > /dev/null 2>&1; then
		PCREDEF="-DPCRE2"
		INCOPT=$(pkg-config --cflags libpcre2-8)
		LIBOPT=$(pkg-config --libs libpcre2-8)
		PCRELIBNAME=pcre2-8
	fi

	OS=`uname -s | sed -e's@/@_@g'` $MAKE -f Makefile.test-pcre clean
	OS=`uname -s | sed -e's@/@_@g'` PCREINC="$INCOPT" PCREDEF="$PCREDEF" $MAKE -f Makefile.test-pcre test-compile
	if test $? -eq 0; then
		echo "Compiling with PCRE library works OK"
	else
		echo "ERROR: Cannot compile using PCRE library."
		PCREOK="NO"
	fi

	OS=`uname -s | sed -e's@/@_@g'` PCRELIB="$LIBOPT" PCRELIBNAME="$PCRELIBNAME" $MAKE -f Makefile.test-pcre test-link
	if test $? -eq 0; then
		echo "Linking with PCRE library works OK"
	else
		echo "ERROR: Cannot link with PCRE library."
		PCREOK="NO"
	fi
	OS=`uname -s | sed -e's@/@_@g'` $MAKE -f Makefile.test-pcre clean
	cd ..

	if test "$PCREOK" = "NO"; then
		echo "Missing PCRE include- or library-files. These are REQUIRED for xymond"
		echo "PCRE can be found at http://www.pcre.org/"
		echo "If you have PCRE installed, use the \"--pcreinclude DIR\" and \"--pcrelib DIR\""
		echo "options to configure to specify where they are."
		exit 1
	fi


